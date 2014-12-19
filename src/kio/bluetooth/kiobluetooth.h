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

#ifndef KIOBLUETOOTH_H
#define KIOBLUETOOTH_H

#include "kdedbluedevil.h"

#include <QObject>
#include <kio/slavebase.h>

/**
 * @short This class implements a bluetooth kioslave that list devices and their services.
 */
class KioBluetoothPrivate;

class KioBluetooth : public QObject, public KIO::SlaveBase
{
  Q_OBJECT

public:
    KioBluetooth(const QByteArray &pool, const QByteArray &app);

    struct Service {
        QString name;
        QString icon;
        QString mimetype;
        QString uuid;
    };

    /**
     * As our kio does not perform any service action, but just list devices and their services, the
     * get function shall not do much other than setting a mimetype and returning some data that
     * could be useful for the mimetype handler.
     */
    void get(const KUrl &url);

    /**
     * List current directory. There are two types of current directories in this kio:
     *
     * 1. First type, is the root dir, bluetooth:/. This directory is unique, and lists the remote
     *    devices that our default bluetooth adapter sees.
     * 2. Remote device directory (something like bluetoth:/00_12_34_56_6d_34). This directory lists
     *    the services provided by the given remote device.
     */
    void listDir(const KUrl &url);

    void stat(const KUrl &url);

    /**
     * As at the momento we don't handle more than one level url paths, @p setHost has not much
     * difference with @p listDir
     *
     */
    void setHost(const QString &hostname, quint16 port, const QString &user, const QString &pass);

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

public Q_SLOTS:
    void listDevice(const DeviceInfo device);

private:

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
     * Uppercase colon separated address (ex. 00:2A:5E:8E:6E:F5)
     */
    QString m_currentHostAddress;

    /**
     * When @p hasCurrentHost to true, this list holds the list of service names provided by the
     * current host (which is a remote device we can connect to using those services).
     */
    QList<Service> m_currentHostServices;

    /**
     * This is an array containing as key the uuid and as value the name of the service that the
     * given uuid represents, and a representative icon. It only contains the supported service names.
     */
    QMap<QString, Service> m_supportedServices;

    /**
     * KDED DBus interface, used to communicate to the daemon since we need some status (like connected)
     */
    org::kde::BlueDevil *m_kded;
};

#endif // KIOBLUETOOTH_H
