/*  This file is part of the KDE libraries

    Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>
    Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
    Copyright (C) 2010 UFO Coders <info@ufocoders.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kiobluetooth.h"
#include "kdedbluedevil.h"

#include <QtCore/QThread>

#include <KDebug>
#include <KComponentData>
#include <KCmdLineArgs>
#include <KAboutData>
#include <KProcess>

#include <KApplication>
#include <KLocale>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

extern "C" int KDE_EXPORT kdemain(int argc, char **argv)
{
    KAboutData about("kiobluetooth", "bluedevil", ki18n("kiobluetooth"), 0);
    KCmdLineArgs::init(&about);

    KApplication app;

    if (argc != 4) {
        fprintf(stderr, "Usage: kio_bluetooth protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    KioBluetooth slave(argv[2], argv[3]);
    slave.dispatchLoop();
    return 0;
}

QList<KioBluetooth::Service> KioBluetooth::getSupportedServices(const QStringList &uuids)
{
    kDebug() << "supported services: " << uuids;
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

    kDebug() << "Listing remote devices";
    m_currentHost = Manager::self()->defaultAdapter()->deviceForAddress(m_currentHostname.replace('-', ':').toUpper());
    m_currentHostServices = getSupportedServices(m_currentHost->UUIDs());

    kDebug() << "Num of supported services: " << m_currentHostServices.size();
    totalSize(m_currentHostServices.count());
    int i = 1;
    Q_FOREACH (const Service &service, m_currentHostServices) {
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, service.uuid);
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, service.name);
        entry.insert(KIO::UDSEntry::UDS_ICON_NAME, service.icon);

        //If it is browse files, act as a folder
        if (service.uuid == "00001106-0000-1000-8000-00805F9B34FB") {
            KUrl obexUrl;
            obexUrl.setProtocol("obexftp");
            obexUrl.setHost(m_currentHostname.replace(':', '-').toUpper());
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
            entry.insert(KIO::UDSEntry::UDS_URL, obexUrl.url());
        } else {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
            entry.insert(KIO::UDSEntry::UDS_ACCESS, S_IRWXU | S_IRWXG | S_IRWXO);
        }

        if (service.mimetype.isEmpty()) {
            entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, "inode/x-vnd.kde.bluedevil.service");
        } else {
            entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, service.mimetype);
        }
        listEntry(entry, false);
        processedSize(i++);
    }

    listEntry(KIO::UDSEntry(), true);
    infoMessage("");
    finished();
}

class SleeperThread
    : public QThread
{
public:
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
};

void KioBluetooth::listDevices()
{
    infoMessage(i18n("Scanning for remote devices..."));
    totalSize(100);
    Manager::self()->defaultAdapter()->startDiscovery();
    for (int i = 0; i < 100; ++i) {
        SleeperThread::msleep(100);
        processedSize(i + 1);
        QApplication::processEvents();
    }
    Manager::self()->defaultAdapter()->stopDiscovery();
    listEntry(KIO::UDSEntry(), true);
    infoMessage("");
    finished();
}

void KioBluetooth::listDevice(Device *device)
{
    const QString target = QString("bluetooth://").append(QString(device->address()).replace(':', '-'));
    const QString alias = device->alias();
    QString name = device->name();
    if (alias.isEmpty() && name.isEmpty()) {
        name = i18n("Untitled device");
    } else {
        name = device->friendlyName();
    }
    KIO::UDSEntry entry;
    entry.insert(KIO::UDSEntry::UDS_URL, target);
    entry.insert(KIO::UDSEntry::UDS_NAME, name);
    entry.insert(KIO::UDSEntry::UDS_ICON_NAME, device->icon());
    entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    entry.insert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH);
    entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, "inode/x-vnd.kde.bluedevil.device");
    listEntry(entry, false);
}
//@endcond

KioBluetooth::KioBluetooth(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("bluetooth", pool, app)
{
    m_hasCurrentHost = false;

    Service s;
    s.name = i18n("Send File");
    s.icon = "edit-copy";
    s.mimetype = "virtual/bluedevil-sendfile";
    s.uuid = "00001105-0000-1000-8000-00805F9B34FB";
    m_supportedServices.insert("00001105-0000-1000-8000-00805F9B34FB", s);
    s.name = i18n("Browse Files");
    s.icon = "edit-find";
    s.mimetype = "";
    s.uuid = "00001106-0000-1000-8000-00805F9B34FB";
    m_supportedServices.insert("00001106-0000-1000-8000-00805F9B34FB", s);
    s.name = i18n("Human Interface Device");
    s.icon = "input-mouse";
    s.mimetype = "virtual/bluedevil-input";
    s.uuid = "00001124-0000-1000-8000-00805F9B34FB";
    m_supportedServices.insert("00001124-0000-1000-8000-00805F9B34FB", s);
    s.name = i18n("Headset");
    s.icon = "audio-headset";
    s.mimetype = "virtual/bluedevil-audio";
    s.uuid = "00001108-0000-1000-8000-00805F9B34FB";
    m_supportedServices.insert("00001108-0000-1000-8000-00805F9B34FB", s);

    if (!Manager::self()->defaultAdapter()) {
        kDebug() << "No available interface";
        infoMessage(i18n("No Bluetooth adapters have been found."));
        return;
    }

    connect(Manager::self()->defaultAdapter(), SIGNAL(deviceFound(Device*)), this, SLOT(listDevice(Device*)));

    kDebug() << "Kio Bluetooth instanced!";
    m_kded = new org::kde::BlueDevil("org.kde.kded", "/modules/bluedevil", QDBusConnection::sessionBus(), 0);
}

void KioBluetooth::listDir(const KUrl &url)
{
    kDebug() << "Listing..." << url;

    /// Url is not used here becuase all we could care about the url is the host, and that's already
    /// handled in @p setHost
    Q_UNUSED(url);

    // If we are not online (ie. there's no working bluetooth adapter), list an empty dir
    kDebug() << m_kded->isOnline().value();
    if (!m_kded->isOnline().value()) {
        infoMessage(i18n("No Bluetooth adapters have been found."));
        listEntry(KIO::UDSEntry(), true);
        finished();
        return;
    }

    if (!m_hasCurrentHost) {
        listDevices();
    } else {
        listRemoteDeviceServices();
    }
}

void KioBluetooth::stat(const KUrl &url)
{
    kDebug() << "Stat: " << url;
    finished();
}

void KioBluetooth::get(const KUrl &url)
{
    kDebug() << "Get: " << url;
    kDebug() << m_supportedServices.value(url.fileName()).mimetype;
    mimeType(m_supportedServices.value(url.fileName()).mimetype);
    finished();
}

void KioBluetooth::setHost(const QString &constHostname, quint16 port, const QString &user,
                           const QString &pass)
{
    kDebug() << "Setting host: " << constHostname;

    // In this kio only the hostname (constHostname) is used
    Q_UNUSED(port)
    Q_UNUSED(user)
    Q_UNUSED(pass)

    QString hostname = constHostname;
    hostname = hostname.replace('-', ':').toUpper();
    if (hostname.isEmpty()) {
        m_hasCurrentHost = false;
    } else {
        m_hasCurrentHost = true;
        m_currentHostname = constHostname;
        m_currentHostServices.clear();
    }
}


#include "kiobluetooth.moc"
