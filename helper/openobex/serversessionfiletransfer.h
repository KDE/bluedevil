/***************************************************************************
 *   Copyright (C) 2008  Tom Patzig <tpatzig@suse.de>                      *
 *   Copyright (C) 2008  Alex Fiestas <alex@eyeos.org>                     *
 *   Copyright (C) 2010  Eduardo Robles Elvira <edulix@gmail.com>          *
 *                                                                         *
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
#ifndef OPENOBEX_SERVERSESSIONFILETRANSFER_H
#define OPENOBEX_SERVERSESSIONFILETRANSFER_H

#include "openobex/serversession.h"
#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothremotedevice.h>
#include <KJob>

class QDBusInterface;

namespace OpenObex {

class ServerSessionFileTransfer : public KJob
{
Q_OBJECT
public:
    ServerSessionFileTransfer(OpenObex::ServerSession*, const QString&, const QString&, qulonglong);
    ~ServerSessionFileTransfer();

    void start();
    void reject();
    QString fileName();
    QString localPath();
    QString remoteAddr() const;
    QString remoteName() const;
    qulonglong fileSize();
    void setLocalPath(const QString&);
    void feedbackReceived();
protected:
    bool doKill();

private Q_SLOTS:
    void receiveFiles();
    void slotTransferProgress(qulonglong);
    void slotTransferCompleted();
    void slotErrorOccured(const QString&, const QString&);

private:
    OpenObex::ServerSession* m_serverSession;
    QString m_fileName;
    QString m_localPath;
    qulonglong m_totalFileSize;
    QString m_remoteName;
    QString m_remoteAddr;
    Solid::Control::BluetoothRemoteDevice bluetoothDevice;
    QTime m_time;
    qlonglong m_procesedBytes;
};

}

#endif // OPENOBEX_SERVERSESSIONFILETRANSFER_H
