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

extern "C" int KDE_EXPORT kdemain(int argc, char **argv)
{
    KAboutData about("kio_bluetooth", 0, ki18n("kio_bluetooth"), 0);
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
     bool online;

    /**
     * This is set to true when @p setHost is called to list a given remote device, like for example
     * 00:2a:5E:8e:6e:f5. If listing the remote devices (bluetooth:/ uri), it's set back to false.
     */
    bool hasCurrentHost;

    /**
     * This is set in @p setHost when it's called to list a given remote device like for example
     * 00:2a:5E:8e:6e:f5. We don't directly set @p currentHost in @p setHost because libbludevil might not
     * have ready the remote bluetooth device yet ready at that time (it.s being created by the call
     * to @p Solid::Control::BluetoothDevice::createBluetoothRemoteDevice .
     */
    QString currentHostname;

    /**
     * Represents the current host when @p hasCurrentHost is set to true. This is set in
     * @p listRemoteDeviceServices function.
     */
    BlueDevil::Device *currentHost;

    /**
     * When @p hasCurrentHost to true, this list holds the list of service names provided by the
     * current host (which is a remote device we can connect to using those services).
     */
    QList<Service> currentHostServices;

    /**
     * Represents the bluetooth adapter connected to our PC.
     */
    BlueDevil::Adapter *adapter;

    /**
     * This is an array containing as key the uuid and as value the name of the service that the
     * given uuid represents.
     */
    QMap<QString, QString> serviceNames;

    /**
     * This is an array containing as key the uuid and as value the name of the service that the
     * given uuid represents, and a representative icon. It only contains the supported service names.
     */
    QMap<QString, Service> supportedServices;

    KioBluetooth *q;
};

