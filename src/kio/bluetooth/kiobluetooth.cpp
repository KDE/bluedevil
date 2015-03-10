/*************************************************************************************
 *  Copyright (C) 2010-2012 by Alejandro Fiestas Olivares <afiestas@kde.org>         *
 *  Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>                      *
 *  Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>                    *
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

#include "kiobluetooth.h"
#include "kdedbluedevil.h"
#include "version.h"

#include <QThread>
#include <QCoreApplication>
#include <QDBusMetaType>

#include <KProcess>
#include <KLocalizedString>

extern "C" int Q_DECL_EXPORT kdemain(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if (argc != 4) {
        fprintf(stderr, "Usage: kio_bluetooth protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    KioBluetooth slave(argv[2], argv[3]);
    slave.dispatchLoop();
    return 0;
}

KioBluetooth::KioBluetooth(const QByteArray &pool, const QByteArray &app)
    : SlaveBase(QByteArrayLiteral("bluetooth"), pool, app)
{
    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<QMapDeviceInfo>();

    m_hasCurrentHost = false;

    Service s;
    s.name = i18n("Send File");
    s.icon = QStringLiteral("edit-copy");
    s.mimetype = QStringLiteral("application/vnd.kde.bluedevil-sendfile");
    s.uuid = QStringLiteral("00001105-0000-1000-8000-00805F9B34FB");
    m_supportedServices.insert(QStringLiteral("00001105-0000-1000-8000-00805F9B34FB"), s);

    s.name = i18n("Browse Files");
    s.icon = QStringLiteral("edit-find");
    s.mimetype = QString();
    s.uuid = QStringLiteral("00001106-0000-1000-8000-00805F9B34FB");
    m_supportedServices.insert(QStringLiteral("00001106-0000-1000-8000-00805F9B34FB"), s);

    qCDebug(BLUETOOTH) << "Kio Bluetooth instanced!";
    m_kded = new org::kde::BlueDevil(QStringLiteral("org.kde.kded5"), QStringLiteral("/modules/bluedevil"),
                                     QDBusConnection::sessionBus(), 0);

    if (!m_kded->isOnline()) {
        qCDebug(BLUETOOTH) << "Bluetooth is offline";
        infoMessage(i18n("No Bluetooth adapters have been found."));
        return;
    }
}

QList<KioBluetooth::Service> KioBluetooth::getSupportedServices(const QStringList &uuids)
{
    qCDebug(BLUETOOTH) << "supported services: " << uuids;
    QList<Service> retValue;
    Q_FOREACH (const QString &uuid, uuids) {
        if (m_supportedServices.contains(uuid)) {
            retValue << m_supportedServices[uuid];
        }
    }
    return retValue;
}

void KioBluetooth::listRemoteDeviceServices()
{
    infoMessage(i18n("Retrieving services..."));

    qCDebug(BLUETOOTH) << "Listing remote devices";

    const DeviceInfo &info = m_kded->device(m_currentHostAddress).value();
    if (info.isEmpty()) {
        qCDebug(BLUETOOTH) << "Invalid hostname!";
        infoMessage(i18n("This address is unavailable."));
        finished();
        return;
    }

    m_currentHostServices = getSupportedServices(info.value(QStringLiteral("UUIDs")).split(QLatin1Char(',')));

    qCDebug(BLUETOOTH) << "Num of supported services: " << m_currentHostServices.size();

    totalSize(m_currentHostServices.count());
    int i = 1;
    Q_FOREACH (const Service &service, m_currentHostServices) {
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, service.uuid);
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, service.name);
        entry.insert(KIO::UDSEntry::UDS_ICON_NAME, service.icon);

        //If it is browse files, act as a folder
        if (service.uuid == QLatin1String("00001106-0000-1000-8000-00805F9B34FB")) {
            QUrl obexUrl;
            obexUrl.setScheme(QStringLiteral("obexftp"));
            obexUrl.setHost(m_currentHostname.replace(QLatin1Char(':'), QLatin1Char('-')).toUpper());
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
            entry.insert(KIO::UDSEntry::UDS_URL, obexUrl.toString());
        } else {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
            entry.insert(KIO::UDSEntry::UDS_ACCESS, S_IRWXU | S_IRWXG | S_IRWXO);
        }

        if (service.mimetype.isEmpty()) {
            entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/vnd.kde.bluedevil.service"));
        } else {
            entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, service.mimetype);
        }
        listEntry(entry);
        processedSize(i++);
    }

    infoMessage(QString());
    finished();
}

void KioBluetooth::listDevices()
{
    qCDebug(BLUETOOTH) << "Asking kded for devices";
    const QMapDeviceInfo &devices = m_kded->allDevices().value();
    qCDebug(BLUETOOTH) << devices.keys();

    Q_FOREACH(const DeviceInfo device, devices) {
        listDevice(device);
    }

    m_kded->startDiscovering(10 * 1000);

    infoMessage(i18n("Scanning for new devices..."));
    finished();
}

void KioBluetooth::listDevice(const DeviceInfo device)
{
    qCDebug(BLUETOOTH) << device;
    if (getSupportedServices(device[QStringLiteral("UUIDs")].split(QStringLiteral(","))).isEmpty()) {
        return;
    }
    QString target = QStringLiteral("bluetooth://");
    target.append(QString(device[QStringLiteral("address")]).replace(QLatin1Char(':'), QLatin1Char('-')));

    KIO::UDSEntry entry;
    entry.insert(KIO::UDSEntry::UDS_URL, target);
    entry.insert(KIO::UDSEntry::UDS_NAME, device[QStringLiteral("name")]);
    entry.insert(KIO::UDSEntry::UDS_ICON_NAME, device[QStringLiteral("icon")]);
    entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    entry.insert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH);
    if (device.contains(QStringLiteral("discovered")) && device[QStringLiteral("discovered")] == QLatin1String("true")) {
        entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/vnd.kde.bluedevil.device.discovered"));
    } else {
        entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/vnd.kde.bluedevil.device"));
    }
    listEntry(entry);
}

void KioBluetooth::listDir(const QUrl &url)
{
    qCDebug(BLUETOOTH) << "Listing..." << url;

    /// Url is not used here becuase all we could care about the url is the host, and that's already
    /// handled in @p setHost
    Q_UNUSED(url);

    // If we are not online (ie. there's no working bluetooth adapter), list an empty dir
    qCDebug(BLUETOOTH) << m_kded->isOnline().value();
    if (!m_kded->isOnline().value()) {
        infoMessage(i18n("No Bluetooth adapters have been found."));
        finished();
        return;
    }

    if (!m_hasCurrentHost) {
        listDevices();
    } else {
        listRemoteDeviceServices();
    }
}

void KioBluetooth::stat(const QUrl &url)
{
    qCDebug(BLUETOOTH) << "Stat: " << url;
    finished();
}

void KioBluetooth::get(const QUrl &url)
{
    m_kded->stopDiscovering();
    qCDebug(BLUETOOTH) << "Get: " << url;
    qCDebug(BLUETOOTH) << m_supportedServices.value(url.fileName()).mimetype;
    mimeType(m_supportedServices.value(url.fileName()).mimetype);
    finished();
}

void KioBluetooth::setHost(const QString &hostname, quint16 port, const QString &user,
                           const QString &pass)
{
    qCDebug(BLUETOOTH) << "Setting host: " << hostname;

    // In this kio only the hostname (constHostname) is used
    Q_UNUSED(port)
    Q_UNUSED(user)
    Q_UNUSED(pass)

    if (hostname.isEmpty()) {
        m_hasCurrentHost = false;
    } else {
        m_hasCurrentHost = true;
        m_currentHostServices.clear();

        m_currentHostname = hostname;
        m_currentHostAddress = hostname.toUpper();
        m_currentHostAddress.replace(QLatin1Char('-'), QLatin1Char(':'));
    }
}

Q_LOGGING_CATEGORY(BLUETOOTH, "KioBluetooth")

#include "kiobluetooth.moc"
