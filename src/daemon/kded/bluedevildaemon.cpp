/*************************************************************************************
 *  Copyright (C) 2010-2012 by Alejandro Fiestas Olivares <afiestas@kde.org>         *
 *  Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>                      *
 *  Copyright (C) 2010 UFO Coders <info@ufocoders.com>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "bluedevildaemon.h"
#include "bluezagent.h"
#include "filereceiversettings.h"
#include "filereceiver/filereceiver.h"
#include "devicemonitor.h"
#include "debug_p.h"
#include "version.h"

#include <QTimer>
#include <QProcess>
#include <QDBusMetaType>

#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>

#include <BluezQt/Device>
#include <BluezQt/Adapter>
#include <BluezQt/PendingCall>
#include <BluezQt/InitManagerJob>

K_PLUGIN_FACTORY_WITH_JSON(BlueDevilFactory,
                           "bluedevil.json",
                           registerPlugin<BlueDevilDaemon>();)

Q_DECLARE_METATYPE(DeviceInfo)
Q_DECLARE_METATYPE(QMapDeviceInfo)

struct BlueDevilDaemon::Private
{
    enum Status {
        Online = 0,
        Offline
    } m_status;

    QTimer m_timer;
    BluezAgent *m_bluezAgent;
    FileReceiver *m_fileReceiver;
    DeviceMonitor *m_deviceMonitor;
    BluezQt::ManagerPtr m_manager;
};

BlueDevilDaemon::BlueDevilDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , d(new Private)
{
    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<QMapDeviceInfo>();

    d->m_status = Private::Offline;
    d->m_bluezAgent = new BluezAgent(this);
    d->m_fileReceiver = Q_NULLPTR;
    d->m_deviceMonitor = Q_NULLPTR;
    d->m_timer.setSingleShot(true);
    connect(&d->m_timer, &QTimer::timeout, this, &BlueDevilDaemon::stopDiscovering);

    KAboutData aboutData(
        QStringLiteral("bluedevildaemon"),
        i18n("Bluetooth Daemon"),
        BLUEDEVIL_VERSION,
        i18n("Bluetooth Daemon"),
        KAboutLicense::GPL,
        i18n("(c) 2010, UFO Coders")
    );

    aboutData.addAuthor(QStringLiteral("David Rosca"), i18n("Maintainer"),
                        QStringLiteral("nowrep@gmail.com"), QStringLiteral("http://david.rosca.cz"));

    aboutData.addAuthor(QStringLiteral("Alejandro Fiestas Olivares"), i18n("Previous Maintainer"),
                        QStringLiteral("afiestas@kde.org"), QStringLiteral("http://www.afiestas.org"));

    aboutData.addAuthor(QStringLiteral("Eduardo Robles Elvira"), i18n("Previous Maintainer"),
                        QStringLiteral("edulix@gmail.com"), QStringLiteral("http://blog.edulix.es"));

    aboutData.setComponentName(QStringLiteral("bluedevil"));
    KAboutData::registerPluginData(aboutData);

    // Initialize BluezQt
    d->m_manager = BluezQt::ManagerPtr(new BluezQt::Manager);
    BluezQt::InitManagerJob *job = d->m_manager->init();
    job->start();
    connect(job, &BluezQt::InitManagerJob::result, this, &BlueDevilDaemon::initJobResult);
}

BlueDevilDaemon::~BlueDevilDaemon()
{
    if (d->m_status == Private::Online) {
        offlineMode();
    }

    delete d;
}

bool BlueDevilDaemon::isOnline()
{
    return d->m_status == Private::Online;
}

QMapDeviceInfo BlueDevilDaemon::allDevices()
{
    QMapDeviceInfo devices;

    Q_FOREACH (BluezQt::DevicePtr device, d->m_manager->devices()) {
        devices[device->address()] = deviceToInfo(device);
    }

    return devices;
}

DeviceInfo BlueDevilDaemon::device(const QString &address)
{
    BluezQt::DevicePtr device = d->m_manager->deviceForAddress(address);
    return deviceToInfo(device);
}

void BlueDevilDaemon::startDiscovering(quint32 timeout)
{
    if (!d->m_manager->usableAdapter()) {
        return;
    }

    qCDebug(BLUEDAEMON) << "Start discovering for" << timeout << "ms";

    d->m_manager->usableAdapter()->startDiscovery();

    if (timeout > 0) {
        d->m_timer.start(timeout);
    }
}

void BlueDevilDaemon::stopDiscovering()
{
    if (!d->m_manager->usableAdapter()) {
        return;
    }

    qCDebug(BLUEDAEMON) << "Stop discovering";

    if (d->m_manager->usableAdapter()->isDiscovering()) {
        d->m_manager->usableAdapter()->stopDiscovery();
    }
}

void BlueDevilDaemon::reloadConfig()
{
    loadConfig();
}

void BlueDevilDaemon::initJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error()) {
        qCDebug(BLUEDAEMON) << "Error initializing manager:" << job->errorText();
        return;
    }

    bluetoothOperationalChanged(d->m_manager->isBluetoothOperational());
    connect(d->m_manager.data(), &BluezQt::Manager::bluetoothOperationalChanged,
            this, &BlueDevilDaemon::bluetoothOperationalChanged);

    d->m_deviceMonitor = new DeviceMonitor(d->m_manager, this);
}

void BlueDevilDaemon::bluetoothOperationalChanged(bool operational)
{
    if (operational) {
        onlineMode();
    } else {
        offlineMode();
    }
}

void BlueDevilDaemon::onlineMode()
{
    if (d->m_status == Private::Online) {
        qCDebug(BLUEDAEMON) << "Already in OnlineMode";
        return;
    }

    connect(d->m_manager->registerAgent(d->m_bluezAgent), &BluezQt::PendingCall::finished, this, [this](BluezQt::PendingCall *call) {
        if (call->error()) {
            qCWarning(BLUEDAEMON) << "Error registering Agent" << call->errorText();
        } else {
            qCDebug(BLUEDAEMON) << "Agent registered";
        }
    });

    connect(d->m_manager->requestDefaultAgent(d->m_bluezAgent), &BluezQt::PendingCall::finished, this, [this](BluezQt::PendingCall *call) {
        if (call->error()) {
            qCWarning(BLUEDAEMON) << "Error requesting default Agent" << call->errorText();
        } else {
            qCDebug(BLUEDAEMON) << "Requested default Agent";
        }
    });

    loadConfig();

    d->m_status = Private::Online;
}

void BlueDevilDaemon::offlineMode()
{
    if (d->m_status == Private::Offline) {
        qCDebug(BLUEDAEMON) << "Already in OfflineMode";
        return;
    }

    d->m_manager->unregisterAgent(d->m_bluezAgent);;

    if (d->m_fileReceiver) {
        qCDebug(BLUEDAEMON) << "Stoppping file receiver";
        delete d->m_fileReceiver;
        d->m_fileReceiver = 0;
    }

    d->m_status = Private::Offline;
}

void BlueDevilDaemon::loadConfig()
{
    FileReceiverSettings::self()->load();

    if (!d->m_fileReceiver && FileReceiverSettings::self()->enabled()) {
        d->m_fileReceiver = new FileReceiver(d->m_manager, this);
    }

    if (d->m_fileReceiver && !FileReceiverSettings::self()->enabled()) {
        qCDebug(BLUEDAEMON) << "Stoppping file receiver";
        delete d->m_fileReceiver;
        d->m_fileReceiver = 0;
    }
}

DeviceInfo BlueDevilDaemon::deviceToInfo(BluezQt::DevicePtr device) const
{
    DeviceInfo info;

    if (!device) {
        return info;
    }

    info[QStringLiteral("name")] = device->name();
    info[QStringLiteral("icon")] = device->icon();
    info[QStringLiteral("address")] = device->address();
    info[QStringLiteral("UBI")] = device->ubi();
    info[QStringLiteral("UUIDs")] = device->uuids().join(QLatin1Char(','));

    return info;
}

#include "bluedevildaemon.moc"