KioBluetoothPrivate::KioBluetoothPrivate(KioBluetooth *parent)
  : q(parent)
{
    serviceNames.insert("00001000-0000-1000-8000-00805f9b34fb", i18n("Service Discovery Server"));
    serviceNames.insert("00001001-0000-1000-8000-00805f9b34fb", i18n("Browse Group"));
    serviceNames.insert("00001002-0000-1000-8000-00805f9b34fb", i18n("Public Browse Group"));
    serviceNames.insert("00001101-0000-1000-8000-00805f9b34fb", i18n("Serial Port"));
    serviceNames.insert("00001102-0000-1000-8000-00805f9b34fb", i18n("LAN Access Using PPP"));
    serviceNames.insert("00001103-0000-1000-8000-00805f9b34fb", i18n("Dial up Networking"));
    serviceNames.insert("00001104-0000-1000-8000-00805f9b34fb", i18n("Ir MCSync"));
    serviceNames.insert("00001105-0000-1000-8000-00805f9b34fb", i18n("OBEX Object Push"));
    serviceNames.insert("00001106-0000-1000-8000-00805f9b34fb", i18n("OBEX File Transfer"));
    serviceNames.insert("00001107-0000-1000-8000-00805f9b34fb", i18n("Ir MCSync Command"));
    serviceNames.insert("00001108-0000-1000-8000-00805f9b34fb", i18n("Headset"));
    serviceNames.insert("00001109-0000-1000-8000-00805f9b34fb", i18n("Cordless Telephony"));
    serviceNames.insert("0000110a-0000-1000-8000-00805f9b34fb", i18n("Audio Source"));
    serviceNames.insert("0000110b-0000-1000-8000-00805f9b34fb", i18n("Audio Sink"));
    serviceNames.insert("0000110c-0000-1000-8000-00805f9b34fb", i18n("AV Remote Control Target"));
    serviceNames.insert("0000110d-0000-1000-8000-00805f9b34fb", i18n("Advanced Audio Distribution"));
    serviceNames.insert("0000110e-0000-1000-8000-00805f9b34fb", i18n("AV Remote Control"));
    serviceNames.insert("0000110f-0000-1000-8000-00805f9b34fb", i18n("Video Conferencing"));
    serviceNames.insert("00001110-0000-1000-8000-00805f9b34fb", i18n("Intercom"));
    serviceNames.insert("00001111-0000-1000-8000-00805f9b34fb", i18n("Fax"));
    serviceNames.insert("00001112-0000-1000-8000-00805f9b34fb", i18n("Headset Audio Gateway"));
    serviceNames.insert("00001113-0000-1000-8000-00805f9b34fb", i18n("WAP"));
    serviceNames.insert("00001114-0000-1000-8000-00805f9b34fb", i18n("WAP Client"));
    serviceNames.insert("00001115-0000-1000-8000-00805f9b34fb", i18n("PANU"));
    serviceNames.insert("00001116-0000-1000-8000-00805f9b34fb", i18n("NAP"));
    serviceNames.insert("00001117-0000-1000-8000-00805f9b34fb", i18n("GN"));
    serviceNames.insert("00001118-0000-1000-8000-00805f9b34fb", i18n("Direct Printing"));
    serviceNames.insert("00001119-0000-1000-8000-00805f9b34fb", i18n("Reference Printing"));
    serviceNames.insert("0000111a-0000-1000-8000-00805f9b34fb", i18n("Imaging"));
    serviceNames.insert("0000111b-0000-1000-8000-00805f9b34fb", i18n("Imaging Responder"));
    serviceNames.insert("0000111c-0000-1000-8000-00805f9b34fb", i18n("Imaging Automatic Archive"));
    serviceNames.insert("0000111d-0000-1000-8000-00805f9b34fb", i18n("Imaging Reference Objects"));
    serviceNames.insert("0000111e-0000-1000-8000-00805f9b34fb", i18n("Hands free"));
    serviceNames.insert("0000111f-0000-1000-8000-00805f9b34fb", i18n("Hands free Audio Gateway"));
    serviceNames.insert("00001120-0000-1000-8000-00805f9b34fb", i18n("Direct Printing Reference Objects"));
    serviceNames.insert("00001121-0000-1000-8000-00805f9b34fb", i18n("Reflected UI"));
    serviceNames.insert("00001122-0000-1000-8000-00805f9b34fb", i18n("Basic Pringing"));
    serviceNames.insert("00001123-0000-1000-8000-00805f9b34fb", i18n("Printing Status"));
    serviceNames.insert("00001124-0000-1000-8000-00805f9b34fb", i18n("Human Interface Device"));
    serviceNames.insert("00001125-0000-1000-8000-00805f9b34fb", i18n("Hardcopy Cable Replacement"));
    serviceNames.insert("00001126-0000-1000-8000-00805f9b34fb", i18n("HCR Print"));
    serviceNames.insert("00001127-0000-1000-8000-00805f9b34fb", i18n("HCR Scan"));
    serviceNames.insert("00001128-0000-1000-8000-00805f9b34fb", i18n("Common ISDN Access"));
    serviceNames.insert("00001129-0000-1000-8000-00805f9b34fb", i18n("Video Conferencing GW"));
    serviceNames.insert("0000112a-0000-1000-8000-00805f9b34fb", i18n("UDIMT"));
    serviceNames.insert("0000112b-0000-1000-8000-00805f9b34fb", i18n("UDITA"));
    serviceNames.insert("0000112c-0000-1000-8000-00805f9b34fb", i18n("Audio Video"));
    serviceNames.insert("0000112d-0000-1000-8000-00805f9b34fb", i18n("SIM Access"));
    serviceNames.insert("00001200-0000-1000-8000-00805f9b34fb", i18n("PnP Information"));
    serviceNames.insert("00001201-0000-1000-8000-00805f9b34fb", i18n("Generic Networking"));
    serviceNames.insert("00001202-0000-1000-8000-00805f9b34fb", i18n("Generic File Transfer"));
    serviceNames.insert("00001203-0000-1000-8000-00805f9b34fb", i18n("Generic Audio"));
    serviceNames.insert("00001204-0000-1000-8000-00805f9b34fb", i18n("Generic Telephony"));

    Service s;
    s.name = i18n("Send File");
    s.icon = "edit-copy";
    s.mimetype = "virtual/bluedevil-sendfile";
    s.uuid = "00001105-0000-1000-8000-00805f9b34fb";
    supportedServices.insert("00001105-0000-1000-8000-00805f9b34fb", s);
    s.name = i18n("Browse Files");
    s.icon = "edit-find";
    s.mimetype = "";
    s.uuid = "00001106-0000-1000-8000-00805f9b34fb";
    supportedServices.insert("00001106-0000-1000-8000-00805f9b34fb", s);
    s.name = i18n("Human Interface Device");
    s.icon = "input-mouse";
    s.mimetype = "virtual/bluedevil-input";
    s.uuid = "00001124-0000-1000-8000-00805f9b34fb";
    supportedServices.insert("00001124-0000-1000-8000-00805f9b34fb", s);
    s.name = i18n("Headset");
    s.icon = "audio-headset";
    s.mimetype = "virtual/bluedevil-audio";
    s.uuid = "00001108-0000-1000-8000-00805f9b34fb";
    supportedServices.insert("00001108-0000-1000-8000-00805f9b34fb", s);
}

QStringList KioBluetoothPrivate::getServiceNames(const QStringList &uuids)
{
    QStringList retValue;
    Q_FOREACH (const QString &uuid, uuids) {
        if (serviceNames.contains(uuid)) {
            retValue << serviceNames[uuid];
        } else {
            retValue << uuid;
        }
    }
    return retValue;
}

QList<KioBluetoothPrivate::Service> KioBluetoothPrivate::getSupportedServices(const QStringList &uuids)
{
    QList<Service> retValue;
    Q_FOREACH (const QString &uuid, uuids) {
        if (supportedServices.contains(uuid)) {
            retValue << supportedServices[uuid];
        }
    }
    return retValue;
}

