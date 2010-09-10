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
    KAboutData about("kiobluetooth", "kiobluetooth", ki18n("kiobluetooth"), 0);
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

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
//@cond PRIVATE
class KioBluetoothPrivate
{
public:
    /**
     * The constructor basically populates the map of uuids to service names
     */
    KioBluetoothPrivate(KioBluetooth *parent);

    /**
     * Returns a list of service names corresponding to the given uuids list. If an uuid is not
     * found in the uuids list, it's added as-is to the returning list of service names.
     */
    QStringList getServiceNames(const QStringList &uuids);

    struct Service {
        QString name;
        QString icon;
        QString mimetype;
        QString uuid;
    };

    /**
     * Returns a list of supported service names corresponding to the given uuids list. If an uuid is
     * not found in the uuids list, it is not added to the list of service names.
     */
    QList<Service> getSupportedServices(const QStringList &uuids);

    /**
     * Called by @p Bluetooth::listDir when listing root dir, bluetooth:/.
     */
    void listDevices();

    /**
     * Called by @p Bluetooth::listDir when listing a remote device (something like
     * bluetoth:/00_12_34_56_6d_34) services.
     */
    void listRemoteDeviceServices();

    void listDevice(Device *device);

public:

     /**
      * Specifies if we've got a working bluetooth adpater to our computer or not.
      */
     bool m_online;

    /**
     * This is set to true when @p setHost is called to list a given remote device, like for example
     * 00:2a:5E:8e:6e:f5. If listing the remote devices (bluetooth:/ uri), it's set back to false.
     */
    bool m_hasCurrentHost;

    /**
     * This is set in @p setHost when it's called to list a given remote device like for example
     * 00:2a:5E:8e:6e:f5. We don't directly set @p currentHost in @p setHost because libbludevil might not
     * have ready the remote bluetooth device yet ready at that time (it.s being created by the call
     * to @p Solid::Control::BluetoothDevice::createBluetoothRemoteDevice .
     */
    QString m_currentHostname;

    /**
     * Represents the current host when @p hasCurrentHost is set to true. This is set in
     * @p listRemoteDeviceServices function.
     */
    Device *m_currentHost;

    /**
     * When @p hasCurrentHost to true, this list holds the list of service names provided by the
     * current host (which is a remote device we can connect to using those services).
     */
    QList<Service> m_currentHostServices;

    /**
     * This is an array containing as key the uuid and as value the name of the service that the
     * given uuid represents.
     */
    QMap<QString, QString> m_serviceNames;

    /**
     * This is an array containing as key the uuid and as value the name of the service that the
     * given uuid represents, and a representative icon. It only contains the supported service names.
     */
    QMap<QString, Service> m_supportedServices;

    KioBluetooth *m_q;
};

