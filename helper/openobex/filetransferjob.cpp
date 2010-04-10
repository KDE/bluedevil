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

#include <QtCore/QTimer>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>

#include <KDebug>
#include <KIO/CopyJob>
#include <KIO/NetAccess>

using namespace OpenObex;

FileTransferJob::FileTransferJob(OpenObex::ServerSession *serverSession,
    const KUrl &url, qulonglong size) : KJob(serverSession)
{
    Q_ASSERT(serverSession != 0);

    connect(serverSession, SIGNAL(customSaveUrl(QString)), SLOT(setCustomSaveUrl(QString)));
    connect(serverSession->dbusServerSession(), SIGNAL(Cancelled()), SLOT(Cancelled()));
    connect(serverSession->dbusServerSession(), SIGNAL(Disconnected()), SLOT(Disconnected()));
    m_transferCompleted = false;
    m_canFinish = false;
    m_serverSession = serverSession;
    m_url = url;
    m_customSaveUrl = QString();
    m_totalFileSize = size;
    setCapabilities(Killable);

    kDebug() << "url" << m_url;
}

FileTransferJob::~FileTransferJob()
{
}

void FileTransferJob::start()
{
    QTimer::singleShot(0, this, SLOT(receiveFiles()));

    /// @see documentation for checkFinish() for details
    QTimer::singleShot(2000, this, SLOT(checkFinish()));
}

void FileTransferJob::checkFinish()
{
    kDebug();
    if (m_transferCompleted) {
        emitResult();
    } else {
        m_canFinish = true;
    }
}

void FileTransferJob::transferCompleted()
{
    kDebug();
    if (m_canFinish) {
        emitResult();
    } else {
        m_transferCompleted = true;
    }
    disconnect(m_serverSession->dbusServerSession(), SIGNAL(Cancelled()), this, SLOT(Cancelled()));
    disconnect(m_serverSession->dbusServerSession(), SIGNAL(Disconnected()), this, SLOT(Disconnected()));
}


void FileTransferJob::reject()
{
    m_serverSession->dbusServerSession()->Reject();
    doKill();
}

void FileTransferJob::receiveFiles()
{
    emit description(this, "Receiving file over bluetooth", QPair<QString, QString>("From", m_bluetoothDevice.name()), QPair<QString, QString>("To", m_url.url()));

    org::openobex::ServerSession *serverSession = m_serverSession->dbusServerSession();
    connect(serverSession, SIGNAL(TransferProgress(qulonglong)),
        this, SLOT(slotTransferProgress(qulonglong)));
    connect(serverSession, SIGNAL(ErrorOccurred(QString, QString)),
        this, SLOT(slotErrorOccured(QString, QString)));
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

void FileTransferJob::setCustomSaveUrl(const QString& customSaveUrl)
{
  kDebug() << "m_customSaveUrl" << m_customSaveUrl;
  m_customSaveUrl = customSaveUrl;

  emit description(this, "Receiving file over bluetooth", QPair<QString, QString>("From", m_bluetoothDevice.name()), QPair<QString, QString>("To", m_customSaveUrl));
}


void FileTransferJob::slotTransferCompleted()
{
    kDebug() << "Transfer completed";
    if (error()) {
      kDebug() << "Error Occurred, do nothing";
      return;
    }

    setProcessedAmount(Bytes, m_totalFileSize);

    if (!m_customSaveUrl.isNull()) {
      // Move the file if it's already been transfered
        KIO::CopyJob *moveJob = KIO::move(KUrl(m_url),KUrl(m_customSaveUrl));
        KIO::NetAccess::synchronousRun(moveJob, NULL);
    }
    m_serverSession->slotJobFinished();
    transferCompleted();
}

void FileTransferJob::slotErrorOccured(const QString &reason1, const QString &reason2)
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

void OpenObex::FileTransferJob::Cancelled()
{
  kDebug();
  kill();
}

void OpenObex::FileTransferJob::Disconnected()
{
  kDebug();
  kill();
}
