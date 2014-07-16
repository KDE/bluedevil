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
#include "filereceiversettings.h"
#include "debug_p.h"

#include <QIcon>
#include <QTemporaryFile>

#include <KIO/CopyJob>
#include <KJobTrackerInterface>
#include <KNotification>
#include <KLocalizedString>
#include <KIconLoader>

#include <QBluez/Manager>
#include <QBluez/InitManagerJob>
#include <QBluez/Adapter>
#include <QBluez/Device>
#include <QBluez/LoadDeviceJob>
#include <QBluez/ObexSession>

ReceiveFileJob::ReceiveFileJob(const QBluez::Request<QString> &req, QBluez::ObexTransfer *transfer, QObject* parent)
    : KJob(parent)
    , m_speedBytes(0)
    , m_request(req)
    , m_transfer(transfer)
{
    setCapabilities(Killable);
}

ReceiveFileJob::~ReceiveFileJob()
{
    m_transfer->deleteLater();
}

void ReceiveFileJob::start()
{
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

bool ReceiveFileJob::doKill()
{
    qCDebug(BLUEDAEMON) << "ReceiveFileJob-Kill";
    m_transfer->cancel();
    return true;
}

void ReceiveFileJob::init()
{
    qCDebug(BLUEDAEMON) << "ReceiveFileJob:";
    qCDebug(BLUEDAEMON) << "\tName:" << m_transfer->name();
    qCDebug(BLUEDAEMON) << "\tFilename:" << m_transfer->fileName();
    qCDebug(BLUEDAEMON) << "\tStatus:" << m_transfer->status();
    qCDebug(BLUEDAEMON) << "\tType:" << m_transfer->type();
    qCDebug(BLUEDAEMON) << "\tSize:" << m_transfer->size();
    qCDebug(BLUEDAEMON) << "\tTransferred:" << m_transfer->transferred();

    qCDebug(BLUEDAEMON) << "ObexSession:";
    qCDebug(BLUEDAEMON) << "\tSource:" << m_transfer->session()->source();
    qCDebug(BLUEDAEMON) << "\tDestination:" << m_transfer->session()->destination();

    connect(m_transfer, &QBluez::ObexTransfer::statusChanged, this, &ReceiveFileJob::statusChanged);
    connect(m_transfer, &QBluez::ObexTransfer::transferredChanged, this, &ReceiveFileJob::transferredChanged);

    m_deviceName = m_transfer->session()->destination();

    // We need to get device (session->destination) from adapter (session->source) for its name and paired property
    QBluez::Manager *manager = new QBluez::Manager(this);
    QBluez::InitManagerJob *job = manager->init(QBluez::Manager::InitManagerAndAdapters);
    job->start();
    connect(job, &QBluez::InitManagerJob::result, [ this, manager ](QBluez::InitManagerJob *job) {
        if (job->error()) {
            manager->deleteLater();
            showNotification();
            return;
        }
        QBluez::Adapter *adapter = job->manager()->adapterForAddress(m_transfer->session()->source());
        if (!adapter) {
            qCDebug(BLUEDAEMON) << "No adapter for" << m_transfer->session()->source();
            manager->deleteLater();
            showNotification();
            return;
        }
        QBluez::Device *device = adapter->deviceForAddress(m_transfer->session()->destination());
        if (!device) {
            qCDebug(BLUEDAEMON) << "No device for" << m_transfer->session()->destination();
            manager->deleteLater();
            showNotification();
            return;
        }
        QBluez::LoadDeviceJob *deviceJob = device->load();
        deviceJob->start();
        connect(deviceJob, &QBluez::LoadDeviceJob::result, [ this, manager ](QBluez::LoadDeviceJob *job) {
            if (job->error()) {
                manager->deleteLater();
                showNotification();
                return;
            }

            m_deviceName = job->device()->name();

            FileReceiverSettings::self()->load();
            switch (FileReceiverSettings::self()->autoAccept()) {
            case 0:
                // Never auto-accept transfers
                showNotification();
                break;

            case 1:
                // Auto-accept only from trusted devices
                if (job->device()->isTrusted()) {
                    qCDebug(BLUEDAEMON) << "Auto-accepting transfer for trusted device";
                    slotAccept();
                } else {
                    showNotification();
                }
                break;

            case 2:
                // Auto-accept all transfers
                qCDebug(BLUEDAEMON) << "Auto-accepting transfers for all devices";
                slotAccept();
                break;

            default:
                // Unknown
                showNotification();
                break;
            }

            manager->deleteLater();
        });
    });
}

void ReceiveFileJob::showNotification()
{
    KNotification *notification = new KNotification(QStringLiteral("bluedevilIncomingFile"),
        KNotification::Persistent, this);

    notification->setText(i18nc(
        "Show a notification asking to authorize or deny an incoming file transfer to this computer from a Bluetooth device.",
        "%1 is sending you the file %2", m_deviceName, m_transfer->name()));

    QStringList actions;

    actions.append(i18nc("Button to accept the incoming file transfer and download it in the default download directory", "Accept"));
    actions.append(i18nc("Deny the incoming file transfer", "Cancel"));

    notification->setActions(actions);

    connect(notification, &KNotification::action1Activated, this, &ReceiveFileJob::slotAccept);
    connect(notification, &KNotification::action2Activated, this, &ReceiveFileJob::slotCancel);
    connect(notification, &KNotification::closed, this, &ReceiveFileJob::slotCancel);

    int size = IconSize(KIconLoader::Desktop);
    notification->setPixmap(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")).pixmap(size, size));
    notification->setComponentName(QStringLiteral("bluedevil"));
    notification->sendEvent();
}

void ReceiveFileJob::slotAccept()
{
    qCDebug(BLUEDAEMON) << "ReceiveFileJob-Accept";
    KIO::getJobTracker()->registerJob(this);

    m_originalFileName = m_transfer->name();
    m_tempPath = createTempPath(m_transfer->name());
    qCDebug(BLUEDAEMON) << "TempPath:" << m_tempPath;

    m_request.accept(m_tempPath);
}

void ReceiveFileJob::slotSaveAs()
{
    QTemporaryFile tmpFile;
    tmpFile.open();
    qCDebug(BLUEDAEMON) << "SaveAs:" << tmpFile.fileName();

    m_request.accept(tmpFile.fileName());
}

void ReceiveFileJob::slotCancel()
{
    if (m_transfer->status() == QBluez::ObexTransfer::Queued) {
        qCDebug(BLUEDAEMON) << "Cancel Push";
        m_request.cancel();
    }
}

void ReceiveFileJob::moveFinished(KJob *job)
{
    if (job->error()) {
        qCDebug(BLUEDAEMON) << job->error();
        qCDebug(BLUEDAEMON) << job->errorText();
        setError(job->error());
        setErrorText(QStringLiteral("Error in KIO::move"));
    }

    emitResult();
}

void ReceiveFileJob::statusChanged(QBluez::ObexTransfer::Status status)
{
    qCDebug(BLUEDAEMON) << "ReceiveFileJob-StatusChanged" << status;

    FileReceiverSettings::self()->load();
    QUrl savePath = FileReceiverSettings::self()->saveUrl().adjusted(QUrl::StripTrailingSlash);
    savePath.setPath(savePath.path() + QLatin1Char('/') + m_originalFileName);

    switch (status) {
    case QBluez::ObexTransfer::Active:
        Q_EMIT description(this, i18n("Receiving file over Bluetooth"),
                        QPair<QString, QString>(i18nc("File transfer origin", "From"), QString(m_deviceName)),
                        QPair<QString, QString>(i18nc("File transfer destination", "To"), savePath.path()));

        setTotalAmount(Bytes, m_transfer->size());
        setProcessedAmount(Bytes, 0);
        m_time = QTime::currentTime();
        break;

    case QBluez::ObexTransfer::Complete: {
        KIO::CopyJob *job = KIO::move(QUrl::fromLocalFile(m_tempPath), savePath, KIO::HideProgressInfo);
        job->setUiDelegate(0);
        connect(job, &KIO::CopyJob::finished, this, &ReceiveFileJob::moveFinished);
        break;
    }

    case QBluez::ObexTransfer::Error:
        setError(KJob::UserDefinedError);
        emitResult();
        break;

    default:
        qCDebug(BLUEDAEMON) << "Not implemented status: " << status;
        break;
    }
}

void ReceiveFileJob::transferredChanged(quint64 transferred)
{
    qCDebug(BLUEDAEMON) << "ReceiveFileJob-Transferred" << transferred;

    // If a least 1 second has passed since last update
    int secondsSinceLastTime = m_time.secsTo(QTime::currentTime());
    if (secondsSinceLastTime > 0) {
        unsigned long speed = (transferred - m_speedBytes) / secondsSinceLastTime;
        emitSpeed(speed);

        m_time = QTime::currentTime();
        m_speedBytes = transferred;
    }

    setProcessedAmount(Bytes, transferred);
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