KioBluetoothPrivate::KioBluetoothPrivate(KioBluetooth *parent)
  : m_q(parent)
{
    m_serviceNames.insert("00001000-0000-1000-8000-00805F9B34FB", i18n("Service Discovery Server"));
    m_serviceNames.insert("00001001-0000-1000-8000-00805F9B34FB", i18n("Browse Group"));
    m_serviceNames.insert("00001002-0000-1000-8000-00805F9B34FB", i18n("Public Browse Group"));
    m_serviceNames.insert("00001101-0000-1000-8000-00805F9B34FB", i18n("Serial Port"));
    m_serviceNames.insert("00001102-0000-1000-8000-00805F9B34FB", i18n("LAN Access Using PPP"));
    m_serviceNames.insert("00001103-0000-1000-8000-00805F9B34FB", i18n("Dial up Networking"));
    m_serviceNames.insert("00001104-0000-1000-8000-00805F9B34FB", i18n("Ir MCSync"));
    m_serviceNames.insert("00001105-0000-1000-8000-00805F9B34FB", i18n("OBEX Object Push"));
    m_serviceNames.insert("00001106-0000-1000-8000-00805F9B34FB", i18n("OBEX File Transfer"));
    m_serviceNames.insert("00001107-0000-1000-8000-00805F9B34FB", i18n("Ir MCSync Command"));
    m_serviceNames.insert("00001108-0000-1000-8000-00805F9B34FB", i18n("Headset"));
    m_serviceNames.insert("00001109-0000-1000-8000-00805F9B34FB", i18n("Cordless Telephony"));
    m_serviceNames.insert("0000110A-0000-1000-8000-00805F9B34FB", i18n("Audio Source"));
    m_serviceNames.insert("0000110B-0000-1000-8000-00805F9B34FB", i18n("Audio Sink"));
    m_serviceNames.insert("0000110C-0000-1000-8000-00805F9B34FB", i18n("AV Remote Control Target"));
    m_serviceNames.insert("0000110D-0000-1000-8000-00805F9B34FB", i18n("Advanced Audio Distribution"));
    m_serviceNames.insert("0000110E-0000-1000-8000-00805F9B34FB", i18n("AV Remote Control"));
    m_serviceNames.insert("0000110F-0000-1000-8000-00805F9B34FB", i18n("Video Conferencing"));
    m_serviceNames.insert("00001110-0000-1000-8000-00805F9B34FB", i18n("Intercom"));
    m_serviceNames.insert("00001111-0000-1000-8000-00805F9B34FB", i18n("Fax"));
    m_serviceNames.insert("00001112-0000-1000-8000-00805F9B34FB", i18n("Headset Audio Gateway"));
    m_serviceNames.insert("00001113-0000-1000-8000-00805F9B34FB", i18n("WAP"));
    m_serviceNames.insert("00001114-0000-1000-8000-00805F9B34FB", i18n("WAP Client"));
    m_serviceNames.insert("00001115-0000-1000-8000-00805F9B34FB", i18n("PANU"));
    m_serviceNames.insert("00001116-0000-1000-8000-00805F9B34FB", i18n("NAP"));
    m_serviceNames.insert("00001117-0000-1000-8000-00805F9B34FB", i18n("GN"));
    m_serviceNames.insert("00001118-0000-1000-8000-00805F9B34FB", i18n("Direct Printing"));
    m_serviceNames.insert("00001119-0000-1000-8000-00805F9B34FB", i18n("Reference Printing"));
    m_serviceNames.insert("0000111A-0000-1000-8000-00805F9B34FB", i18n("Imaging"));
    m_serviceNames.insert("0000111B-0000-1000-8000-00805F9B34FB", i18n("Imaging Responder"));
    m_serviceNames.insert("0000111C-0000-1000-8000-00805F9B34FB", i18n("Imaging Automatic Archive"));
    m_serviceNames.insert("0000111D-0000-1000-8000-00805F9B34FB", i18n("Imaging Reference Objects"));
    m_serviceNames.insert("0000111E-0000-1000-8000-00805F9B34FB", i18n("Hands free"));
    m_serviceNames.insert("0000111F-0000-1000-8000-00805F9B34FB", i18n("Hands free Audio Gateway"));
    m_serviceNames.insert("00001120-0000-1000-8000-00805F9B34FB", i18n("Direct Printing Reference Objects"));
    m_serviceNames.insert("00001121-0000-1000-8000-00805F9B34FB", i18n("Reflected UI"));
    m_serviceNames.insert("00001122-0000-1000-8000-00805F9B34FB", i18n("Basic Printing"));
    m_serviceNames.insert("00001123-0000-1000-8000-00805F9B34FB", i18n("Printing Status"));
    m_serviceNames.insert("00001124-0000-1000-8000-00805F9B34FB", i18n("Human Interface Device"));
    m_serviceNames.insert("00001125-0000-1000-8000-00805F9B34FB", i18n("Hardcopy Cable Replacement"));
    m_serviceNames.insert("00001126-0000-1000-8000-00805F9B34FB", i18n("HCR Print"));
    m_serviceNames.insert("00001127-0000-1000-8000-00805F9B34FB", i18n("HCR Scan"));
    m_serviceNames.insert("00001128-0000-1000-8000-00805F9B34FB", i18n("Common ISDN Access"));
    m_serviceNames.insert("00001129-0000-1000-8000-00805F9B34FB", i18n("Video Conferencing GW"));
    m_serviceNames.insert("0000112A-0000-1000-8000-00805F9B34FB", i18n("UDIMT"));
    m_serviceNames.insert("0000112B-0000-1000-8000-00805F9B34FB", i18n("UDITA"));
    m_serviceNames.insert("0000112C-0000-1000-8000-00805F9B34FB", i18n("Audio Video"));
    m_serviceNames.insert("0000112D-0000-1000-8000-00805F9B34FB", i18n("SIM Access"));
    m_serviceNames.insert("00001200-0000-1000-8000-00805F9B34FB", i18n("PnP Information"));
    m_serviceNames.insert("00001201-0000-1000-8000-00805F9B34FB", i18n("Generic Networking"));
    m_serviceNames.insert("00001202-0000-1000-8000-00805F9B34FB", i18n("Generic File Transfer"));
    m_serviceNames.insert("00001203-0000-1000-8000-00805F9B34FB", i18n("Generic Audio"));
    m_serviceNames.insert("00001204-0000-1000-8000-00805F9B34FB", i18n("Generic Telephony"));

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

    kDebug() << "Private instanced";
}