void KioBluetoothPrivate::listRemoteDeviceServices()
{
    q->infoMessage(i18n("Retrieving services..."));

    kDebug();
    currentHost = adapter->deviceForAddress(currentHostname.replace('-', ':').toUpper());
    currentHostServices = getSupportedServices(currentHost->UUIDs());

    q->totalSize(currentHostServices.count());
    int i = 1;
    Q_FOREACH (const Service &service, currentHostServices) {
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, service.uuid);
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, service.name);
        entry.insert(KIO::UDSEntry::UDS_ICON_NAME, service.icon);

        //If it is browse files, act as a folder
        if (service.uuid == "00001106-0000-1000-8000-00805f9b34fb") {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
            entry.insert(KIO::UDSEntry::UDS_URL, "obexftp:/A8-7E-33-5D-6F-4E/");
        } else {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
            entry.insert(KIO::UDSEntry::UDS_ACCESS, S_IRWXU | S_IRWXG | S_IRWXO);
        }

        if (service.mimetype.isEmpty()) {
            entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, "inode/x-vnd.kde.bluedevil.service");
        } else {
            entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, service.mimetype);
        }
        q->listEntry(entry, false);
        q->processedSize(i++);
    }

    q->listEntry(KIO::UDSEntry(), true);
    q->finished();
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
    q->infoMessage(i18n("Scanning for remote devices..."));
    q->totalSize(100);
    adapter->startDiscovery();
    for (int i = 0; i < 100; ++i) {
        SleeperThread::msleep(100);
        q->processedSize(i + 1);
        QApplication::processEvents();
    }
    adapter->stopDiscovery();
    q->listEntry(KIO::UDSEntry(), true);
    q->finished();
}

void KioBluetoothPrivate::listDevice(Device *device)
{
    const QString target = QString("bluetooth://").append(QString(device->address()).replace(':', '-'));
    const QString alias = device->alias();
    QString name = device->name();
    if (alias.isEmpty() && name.isEmpty()) {
        name = i18n("Untitled device");
    } else if (name != alias) {
        if (name.isEmpty() && !alias.isEmpty()) {
            name = alias;
        } else if (!name.isEmpty() && !alias.isEmpty()) {
            name = i18n("%1 (%2)").arg(alias).arg(name);
        }
    }
    KIO::UDSEntry entry;
    entry.insert(KIO::UDSEntry::UDS_URL, target);
    entry.insert(KIO::UDSEntry::UDS_NAME, name);
    entry.insert(KIO::UDSEntry::UDS_ICON_NAME, device->icon());
    entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    entry.insert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH);
    entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, "inode/x-vnd.kde.bluedevil.device");
    q->listEntry(entry, false);
}
//@endcond

KioBluetooth::KioBluetooth(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("bluetooth", pool, app)
    , d(new KioBluetoothPrivate(this))
{
    d->hasCurrentHost = false;

    BlueDevil::Adapter *defaultAdapter = BlueDevil::Manager::self()->defaultAdapter();
    if (!defaultAdapter) {
        kDebug() << "No available interface";
        d->online = false;
        return;
    }

    connect(defaultAdapter, SIGNAL(deviceFound(Device*)), this, SLOT(listDevice(Device*)));
    d->adapter = defaultAdapter;
    d->online = true;
}

KioBluetooth::~KioBluetooth()
{
    delete d;
}

void KioBluetooth::listDir(const KUrl &url)
{
    kDebug() << url;
    /// Url is not used here becuase all we could care about the url is the host, and that's already
    /// handled in @p setHost
    Q_UNUSED(url);

    // If we are not online (ie. there's no working bluetooth adapter), list an empty dir
    if (!d->online) {
        listEntry(KIO::UDSEntry(), true);
        finished();
        return;
    }

    if (!d->hasCurrentHost && url.hasHost()) {
        setHost(url.host(), -1, QString(), QString());
    }

    if (!d->hasCurrentHost) {
        d->listDevices();
    } else {
        d->listRemoteDeviceServices();
    }
}

void KioBluetooth::stat(const KUrl &url)
{
    finished();
}

void KioBluetooth::get(const KUrl &url)
{
    mimeType(d->supportedServices.value(url.fileName()).mimetype);
    finished();
}

void KioBluetooth::setHost(const QString &constHostname, quint16 port, const QString &user,
                           const QString &pass)
{
    kDebug() << constHostname;
    // In this kio only the hostname (constHostname) is used
    Q_UNUSED(port)
    Q_UNUSED(user)
    Q_UNUSED(pass)

    QString hostname = constHostname;
    hostname = hostname.replace('-', ':').toUpper();
    if (hostname.isEmpty()) {
        d->hasCurrentHost = false;
    } else {
        d->hasCurrentHost = true;
        d->currentHostname = constHostname;
        d->currentHostServices.clear();
    }
}

#include "kiobluetooth.moc"
