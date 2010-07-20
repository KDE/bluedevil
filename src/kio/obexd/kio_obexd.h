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
#ifndef KIO_OBEXD_H
#define KIO_OBEXD_H

#include <QObject>
#include <kio/slavebase.h>

class KioObexd
    : public QObject
    , public KIO::SlaveBase
{
  Q_OBJECT

public:
    /**
     * Constructor
     */
    KioObexd(const QByteArray &pool, const QByteArray &app);

    /**
     * Destructor
     */
    virtual ~KioObexd();

    /**
     * Retrieves a file from the remote device.
     * 
     * Overrides virtual SlaveBase::get()
     */
    void get(const KUrl &url);

    /**
     * List a remote directory. There are two types of directories in this kio:
     *
     * 1. The root dir, obexd://. This directory is empty.
     * 2. Remote device directory (something like bluetoth:/00_12_34_56_6d_34/path/to/dir). This is
     *    used when the setHost function has been called, and lists directories inside a remote
     *    bluetooth device ftp service.
     * 
     * Overrides virtual SlaveBase::listDir()
     */
    void listDir(const KUrl &url);

    /**
     * Sets the remote bluetooth device to which the kio will be connected to, the device that will
     * be used for listing and managing files and directories.
     *
     * Overrides virtual SlaveBase::setHost()
     */
    void setHost(const QString &constHostname, quint16 port, const QString &user,
      const QString &pass);

    /**
     * Calls to slaveStatus().
     *
     * Overrides virtual SlaveBase::slave_status()
     */
    void slave_status();

    /**
     * Overrides virtual SlaveBase::stat()
     */
    void stat(const KUrl &url);

    /**
     * Overrides virtual SlaveBase::del()
     */
    void del(const KUrl &url, bool isfile);

    /**
     * Overrides virtual SlaveBase::url()
     */
    void mkdir(const KUrl&url, int permissions);

private:
    class Private;
    Private *const d;
};

#endif // KIO_OBEXD_H