QStringList KioBluetoothPrivate::getServiceNames(const QStringList &uuids)
{
    kDebug() << "getting services :" << uuids;
    QStringList retValue;
    Q_FOREACH (const QString &uuid, uuids) {
        if (m_serviceNames.contains(uuid)) {
            retValue << m_serviceNames[uuid];
        } else {
            retValue << uuid;
        }
    }
    return retValue;
}

QList<KioBluetoothPrivate::Service> KioBluetoothPrivate::getSupportedServices(const QStringList &uuids)
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

void KioBluetoothPrivate::listRemoteDeviceServices()
{
    m_q->infoMessage(i18n("Retrieving services..."));

    kDebug() << "Listing remote devices";
    m_currentHost = Manager::self()->defaultAdapter()->deviceForAddress(m_currentHostname.replace('-', ':').toUpper());
    m_currentHostServices = getSupportedServices(m_currentHost->UUIDs());

    kDebug() << "Num of supported services: " << m_currentHostServices.size();
    m_q->totalSize(m_currentHostServices.count());
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
        m_q->listEntry(entry, false);
        m_q->processedSize(i++);
    }

    m_q->listEntry(KIO::UDSEntry(), true);
    m_q->finished();
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

void KioBluetoothPrivate::listDevices()
{
    m_q->infoMessage(i18n("Scanning for remote devices..."));
    m_q->totalSize(100);
    Manager::self()->defaultAdapter()->startDiscovery();
    for (int i = 0; i < 100; ++i) {
        SleeperThread::msleep(100);
        m_q->processedSize(i + 1);
        QApplication::processEvents();
    }
    Manager::self()->defaultAdapter()->stopDiscovery();
    m_q->listEntry(KIO::UDSEntry(), true);
    m_q->finished();
}

void KioBluetoothPrivate::listDevice(Device *device)
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
    m_q->listEntry(entry, false);
}
//@endcond

KioBluetooth::KioBluetooth(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("bluetooth", pool, app)
    , d(new KioBluetoothPrivate(this))
{
    d->m_hasCurrentHost = false;

    connect(Manager::self(), SIGNAL(adapterAdded(Adapter*)), this,
                    SLOT(defaultAdapterChanged(Adapter*)));

    connect(Manager::self(), SIGNAL(defaultAdapterChanged(Adapter*)), this,
                    SLOT(defaultAdapterChanged(Adapter*)));

    if (!Manager::self()->defaultAdapter()) {
        kDebug() << "No available interface";
        infoMessage(i18n("No bluetooth adapter has been found"));
        d->m_online = false;
        return;
    }

    connect(Manager::self()->defaultAdapter(), SIGNAL(deviceFound(Device*)), this, SLOT(listDevice(Device*)));
    d->m_online = true;

    kDebug() << "Kio Bluetooth instanced!";
}

KioBluetooth::~KioBluetooth()
{
    delete d;
}

void KioBluetooth::listDir(const KUrl &url)
{
    kDebug() << "Listing..." << url;

    /// Url is not used here becuase all we could care about the url is the host, and that's already
    /// handled in @p setHost
    Q_UNUSED(url);

    // If we are not online (ie. there's no working bluetooth adapter), list an empty dir
    if (!d->m_online) {
        infoMessage(i18n("No bluetooth adapter has been found"));
        listEntry(KIO::UDSEntry(), true);
        finished();
        return;
    }

    if (!d->m_hasCurrentHost) {
        d->listDevices();
    } else {
        d->listRemoteDeviceServices();
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
    kDebug() << d->m_supportedServices.value(url.fileName()).mimetype;
    mimeType(d->m_supportedServices.value(url.fileName()).mimetype);
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
        d->m_hasCurrentHost = false;
    } else {
        d->m_hasCurrentHost = true;
        d->m_currentHostname = constHostname;
        d->m_currentHostServices.clear();
    }
}

void KioBluetooth::defaultAdapterChanged(Adapter *adapter)
{
    kDebug() << "Default Adapter Changed: " << adapter;
    if (adapter) {
        kDebug() << "online is true now";
        d->m_online = true;
        return;
    }

    kDebug() << "Default Adapter Removed";
    d->m_online = false;
}


#include "kiobluetooth.moc"
