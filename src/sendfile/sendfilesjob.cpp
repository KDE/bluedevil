/*****************************************************************************
 * This file is part of the KDE project                                      *
 *                                                                           *
 * Copyright (C) 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>     *
 * Copyright (C) 2010-2011 UFO Coders <info@ufocoders.com>                   *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "sendfilesjob.h"
#include "debug_p.h"

#include <QUrl>
#include <QFile>
#include <QDBusObjectPath>

#include <KLocalizedString>

#include <BluezQt/Device>
#include <BluezQt/PendingCall>
#include <BluezQt/ObexManager>
#include <BluezQt/ObexObjectPush>
#include <BluezQt/InitObexManagerJob>

SendFilesJob::SendFilesJob(const QStringList &files, BluezQt::DevicePtr device, QObject *parent)
    : KJob(parent)
    , m_files(files)
    , m_progress(0)
    , m_totalSize(0)
    , m_speedBytes(0)
    , m_currentFileSize(0)
    , m_currentFileProgress(0)
    , m_device(device)
    , m_objectPush(0)
{
    qCDebug(SENDFILE) << "SendFilesJob:" << files;

    Q_FOREACH(const QString &filePath, files) {
        QFile file(filePath);
        m_filesSizes << file.size();
        m_totalSize += file.size();
    }

    setCapabilities(Killable);
}

void SendFilesJob::start()
{
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

bool SendFilesJob::doKill()
{
    if (m_transfer) {
        m_transfer->cancel();
    }
    return true;
}

void SendFilesJob::initJobResult(BluezQt::InitObexManagerJob *job)
{
    if (job->error()) {
        qCWarning(SENDFILE) << "Error initializing obex manager" << job->errorText();
        setError(UserDefinedError);
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    // Create ObjectPush session
    QVariantMap map;
    map[QStringLiteral("Target")] = QStringLiteral("opp");
    BluezQt::PendingCall *call = job->manager()->createSession(m_device->address(), map);
    connect(call, &BluezQt::PendingCall::finished, this, &SendFilesJob::createSessionFinished);
}

void SendFilesJob::createSessionFinished(BluezQt::PendingCall *call)
{
    if (call->error()) {
        qCWarning(SENDFILE) << "Error creating session" << call->errorText();
        setError(UserDefinedError);
        setErrorText(call->errorText());
        emitResult();
        return;
    }

    m_objectPush = new BluezQt::ObexObjectPush(call->value().value<QDBusObjectPath>(), this);
    nextJob();
}

void SendFilesJob::doStart()
{
    qCDebug(SENDFILE) << "SendFilesJob-DoStart";

    setTotalAmount(Bytes, m_totalSize);
    setProcessedAmount(Bytes, 0);

    Q_EMIT description(this, i18n("Sending file over Bluetooth"),
                       QPair<QString, QString>(i18nc("File transfer origin", "From"), m_files.first()),
                       QPair<QString, QString>(i18nc("File transfer destination", "To"), m_device->name()));

    // Init BluezQt
    BluezQt::ObexManager *manager = new BluezQt::ObexManager(this);
    BluezQt::InitObexManagerJob *job = manager->init();
    job->start();
    connect(job, &BluezQt::InitObexManagerJob::result, this, &SendFilesJob::initJobResult);
}

void SendFilesJob::nextJob()
{
    qCDebug(SENDFILE) << "SendFilesJob-NextJob";

    m_transfer.clear();
    m_currentFile = m_files.takeFirst();
    m_currentFileSize = m_filesSizes.takeFirst();

    Q_EMIT description(this, i18n("Sending file over Bluetooth"),
                       QPair<QString, QString>(i18nc("File transfer origin", "From"), m_currentFile),
                       QPair<QString, QString>(i18nc("File transfer destination", "To"), m_device->name()));

    BluezQt::PendingCall *call = m_objectPush->sendFile(m_currentFile);
    connect(call, &BluezQt::PendingCall::finished, this, &SendFilesJob::sendFileFinished);
}

void SendFilesJob::sendFileFinished(BluezQt::PendingCall *call)
{
    if (call->error()) {
        qCWarning(SENDFILE) << "Error sending file" << call->errorText();
        setError(UserDefinedError);
        setErrorText(call->errorText());
        emitResult();
        return;
    }

    m_transfer = call->value().value<BluezQt::ObexTransferPtr>();
    connect(m_transfer.data(), &BluezQt::ObexTransfer::statusChanged, this, &SendFilesJob::statusChanged);
    connect(m_transfer.data(), &BluezQt::ObexTransfer::transferredChanged, this, &SendFilesJob::transferredChanged);

}

void SendFilesJob::jobDone()
{
    qCDebug(SENDFILE) << "SendFilesJob-JobDone";

    m_speedBytes = 0;
    m_currentFileSize = 0;
    m_currentFileProgress = 0;

    if (!m_files.isEmpty()) {
        nextJob();
        return;
    }

    emitResult();
}

void SendFilesJob::transferredChanged(quint64 transferred)
{
    // qCDebug(SENDFILE) << "SendFilesJob-Transferred" << transferred;

    // If at least 1 second has passed since last update
    int secondsSinceLastTime = m_time.secsTo(QTime::currentTime());
    if (secondsSinceLastTime > 0) {
        float speed = (transferred - m_speedBytes) / secondsSinceLastTime;
        emitSpeed(speed);

        m_time = QTime::currentTime();
        m_speedBytes = transferred;
    }

    progress(transferred);
}

void SendFilesJob::statusChanged(BluezQt::ObexTransfer::Status status)
{
    switch (status) {
    case BluezQt::ObexTransfer::Active:
        qCDebug(SENDFILE) << "SendFilesJob-Transfer Active";
        m_time = QTime::currentTime();
        break;

    case BluezQt::ObexTransfer::Complete:
        qCDebug(SENDFILE) << "SendFilesJob-Transfer Complete";
        jobDone();
        break;

    case BluezQt::ObexTransfer::Error:
        qCDebug(SENDFILE) << "SendFilesJob-Transfer Error";
        setError(UserDefinedError);
        setErrorText(i18n("Bluetooth transfer failed"));
        emitResult();
        break;

    default:
        qCWarning(SENDFILE) << "Not implemented status: " << status;
        break;
    }
}

void SendFilesJob::progress(quint64 transferBytes)
{
    quint64 toAdd = transferBytes - m_currentFileProgress;
    m_currentFileProgress = transferBytes;
    m_progress += toAdd;
    setProcessedAmount(Bytes, m_progress);
}
