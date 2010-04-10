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

    /**
     * Because plasma won't show notifications for less than 1.2 seconds, we'll wait to send the
     * emitResult() signal for successfully completed transfers at least for 1.2 seconds. This
     * function will be called 1.2 seconds after the start() function was called to see if the job
     * has already finished, and it will finish if m_transferCompleted is true, or else it will set
     * m_canFinish to true so that when transferCompleted() it will directly emitResult().
     */
    void checkFinish();

    /**
     * If m_canFinish is true, this function will directly call to emitResult(). Else, it will set
     * m_transferCompleted to true.
     *
     * @see checkFinish()
     */
    void transferCompleted();

private:
    OpenObex::ServerSession* m_serverSession;
    qulonglong m_totalFileSize;
    Solid::Control::BluetoothRemoteDevice bluetoothDevice;
    QTime m_time;
    qlonglong m_procesedBytes;
    KUrl m_url;
    bool m_canFinish;
    bool m_transferCompleted;
};

}

#endif // OPENOBEX_FILETRANSFERJOB_H
