/*
    This file is part of the KDE project

    Copyright (C) 2010  Alex Fiestas <alex@eyeos.org>
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

#ifndef OPENOBEX_FILETRANSFERJOB_H
#define OPENOBEX_FILETRANSFERJOB_H

#include "openobex/serversession.h"
#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothremotedevice.h>
#include <KJob>
#include <KUrl>

class QDBusInterface;

namespace OpenObex {

class FileTransferJob : public KJob
{
Q_OBJECT
public:
    FileTransferJob(OpenObex::ServerSession* serverSession, const KUrl& url, qulonglong size);
    ~FileTransferJob();

    void start();
    void reject();
    qulonglong fileSize();
protected:
    bool doKill();

private Q_SLOTS:
    void receiveFiles();
    void slotTransferProgress(qulonglong);
    void slotTransferCompleted();
    void slotErrorOccured(const QString&, const QString&);

private:
    OpenObex::ServerSession* m_serverSession;
    qulonglong m_totalFileSize;
    Solid::Control::BluetoothRemoteDevice bluetoothDevice;
    QTime m_time;
    qlonglong m_procesedBytes;
    KUrl m_url;
};

}

#endif // OPENOBEX_FILETRANSFERJOB_H
