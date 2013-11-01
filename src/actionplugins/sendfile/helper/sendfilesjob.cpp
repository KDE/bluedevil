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
#include "obexagent.h"
#include "obex_transfer.h"
#include "obex_client.h"
#include "obexd_client.h"
#include "obexd_push.h"
#include "obexd_transfer.h"

#include <KLocalizedString>
#include <KDebug>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;
SendFilesJob::SendFilesJob(const QStringList& files, Device* device, ObexAgent* agent, QObject* parent): KJob(parent)
,m_currentTransferJob(0)
{
    kDebug() << files;
    m_filesToSend = files;
    m_agent = agent;
    m_device = device;

    m_totalSize = 0;
    m_progress = 0;

    Q_FOREACH(const QString &filePath, files) {
        QFile file(filePath);
        kDebug() << "Adding size : " << file.size();
        m_filesToSendSize << file.size();
        m_totalSize += file.size();
    }

    connect(m_agent, SIGNAL(request(OrgOpenobexTransferInterface *)), this, SLOT(nextJob(OrgOpenobexTransferInterface*)));
    connect(m_agent, SIGNAL(completed(QDBusObjectPath)), this, SLOT(jobDone(QDBusObjectPath)));
    connect(m_agent, SIGNAL(progress(QDBusObjectPath, quint64)), this, SLOT(progress(QDBusObjectPath, quint64)));
    connect(m_agent, SIGNAL(error(QDBusObjectPath, QString)), this, SLOT(error(QDBusObjectPath, QString)));

    setCapabilities(Killable);
}

bool SendFilesJob::doKill()
{
    if(m_currentTransferJob) {
        m_currentTransferJob->Cancel();
    }
    m_agent->setKilled();
    return true;
}


void SendFilesJob::start()
{
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

void SendFilesJob::doStart()
{
    kDebug();
    QVariantMap map;
    map["Target"] = "opp";

    setTotalAmount(Bytes, m_totalSize);
    setProcessedAmount(Bytes, 0);

    emit description(this, i18n("Sending file over Bluetooth"), QPair<QString, QString>(i18nc("File transfer origin", "From"), m_filesToSend.first()), QPair<QString, QString>(i18nc("File transfer destination", "To"), m_device->name()));

    m_client = new OrgBluezObexClient1Interface("org.bluez.obex", "/org/bluez/obex", QDBusConnection::sessionBus(), this);

    QDBusPendingReply <QDBusObjectPath > reply = m_client->CreateSession(m_device->address(), map);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(createSessionSlot(QDBusPendingCallWatcher*)));
}

void SendFilesJob::createSessionSlot(QDBusPendingCallWatcher *call)
{
    const QDBusPendingReply<QDBusObjectPath> reply = *call;
    call->deleteLater();
    if (reply.isError()) {
        kDebug() << "Error:";
        kDebug() << reply.error().name();
        kDebug() << reply.error().message();
        setError(-1);
        emitResult();
        return;
    }

    QString path = reply.value().path();
    m_push = new OrgBluezObexObjectPush1Interface("org.bluez.obex", path, QDBusConnection::sessionBus(), this);
    QDBusPendingReply<QDBusObjectPath, QVariantMap> fileReply = m_push->SendFile(m_filesToSend.first());

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(fileReply);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(sendFileSlot(QDBusPendingCallWatcher*)));
}

void SendFilesJob::sendFileSlot(QDBusPendingCallWatcher* watcher)
{
    const QDBusPendingReply<QDBusObjectPath> reply = *watcher;
    watcher->deleteLater();
    QString path = reply.value().path();

    m_props = new OrgFreedesktopDBusPropertiesInterface("org.bluez.obex", path, QDBusConnection::sessionBus(), this);
    connect(m_props, SIGNAL(PropertiesChanged(QString,QVariantMap,QStringList)), SLOT(propertiesChangedSlot(QString,QVariantMap,QStringList)));
}

void SendFilesJob::propertiesChangedSlot(const QString& interface, const QVariantMap& props, const QStringList& invalidProps)
{
    kDebug() << interface;
    kDebug() << props;
    kDebug() << invalidProps;

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
    kDebug() << value;
    QString status = value.toString();

    if (status == QLatin1String("active")) {
        m_time = QTime::currentTime();
        return;
    } else if (status == QLatin1String("complete")) {
        jobDone();
        return;
    } else if (status == QLatin1String("error")) {
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }

    kDebug() << "Not implemented status: " << status;
}

void SendFilesJob::transferChanged(const QVariant& value)
{
    kDebug() << value;
    bool ok = false;
    qulonglong bytes = value.toULongLong(&ok);
    if (!ok) {
        kWarning() << "Couldn't cast transferChanged value" << value;
        return;
    }

    //If a least 1 second has passed since last update
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
    kDebug();
    m_currentFile = m_filesToSend.takeFirst();
    m_currentFileProgress = 0;
    m_currentFileSize = m_filesToSendSize.takeFirst();

    emit description(this, i18n("Sending file over Bluetooth"), QPair<QString, QString>(i18nc("File transfer origin", "From"), m_currentFile), QPair<QString, QString>(i18nc("File transfer destination", "To"), m_device->name()));

    QDBusPendingReply<QDBusObjectPath, QVariantMap> fileReply = m_push->SendFile(m_filesToSend.first());

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(fileReply);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(sendFileSlot(QDBusPendingCallWatcher*)));
}

void SendFilesJob::jobDone()
{
    m_currentFileProgress = 0;
    m_currentFileSize = 0;
    if (!m_filesToSend.isEmpty()) {
        nextJob();
        return;
    }

    emitResult();
}

void SendFilesJob::progress(quint64 transferBytes)
{
    kDebug();

    quint64 toAdd = transferBytes - m_currentFileProgress;
    m_currentFileProgress = transferBytes;
    m_progress += toAdd;
    setProcessedAmount(Bytes, m_progress);
}

void SendFilesJob::error(QDBusObjectPath transfer, const QString& error)
{
    kDebug() << error;

    //if this is the last file, do not complete it
    if (!m_filesToSend.isEmpty()) {
        quint64 toAdd = m_currentFileSize - m_currentFileProgress;
        m_progress += toAdd;
        setProcessedAmount(Bytes, m_progress);
    }

//     jobDone(transfer);
}
