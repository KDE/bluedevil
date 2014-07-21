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

#include <KLocalizedString>

#include <QBluez/Device>
#include <QBluez/PendingCall>
#include <QBluez/ObexManager>
#include <QBluez/ObexObjectPush>
#include <QBluez/InitObexManagerJob>

SendFilesJob::SendFilesJob(const QStringList &files, QBluez::Device *device, QObject *parent)
    : KJob(parent)
    , m_filesToSend(files)
    , m_progress(0)
    , m_totalSize(0)
    , m_speedBytes(0)
    , m_currentFileSize(0)
    , m_currentFileProgress(0)
    , m_device(device)
    , m_transfer(0)
    , m_objectPush(0)
{
    qCDebug(SENDFILE) << "SendFilesJob:" << m_filesToSend;

    Q_FOREACH (const QString &filePath, m_filesToSend) {
        QFile file(filePath);
        m_filesToSendSize << file.size();
        m_totalSize += file.size();
    }

    setCapabilities(Killable);
}

bool SendFilesJob::doKill()
{
    if (m_transfer) {
        m_transfer->cancel();
    }
    return true;
}

void SendFilesJob::start()
{
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

void SendFilesJob::doStart()
{
    qCDebug(SENDFILE) << "SendFilesJob-DoStart";

    setTotalAmount(Bytes, m_totalSize);
    setProcessedAmount(Bytes, 0);

    emit description(this, i18n("Sending file over Bluetooth"), QPair<QString, QString>(i18nc("File transfer origin", "From"), m_filesToSend.first()), QPair<QString, QString>(i18nc("File transfer destination", "To"), m_device->name()));

    // Init QBluez
    QBluez::ObexManager *manager = new QBluez::ObexManager(this);
    QBluez::InitObexManagerJob *job = manager->init();
    job->start();
    connect(job, &QBluez::InitObexManagerJob::result, [ this ](QBluez::InitObexManagerJob *job) {
        if (job->error()) {
            qCDebug(SENDFILE) << "Error initializing obex manager" << job->errorText();
            setError(UserDefinedError);
            setErrorText(job->errorText());
            emitResult();
            return;
        }

        // Create ObjectPush session
        QVariantMap map;
        map["Target"] = "opp";
        QBluez::PendingCall *call = job->manager()->createSession(m_device->address(), map);
        connect(call, &QBluez::PendingCall::finished, [ this ](QBluez::PendingCall *call) {
            if (call->error()) {
                qCDebug(SENDFILE) << "Error creating session" << call->errorText();
                setError(UserDefinedError);
                setErrorText(call->errorText());
                emitResult();
                return;
            }

            m_objectPush = new QBluez::ObexObjectPush(call->value().value<QDBusObjectPath>(), this);
            nextJob();
        });
    });
}

void SendFilesJob::nextJob()
{
    qCDebug(SENDFILE) << "SendFilesJob-NextJob";

    if (m_transfer) {
        m_transfer->deleteLater();
        m_transfer = 0;
    }

    m_currentFile = m_filesToSend.takeFirst();
    m_currentFileSize = m_filesToSendSize.takeFirst();

    emit description(this, i18n("Sending file over Bluetooth"), QPair<QString, QString>(i18nc("File transfer origin", "From"), m_currentFile), QPair<QString, QString>(i18nc("File transfer destination", "To"), m_device->name()));

    QBluez::PendingCall *call = m_objectPush->sendFile(m_currentFile);
    connect(call, &QBluez::PendingCall::finished, [ this ](QBluez::PendingCall *call) {
        if (call->error()) {
            qCDebug(SENDFILE) << "Error sending file" << call->errorText();
            setError(UserDefinedError);
            setErrorText(call->errorText());
            emitResult();
            return;
        }

        m_transfer = call->value().value<QBluez::ObexTransfer*>();
        connect(m_transfer, &QBluez::ObexTransfer::statusChanged, this, &SendFilesJob::statusChanged);
        connect(m_transfer, &QBluez::ObexTransfer::transferredChanged, this, &SendFilesJob::transferredChanged);
    });
}

void SendFilesJob::jobDone()
{
    qCDebug(SENDFILE) << "SendFilesJob-JobDone";

    m_speedBytes = 0;
    m_currentFileSize = 0;
    m_currentFileProgress = 0;

    if (!m_filesToSend.isEmpty()) {
        nextJob();
        return;
    }

    emitResult();
}

void SendFilesJob::transferredChanged(quint64 transferred)
{
    qCDebug(SENDFILE) << "SendFilesJob-Transferred" << transferred;

    // If a least 1 second has passed since last update
    int secondsSinceLastTime = m_time.secsTo(QTime::currentTime());
    if (secondsSinceLastTime > 0) {
        float speed = (transferred - m_speedBytes) / secondsSinceLastTime;
        emitSpeed(speed);

        m_time = QTime::currentTime();
        m_speedBytes = transferred;
    }

    progress(transferred);
}

void SendFilesJob::statusChanged(QBluez::ObexTransfer::Status status)
{
    qCDebug(SENDFILE) << "SendFilesJob-StatusChanged" << status;

    switch (status) {
    case QBluez::ObexTransfer::Active:
        m_time = QTime::currentTime();
        break;

    case QBluez::ObexTransfer::Complete:
        jobDone();
        break;

    case QBluez::ObexTransfer::Error:
        setError(UserDefinedError);
        emitResult();
        break;

    default:
        qCDebug(SENDFILE) << "Not implemented status: " << status;
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
