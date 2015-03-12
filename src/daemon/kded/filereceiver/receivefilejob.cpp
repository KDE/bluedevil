/*************************************************************************************
 *  Copyright (C) 2013 by Alejandro Fiestas Fiestas <afiestas@kde.org>               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "receivefilejob.h"
#include "../BlueDevilDaemon.h"
#include "filereceiversettings.h"
#include "obex_transfer.h"
#include "obex_session.h"
#include "dbus_properties.h"

#include <QIcon>
#include <QDebug>
#include <QDBusConnection>
#include <QTemporaryFile>

#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>
#include <bluedevil/bluedevildevice.h>

#include <KIO/Job>
#include <KIO/Global>
#include <KIO/CopyJob>
#include <KJobTrackerInterface>
#include <KNotification>
#include <KLocalizedString>
#include <KIconLoader>

using namespace BlueDevil;

ReceiveFileJob::ReceiveFileJob(const QDBusMessage &msg, const QString &path, QObject *parent)
    : KJob(parent)
    , m_speedBytes(0)
    , m_path(path)
    , m_msg(msg)
{
    setCapabilities(Killable);
}

void ReceiveFileJob::start()
{
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

bool ReceiveFileJob::doKill()
{
    m_transfer->Cancel();
    return true;
}

void ReceiveFileJob::init()
{
    m_transfer = new org::bluez::obex::Transfer1(QStringLiteral("org.bluez.obex"),
                                                 m_path,
                                                 QDBusConnection::sessionBus(),
                                                 this);
    qCDebug(BLUEDAEMON) << m_transfer->name();
    qCDebug(BLUEDAEMON) << m_transfer->filename();
    qCDebug(BLUEDAEMON) << m_transfer->status();
    qCDebug(BLUEDAEMON) << m_transfer->type();
    qCDebug(BLUEDAEMON) << m_transfer->size();
    qCDebug(BLUEDAEMON) << m_transfer->transferred();

    m_transferProps = new org::freedesktop::DBus::Properties(QStringLiteral("org.bluez.obex"),
                                                             m_path,
                                                             QDBusConnection::sessionBus(),
                                                             this);
    connect(m_transferProps, &org::freedesktop::DBus::Properties::PropertiesChanged,
            this, &ReceiveFileJob::transferPropertiesChanged);

    m_session = new org::bluez::obex::Session1(QStringLiteral("org.bluez.obex"),
                                               m_transfer->session().path(),
                                               QDBusConnection::sessionBus(),
                                               this);

    qCDebug(BLUEDAEMON) << m_session->destination();

    Device *device = 0;
    bool isDeviceTrusted = false;

    Q_FOREACH (Adapter *adapter, Manager::self()->adapters()) {
        if (adapter->address() == m_session->source()) {
            device = adapter->deviceForAddress(m_session->destination());
            break;
        }
    }

    qCDebug(BLUEDAEMON) << device;

    m_deviceName = m_session->destination();

    if (device) {
        qCDebug(BLUEDAEMON) << device->name();
        m_deviceName = device->name();
        isDeviceTrusted = device->isTrusted();
    }

    FileReceiverSettings::self()->load();
    qCDebug(BLUEDAEMON) << "Auto Accept: " << FileReceiverSettings::self()->autoAccept();

    if (FileReceiverSettings::self()->autoAccept() == 1 && isDeviceTrusted) {
        slotAccept();
        return;
    } else if (FileReceiverSettings::self()->autoAccept() == 2) {
        slotAccept();
        return;
    }

    showNotification();
}

void ReceiveFileJob::showNotification()
{
    KNotification *m_notification = new KNotification(QStringLiteral("bluedevilIncomingFile"),
        KNotification::Persistent, this);

    m_notification->setText(i18nc(
        "Show a notification asking to authorize or deny an incoming file transfer to this computer from a Bluetooth device.",
        "%1 is sending you the file %2", m_deviceName, m_transfer->name()));

    QStringList actions;

    actions.append(i18nc("Button to accept the incoming file transfer and download it in the default download directory", "Accept"));
    actions.append(i18nc("Deny the incoming file transfer", "Cancel"));

    m_notification->setActions(actions);

    connect(m_notification, &KNotification::action1Activated, this, &ReceiveFileJob::slotAccept);
    connect(m_notification, &KNotification::action2Activated, this, &ReceiveFileJob::slotCancel);
    connect(m_notification, &KNotification::closed, this, &ReceiveFileJob::slotCancel);

    int size = IconSize(KIconLoader::Desktop);
    m_notification->setPixmap(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")).pixmap(size, size));
    m_notification->setComponentName(QStringLiteral("bluedevil"));
    m_notification->sendEvent();
}

void ReceiveFileJob::slotAccept()
{
    KIO::getJobTracker()->registerJob(this);

    m_originalFileName = m_transfer->name();
    m_tempPath = createTempPath(m_transfer->name());

    qCDebug(BLUEDAEMON) << m_tempPath;

    QDBusMessage msg = m_msg.createReply(m_tempPath);
    QDBusConnection::sessionBus().send(msg);
}

void ReceiveFileJob::slotSaveAs()
{
    QTemporaryFile tmpFile;
    tmpFile.open();

    QDBusConnection::sessionBus().send(m_msg.createReply(tmpFile.fileName()));
    qCDebug(BLUEDAEMON) << tmpFile.fileName();
}

void ReceiveFileJob::slotCancel()
{
    QDBusMessage msg = m_msg.createErrorReply(QStringLiteral("org.bluez.obex.Error.Rejected"),
                                              QStringLiteral("org.bluez.obex.Error.Rejected"));
    QDBusConnection::sessionBus().send(msg);
}

void ReceiveFileJob::transferPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProperties)
{
    qCDebug(BLUEDAEMON) << interface;
    qCDebug(BLUEDAEMON) << properties;
    qCDebug(BLUEDAEMON) << invalidatedProperties;

    QStringList changedProps = properties.keys();
    Q_FOREACH(const QString &prop, changedProps) {
        if (prop == QLatin1String("Status")) {
            statusChanged(properties.value(prop));
        } else if (prop == QLatin1String("Transferred")) {
            transferChanged(properties.value(prop));
        }
    }
}

void ReceiveFileJob::statusChanged(const QVariant &value)
{
    qCDebug(BLUEDAEMON) << value;

    QString status = value.toString();

    FileReceiverSettings::self()->load();
    QUrl savePath = FileReceiverSettings::self()->saveUrl().adjusted(QUrl::StripTrailingSlash);
    savePath.setPath(savePath.path() + QLatin1Char('/') + m_originalFileName);

    if (status == QLatin1String("active")) {
        Q_EMIT description(this, i18n("Receiving file over Bluetooth"),
                           QPair<QString, QString>(i18nc("File transfer origin", "From"),
                           QString(m_deviceName)),
                           QPair<QString, QString>(i18nc("File transfer destination", "To"), savePath.path()));

        setTotalAmount(Bytes, m_transfer->size());
        setProcessedAmount(Bytes, 0);
        m_time = QTime::currentTime();
        return;
    } else if (status == QLatin1String("complete")) {
        KIO::CopyJob* job = KIO::move(QUrl::fromLocalFile(m_tempPath), savePath, KIO::HideProgressInfo);
        job->setUiDelegate(0);
        connect(job, &KIO::CopyJob::finished, this, &ReceiveFileJob::moveFinished);
        return;
    } else if (status == QLatin1String("error")) {
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }

    qCDebug(BLUEDAEMON) << "Not implemented status: " << status;
}

void ReceiveFileJob::transferChanged(const QVariant &value)
{
    qCDebug(BLUEDAEMON) << value;

    bool ok = false;
    qulonglong bytes = value.toULongLong(&ok);
    if (!ok) {
        qCWarning(BLUEDAEMON) << "Couldn't cast transferChanged value" << value;
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

    setProcessedAmount(Bytes, bytes);
}

void ReceiveFileJob::moveFinished(KJob *job)
{
    if (job->error()) {
        qCDebug(BLUEDAEMON) << job->error();
        qCDebug(BLUEDAEMON) << job->errorText();
        setError(job->error());
        QFile::remove(m_tempPath);
    }

    // Delay emitResult to make sure notification is displayed even
    // for very small files that are received instantly
    QTimer::singleShot(500, this, [this]() {
        emitResult();
    });
}

QString ReceiveFileJob::createTempPath(const QString &fileName) const
{
    QString xdgCacheHome = QFile::decodeName(qgetenv("XDG_CACHE_HOME"));
    if (xdgCacheHome.isEmpty()) {
        xdgCacheHome = QDir::homePath() + QLatin1String("/.cache");
    }

    xdgCacheHome.append(QLatin1String("/obexd/"));
    QString path = xdgCacheHome + fileName;

    int i = 0;
    while (QFile::exists(path)) {
        path = xdgCacheHome + fileName + QString::number(i);
        i++;
    }

    return path;
}
