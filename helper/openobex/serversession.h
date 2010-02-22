/***************************************************************************
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef OPENOBEX_SERVERSESSION_H
#define OPENOBEX_SERVERSESSION_H

#include <QObject>
#include "server_session_interface.h"

namespace OpenObex {

class ServerSessionFileTransfer;

class ServerSession : public QObject
{
Q_OBJECT
public:
    ServerSession(const QString& path);
    virtual ~ServerSession();
    QString path();
    void accept();
    void reject();
    void cancel();
    org::openobex::ServerSession* dbusServerSession();
    
public Q_SLOTS:
    QMap<QString,QString> getServerSessionInfo(QDBusObjectPath path);
    void slotCancelled();
    void slotDisconnected();
    void slotTransferStarted(const QString& filename, const QString& localPath,
        qulonglong totalBytes);
    void slotErrorOccurred(const QString& errorName, const QString& errorMessage);

private:
    QString m_path;
    org::openobex::ServerSession* m_dbusServerSession;
    OpenObex::ServerSessionFileTransfer* m_fileTransfer;
};

}

#endif // OPENOBEX_SERVERSESSION_H
