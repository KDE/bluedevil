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

#include <QObject>
#include <kio/slavebase.h>

/**
 * @short This class implements a bluetooth kioslave that list devices and their services.
 */
class KioBluetoothPrivate;

namespace BlueDevil {
    class Device;
}

typedef BlueDevil::Device Device;

class KioBluetooth : public QObject, public KIO::SlaveBase
{
  Q_OBJECT

public:
    KioBluetooth(const QByteArray &pool, const QByteArray &app);
    virtual ~KioBluetooth();

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

    /**
     * As at the momento we don't handle more than one level url paths, @p setHost has not much
     * difference with @p listDir
     *
     */
    void setHost(const QString &constHostname, quint16 port, const QString &user, const QString &pass);

private:
    KioBluetoothPrivate *d;

    Q_PRIVATE_SLOT(d, void listDevice(Device *device))
};

#endif // KIOBLUETOOTH_H
