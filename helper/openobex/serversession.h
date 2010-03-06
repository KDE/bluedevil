/*  This file is part of the KDE project

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
    void accept();
    void reject();
    void cancel();
    org::openobex::ServerSession* dbusServerSession();

public Q_SLOTS:
    void slotCancelled();
    void slotDisconnected();
    void slotTransferStarted(const QString& filename, const QString& localPath,
        qulonglong totalBytes);
    QString bluetoothAddress();

private:
    QString m_path;
    org::openobex::ServerSession* m_dbusServerSession;
    QDBusObjectPath m_serverPath;
    QString m_bluetoothAddress;
};

}

#endif // OPENOBEX_SERVERSESSION_H
