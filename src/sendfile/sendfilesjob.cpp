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
#include "obexd_client.h"
#include "obexd_push.h"
#include "obexd_transfer.h"
#include "debug_p.h"

#include <KLocalizedString>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

SendFilesJob::SendFilesJob(const QStringList &files, Device *device, QObject *parent)
    : KJob(parent)
    , m_progress(0)
    , m_totalSize(0)
    , m_speedBytes(0)
    , m_device(device)
    , m_currentFileSize(0)
    , m_currentFileProgress(0)
{
    qCDebug(SENDFILE) << files;

    m_filesToSend = files;

    Q_FOREACH(const QString &filePath, files) {
        QFile file(filePath);
        m_filesToSendSize << file.size();
        m_totalSize += file.size();
    }

    setCapabilities(Killable);
}

bool SendFilesJob::doKill()
{
    return true;
}

void SendFilesJob::start()
{
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

void SendFilesJob::doStart()
{
    QVariantMap map;
    map[QStringLiteral("Target")] = QStringLiteral("opp");

    setTotalAmount(Bytes, m_totalSize);
    setProcessedAmount(Bytes, 0);

    emit description(this, i18n("Sending file over Bluetooth"),
                     QPair<QString, QString>(i18nc("File transfer origin", "From"), m_filesToSend.first()),
                     QPair<QString, QString>(i18nc("File transfer destination", "To"), m_device->name()));

    m_client = new OrgBluezObexClient1Interface(QStringLiteral("org.bluez.obex"),
                                                QStringLiteral("/org/bluez/obex"),
                                                QDBusConnection::sessionBus(),
                                                this);

    QDBusPendingReply <QDBusObjectPath > reply = m_client->CreateSession(m_device->address(), map);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &SendFilesJob::createSessionSlot);
}

void SendFilesJob::createSessionSlot(QDBusPendingCallWatcher *call)
{
    const QDBusPendingReply<QDBusObjectPath> reply = *call;
    call->deleteLater();
    if (reply.isError()) {
        qCDebug(SENDFILE) << "Error:";
        qCDebug(SENDFILE) << reply.error().name();
        qCDebug(SENDFILE) << reply.error().message();
        setError(KJob::UserDefinedError);
        setErrorText(i18n("Creating OBEX session failed"));
        emitResult();
        return;
    }

    QString path = reply.value().path();
    m_push = new OrgBluezObexObjectPush1Interface(QStringLiteral("org.bluez.obex"), path, QDBusConnection::sessionBus(), this);

    nextJob();
}

void SendFilesJob::sendFileSlot(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<QDBusObjectPath> reply = *watcher;
    watcher->deleteLater();
    QString path = reply.value().path();

    m_props = new OrgFreedesktopDBusPropertiesInterface(QStringLiteral("org.bluez.obex"), path, QDBusConnection::sessionBus(), this);
    connect(m_props, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged, this, &SendFilesJob::propertiesChangedSlot);
}

void SendFilesJob::propertiesChangedSlot(const QString& interface, const QVariantMap& props, const QStringList& invalidProps)
{
    qCDebug(SENDFILE) << interface;
    qCDebug(SENDFILE) << props;
    qCDebug(SENDFILE) << invalidProps;

    QStringList changedProps = props.keys();
    Q_FOREACH(const QString &prop, changedProps) {
        if (prop == QLatin1String("Status")) {
            statusChanged(props.value(prop));
        } else if (prop == QLatin1String("Transferred")) {
            transferChanged(props.value(prop));
        }
    }
}

void SendFilesJob::statusChanged(const QVariant& value)
{
    qCDebug(SENDFILE) << value;

    const QString &status = value.toString();

    if (status == QLatin1String("active")) {
        m_time = QTime::currentTime();
        return;
    } else if (status == QLatin1String("complete")) {
        jobDone();
        return;
    } else if (status == QLatin1String("error")) {
        setError(KJob::UserDefinedError);
        setErrorText(i18n("Bluetooth transfer failed"));
        emitResult();
        return;
    }

    qCDebug(SENDFILE) << "Not implemented status: " << status;
}

void SendFilesJob::transferChanged(const QVariant& value)
{
    qCDebug(SENDFILE) << value;

    bool ok = false;
    qulonglong bytes = value.toULongLong(&ok);
    if (!ok) {
        qCWarning(SENDFILE) << "Couldn't cast transferChanged value" << value;
        return;
    }

    // If a least 1 second has passed since last update
    int secondsSinceLastTime = m_time.secsTo(QTime::currentTime());
    if (secondsSinceLastTime > 0) {
        float speed = (bytes - m_speedBytes) / secondsSinceLastTime;
        emitSpeed(speed);

        m_time = QTime::currentTime();
        m_speedBytes = bytes;
    }

    progress(bytes);
}

void SendFilesJob::nextJob()
{
    qCDebug(SENDFILE);
    m_currentFile = m_filesToSend.takeFirst();
    m_currentFileSize = m_filesToSendSize.takeFirst();

    emit description(this, i18n("Sending file over Bluetooth"),
                     QPair<QString, QString>(i18nc("File transfer origin", "From"), m_currentFile),
                     QPair<QString, QString>(i18nc("File transfer destination", "To"), m_device->name()));

    QDBusPendingReply<QDBusObjectPath, QVariantMap> fileReply = m_push->SendFile(m_currentFile);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(fileReply);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &SendFilesJob::sendFileSlot);
}

void SendFilesJob::jobDone()
{
    m_speedBytes = 0;
    m_currentFileSize = 0;
    m_currentFileProgress = 0;

    if (!m_filesToSend.isEmpty()) {
        nextJob();
        return;
    }

    emitResult();
}

void SendFilesJob::progress(quint64 transferBytes)
{
    quint64 toAdd = transferBytes - m_currentFileProgress;
    m_currentFileProgress = transferBytes;
    m_progress += toAdd;
    setProcessedAmount(Bytes, m_progress);
}
