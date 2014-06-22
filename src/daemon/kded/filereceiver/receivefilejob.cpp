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

#include <KIcon>
#include <QDBusConnection>

#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>
#include <bluedevil/bluedevildevice.h>

#include <KDebug>
#include <KIO/Job>
#include <kio/copyjob.h>
#include <kio/global.h>
#include <kjobtrackerinterface.h>
#include <KIconLoader>
#include <KNotification>
#include <KTemporaryFile>
#include <KLocalizedString>

using namespace BlueDevil;

ReceiveFileJob::ReceiveFileJob(const QDBusMessage& msg, const QString &path, const KComponentData &componentData, QObject* parent)
    : KJob(parent)
    , m_speedBytes(0)
    , m_path(path)
    , m_msg(msg)
    , m_componentData(componentData)
{
    setCapabilities(Killable);
}

ReceiveFileJob::~ReceiveFileJob()
{

}

void ReceiveFileJob::start()
{
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

bool ReceiveFileJob::doKill()
{
    kDebug(dblue());
    m_transfer->Cancel();
    return true;
}

void ReceiveFileJob::init()
{
    m_transfer = new org::bluez::obex::Transfer1("org.bluez.obex", m_path, QDBusConnection::sessionBus(), this);
    kDebug(dblue()) << m_transfer->name();
    kDebug(dblue()) << m_transfer->filename();
    kDebug(dblue()) << m_transfer->status();
    kDebug(dblue()) << m_transfer->type();
    kDebug(dblue()) << m_transfer->size();
    kDebug(dblue()) << m_transfer->transferred();

    m_transferProps = new org::freedesktop::DBus::Properties("org.bluez.obex", m_path, QDBusConnection::sessionBus(), this);
    connect(m_transferProps,
            SIGNAL(PropertiesChanged(QString,QVariantMap,QStringList)),
            SLOT(transferPropertiesChanged(QString,QVariantMap,QStringList)));

    m_session = new org::bluez::obex::Session1("org.bluez.obex", m_transfer->session().path(), QDBusConnection::sessionBus(), this);
    kDebug(dblue()) << m_session->destination();

    Device* device = Manager::self()->usableAdapter()->deviceForAddress(m_session->destination());
    kDebug(dblue()) << device;

    m_deviceName = m_session->destination();
    if (device) {
        kDebug(dblue()) << device->name();
        m_deviceName = device->name();
    }

    FileReceiverSettings::self()->readConfig();
    kDebug(dblue()) << "Auto Accept: " << FileReceiverSettings::self()->autoAccept();
    if (FileReceiverSettings::self()->autoAccept() == 1 && device->isTrusted()) {
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
    KNotification *m_notification = new KNotification("bluedevilIncomingFile",
        KNotification::Persistent, this);

    m_notification->setText(i18nc(
        "Show a notification asking to authorize or deny an incoming file transfer to this computer from a Bluetooth device.",
        "%1 is sending you the file %2", m_deviceName, m_transfer->name()));

    QStringList actions;

    actions.append(i18nc("Button to accept the incoming file transfer and download it in the default download directory", "Accept"));
    actions.append(i18nc("Deny the incoming file transfer", "Cancel"));

    m_notification->setActions(actions);

    connect(m_notification, SIGNAL(action1Activated()), SLOT(slotAccept()));
    connect(m_notification, SIGNAL(action2Activated()), SLOT(slotCancel()));
    connect(m_notification, SIGNAL(closed()), SLOT(slotCancel()));

    int size = IconSize(KIconLoader::Desktop);
    m_notification->setPixmap(KIcon("preferences-system-bluetooth").pixmap(size, size));
    m_notification->setComponentData(KComponentData("bluedevil"));
    m_notification->sendEvent();
}

void ReceiveFileJob::slotAccept()
{
    kDebug(dblue());
    KComponentData data = KGlobal::mainComponent();
    KGlobal::setActiveComponent(m_componentData);
    KIO::getJobTracker()->registerJob(this);
    KGlobal::setActiveComponent(data);

    m_originalFileName = m_transfer->name();
    m_tempPath = createTempPath(m_transfer->name());
    kDebug(dblue()) << m_tempPath;
    QDBusMessage msg = m_msg.createReply(m_tempPath);
    QDBusConnection::sessionBus().send(msg);
}

void ReceiveFileJob::slotSaveAs()
{
    KTemporaryFile tmpFile;
    tmpFile.open();
    tmpFile.close();

    QDBusConnection::sessionBus().send(m_msg.createReply(tmpFile.fileName()));
    kDebug(dblue()) << tmpFile.fileName();
}

void ReceiveFileJob::slotCancel()
{
    kDebug(dblue());
    QDBusMessage msg = m_msg.createErrorReply("org.bluez.obex.Error.Rejected", "org.bluez.obex.Error.Rejected");
    QDBusConnection::sessionBus().send(msg);
}

void ReceiveFileJob::transferPropertiesChanged(const QString& interface, const QVariantMap& properties, const QStringList& invalidatedProperties)
{
    kDebug(dblue()) << interface;
    kDebug(dblue()) << properties;
    kDebug(dblue()) << invalidatedProperties;

    QStringList changedProps = properties.keys();
    Q_FOREACH(const QString &prop, changedProps) {
        if (prop == QLatin1String("Status")) {
            statusChanged(properties.value(prop));
        } else if (prop == QLatin1String("Transferred")) {
            transferChanged(properties.value(prop));
        }
    }
}

void ReceiveFileJob::statusChanged(const QVariant& value)
{
    kDebug(dblue()) << value;
    QString status = value.toString();

    FileReceiverSettings::self()->readConfig();
    KUrl savePath = FileReceiverSettings::self()->saveUrl();
    savePath.addPath(m_originalFileName);

    if (status == QLatin1String("active")) {
        emit description(this, i18n("Receiving file over Bluetooth"),
                        QPair<QString, QString>(i18nc("File transfer origin", "From"),
                        QString(m_deviceName)),
                        QPair<QString, QString>(i18nc("File transfer destination", "To"), savePath.path()));

        setTotalAmount(Bytes, m_transfer->size());
        setProcessedAmount(Bytes, 0);
        m_time = QTime::currentTime();
        return;
    } else if (status == QLatin1String("complete")) {
        KIO::CopyJob* job = KIO::move(KUrl(m_tempPath), KUrl(savePath), KIO::HideProgressInfo);
        job->setUiDelegate(0);
        connect(job, SIGNAL(finished(KJob*)), SLOT(moveFinished(KJob*)));
        return;
    } else if (status == QLatin1String("error")) {
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }

    kDebug(dblue()) << "Not implemented status: " << status;
}

void ReceiveFileJob::transferChanged(const QVariant& value)
{
    kDebug(dblue()) << value;
    bool ok = false;
    qulonglong bytes = value.toULongLong(&ok);
    if (!ok) {
        kWarning(dblue()) << "Couldn't cast transferChanged value" << value;
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

    setProcessedAmount(Bytes, bytes);
}

void ReceiveFileJob::moveFinished(KJob* job)
{
    if (job->error()) {
        kDebug(dblue()) << job->error();
        kDebug(dblue()) << job->errorText();
        setError(job->error());
        setErrorText("Error in KIO::move");
    }

    emitResult();
}

QString ReceiveFileJob::createTempPath(const QString &fileName) const
{
    QString xdgCacheHome = QFile::decodeName(qgetenv("XDG_CACHE_HOME"));
    if (xdgCacheHome.isEmpty()) {
            xdgCacheHome = QDir::homePath() + QLatin1String("/.cache");
    }

    xdgCacheHome.append(QLatin1String("/obexd/"));
    QString path =  xdgCacheHome + fileName;
    int i = 0;

    while (QFile::exists(path)) {
        path = xdgCacheHome + fileName + QString::number(i);
        i++;
    }

    return path;
}
