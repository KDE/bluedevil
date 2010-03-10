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

#include "openobex/filetransferjob.h"
#include "openobex/serversession.h"

#include <QTimer>
#include <qdbusconnection.h>
#include <qdbusinterface.h>

#include <KDebug>

using namespace OpenObex;

FileTransferJob::FileTransferJob(OpenObex::ServerSession* serverSession,
    const QString& filename, const QString& path, qulonglong size) : KJob(serverSession)
{
    m_serverSession = serverSession;
    m_fileName = filename;
    if (path.length() != 0) {
        m_localPath = path;
    }

    m_totalFileSize = size;
    setCapabilities(Killable);

    qDebug() << "m_localPath" << m_localPath;
}

FileTransferJob::~FileTransferJob()
{
}

void FileTransferJob::start()
{
    // TODO get notification even if file transfer takes less than 1.2 seconds
    QTimer::singleShot(0, this, SLOT(receiveFiles()));
}

void FileTransferJob::reject()
{
    m_serverSession->dbusServerSession()->Reject();
    doKill();
}

void FileTransferJob::receiveFiles()
{
    emit description(this, "Receiving file over bluetooth", QPair<QString, QString>("From", bluetoothDevice.name()), QPair<QString, QString>("To", m_localPath));

    org::openobex::ServerSession* serverSession = m_serverSession->dbusServerSession();
    connect(serverSession, SIGNAL(TransferProgress(qulonglong)),
        this, SLOT(slotTransferProgress(qulonglong)));
    connect(serverSession, SIGNAL(ErrorOccurred(const QString&, const QString&)),
        this, SLOT(slotErrorOccured(const QString&, const QString&)));
    connect(serverSession, SIGNAL(TransferCompleted()),
        this, SLOT(slotTransferCompleted()));
    kDebug() << "Transfer started ...";
    setTotalAmount(Bytes, m_totalFileSize);
    setProcessedAmount(Bytes, 0);
    serverSession->Accept();
    m_time = QTime::currentTime();
    m_procesedBytes = 0;
}

void FileTransferJob::slotTransferProgress(qulonglong transferred)
{
    kDebug() << "Transfer progress ..." << transferred;

    QTime currentTime = QTime::currentTime();
    int time = m_time.secsTo(currentTime);
    if (time != 0) {
        qulonglong diffBytes = transferred - m_procesedBytes;
        float speed = diffBytes / time;
        kDebug() << "Bytes: " << diffBytes << " Speed: " << speed;
        emitSpeed(speed);
        m_time = currentTime;
        m_procesedBytes = transferred;
    }
    setProcessedAmount(Bytes, transferred);
}

void FileTransferJob::slotTransferCompleted()
{
    kDebug() << "Transfer completed";
    setProcessedAmount(Bytes, m_totalFileSize);
    emitResult();
}

void FileTransferJob::slotErrorOccured(const QString& reason1, const QString& reason2)
{
    kDebug() << "slotErrorOccured";
    kDebug() << reason1;
    kDebug() << reason2;
    setError(UserDefinedError);
    setErrorText(reason1 + ": " + reason2);
    emitResult();
}

bool FileTransferJob::doKill()
{
    kDebug() << "doKill called";
    m_serverSession->disconnect();
    m_serverSession->dbusServerSession()->Cancel();
    return true;
}
