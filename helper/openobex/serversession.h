/*
    This file is part of the KDE project

    Copyright (C) 2010 by Eduardo Robles Elvira <edulix@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef OPENOBEX_SERVERSESSION_H
#define OPENOBEX_SERVERSESSION_H

#include <QObject>
#include <solid/control/bluetoothremotedevice.h>
#include "server_session_interface.h"

namespace OpenObex {

class FileTransferJob;

class ServerSession : public QObject
{
    Q_OBJECT
public:
    ServerSession(const QString& path, const QString& bluetoothAddress);
    virtual ~ServerSession();
    QString path();
    org::openobex::ServerSession* dbusServerSession();

public Q_SLOTS:
    void slotCancelled();
    void slotDisconnected();
    void slotTransferStarted(const QString& filename, const QString& localPath,
        qulonglong totalBytes);

private Q_SLOTS:
    void slotAccept();
    void slotCancel();
    void slotSaveAs();

private:
    QString m_path;
    org::openobex::ServerSession* m_dbusServerSession;
    QDBusObjectPath m_serverPath;
    Solid::Control::BluetoothRemoteDevice m_bluetoothDevice;

    // These three vars store information received when the slotTransferStarted gets called and
    // needs to be stored so that it's available when the user decides to accept the file transfer
    QString  m_filename;
    QString m_localDir;
    qulonglong m_totalBytes;
};

}

#endif // OPENOBEX_SERVERSESSION_H
