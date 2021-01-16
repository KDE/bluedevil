/*
 *  SPDX-FileCopyrightText: 2013 Alejandro Fiestas Fiestas <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "receivefilejob.h"
#include "debug_p.h"
#include "filereceiversettings.h"
#include "obexagent.h"

#include <QDir>
#include <QIcon>
#include <QTemporaryFile>
#include <QTimer>

#include <KIO/CopyJob>
#include <KJobTrackerInterface>
#include <KLocalizedString>
#include <KNotification>

#include <BluezQt/Adapter>
#include <BluezQt/Device>
#include <BluezQt/Manager>
#include <BluezQt/ObexSession>

ReceiveFileJob::ReceiveFileJob(const BluezQt::Request<QString> &req, BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, ObexAgent *parent)
    : KJob(parent)
    , m_speedBytes(0)
    , m_agent(parent)
    , m_transfer(transfer)
    , m_session(session)
    , m_request(req)
    , m_accepted(false)
{
    setCapabilities(Killable);
}

QString ReceiveFileJob::deviceAddress() const
{
    return m_deviceAddress;
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
    qCDebug(BLUEDAEMON) << "\tSource:" << m_session->source();
    qCDebug(BLUEDAEMON) << "\tDestination:" << m_session->destination();

    connect(m_transfer.data(), &BluezQt::ObexTransfer::statusChanged, this, &ReceiveFileJob::statusChanged);
    connect(m_transfer.data(), &BluezQt::ObexTransfer::transferredChanged, this, &ReceiveFileJob::transferredChanged);

    m_deviceName = m_session->destination();

    BluezQt::AdapterPtr adapter = m_agent->manager()->adapterForAddress(m_session->source());
    if (!adapter) {
        qCDebug(BLUEDAEMON) << "No adapter for" << m_session->source();
        showNotification();
        return;
    }

    BluezQt::DevicePtr device = adapter->deviceForAddress(m_session->destination());
    if (!device) {
        qCDebug(BLUEDAEMON) << "No device for" << m_session->destination();
        showNotification();
        return;
    }

    m_deviceName = device->name();
    m_deviceAddress = device->address();

    if (m_agent->shouldAutoAcceptTransfer(m_deviceAddress)) {
        slotAccept();
        return;
    }

    FileReceiverSettings::self()->load();
    switch (FileReceiverSettings::self()->autoAccept()) {
    case 0: // Never auto-accept transfers
        showNotification();
        break;

    case 1: // Auto-accept only from trusted devices
        if (device->isTrusted()) {
            qCDebug(BLUEDAEMON) << "Auto-accepting transfer for trusted device";
            slotAccept();
        } else {
            showNotification();
        }
        break;

    case 2: // Auto-accept all transfers
        qCDebug(BLUEDAEMON) << "Auto-accepting transfers for all devices";
        slotAccept();
        break;

    default: // Unknown
        showNotification();
        break;
    }
}

void ReceiveFileJob::showNotification()
{
    KNotification *notification = new KNotification(QStringLiteral("IncomingFile"), KNotification::Persistent, this);

    notification->setTitle(QStringLiteral("%1 (%2)").arg(m_deviceName.toHtmlEscaped(), m_deviceAddress));
    notification->setText(i18nc("Show a notification asking to authorize or deny an incoming file transfer to this computer from a Bluetooth device.",
                                "%1 is sending you the file %2",
                                m_deviceName.toHtmlEscaped(),
                                m_transfer->name()));

    QStringList actions;
    actions.append(i18nc("Button to accept the incoming file transfer and download it in the default download directory", "Accept"));
    actions.append(i18nc("Deny the incoming file transfer", "Cancel"));

    notification->setActions(actions);

    connect(notification, &KNotification::action1Activated, this, &ReceiveFileJob::slotAccept);
    connect(notification, &KNotification::action2Activated, this, &ReceiveFileJob::slotCancel);
    connect(notification, &KNotification::closed, this, &ReceiveFileJob::slotCancel);

    notification->setComponentName(QStringLiteral("bluedevil"));

    notification->sendEvent();
}

void ReceiveFileJob::slotAccept()
{
    qCDebug(BLUEDAEMON) << "ReceiveFileJob-Accept";

    KIO::getJobTracker()->registerJob(this);

    FileReceiverSettings::self()->load();
    m_targetPath = FileReceiverSettings::self()->saveUrl().adjusted(QUrl::StripTrailingSlash);
    m_targetPath.setPath(m_targetPath.path() + QLatin1Char('/') + m_transfer->name());

    setTotalAmount(Files, 1);

    Q_EMIT description(this,
                       i18n("Receiving file over Bluetooth"),
                       QPair<QString, QString>(i18nc("File transfer origin", "From"), m_deviceName),
                       QPair<QString, QString>(i18nc("File transfer destination", "To"), m_targetPath.toDisplayString()));

    m_tempPath = createTempPath(m_transfer->name());
    qCDebug(BLUEDAEMON) << "TempPath" << m_tempPath;

    m_accepted = true;
    m_request.accept(m_tempPath);
}

void ReceiveFileJob::slotCancel()
{
    if (!m_accepted && m_transfer->status() == BluezQt::ObexTransfer::Queued) {
        qCDebug(BLUEDAEMON) << "Cancel Push";
        m_request.reject();
        setError(KJob::UserDefinedError);
        emitResult();
    }
}

void ReceiveFileJob::moveFinished(KJob *job)
{
    if (job->error()) {
        qCDebug(BLUEDAEMON) << job->error();
        qCDebug(BLUEDAEMON) << job->errorText();
        setError(job->error());
        setErrorText(i18n("Saving file failed"));

        QFile::remove(m_tempPath);
    }

    setProcessedAmount(Files, 1);

    // Delay emitResult to make sure notification is displayed even
    // for very small files that are received instantly
    QTimer::singleShot(500, this, [this]() {
        emitResult();
    });
}

void ReceiveFileJob::statusChanged(BluezQt::ObexTransfer::Status status)
{
    switch (status) {
    case BluezQt::ObexTransfer::Active:
        qCDebug(BLUEDAEMON) << "ReceiveFileJob-Transfer Active";
        setTotalAmount(Bytes, m_transfer->size());
        setProcessedAmount(Bytes, 0);
        m_time = QTime::currentTime();
        break;

    case BluezQt::ObexTransfer::Complete: {
        qCDebug(BLUEDAEMON) << "ReceiveFileJob-Transfer Complete";
        KIO::CopyJob *job = KIO::move(QUrl::fromLocalFile(m_tempPath), m_targetPath, KIO::HideProgressInfo);
        job->setUiDelegate(nullptr);
        connect(job, &KIO::CopyJob::finished, this, &ReceiveFileJob::moveFinished);
        break;
    }

    case BluezQt::ObexTransfer::Error:
        qCDebug(BLUEDAEMON) << "ReceiveFileJob-Transfer Error";
        setError(KJob::UserDefinedError);
        setErrorText(i18n("Bluetooth transfer failed"));

        // Delay emitResult to make sure notification is displayed even
        // when transfer errors right after accepting it
        QTimer::singleShot(500, this, [this]() {
            emitResult();
        });
        break;

    default:
        qCDebug(BLUEDAEMON) << "Not implemented status: " << status;
        break;
    }
}

void ReceiveFileJob::transferredChanged(quint64 transferred)
{
    // qCDebug(BLUEDAEMON) << "ReceiveFileJob-Transferred" << transferred;

    // If at least 1 second has passed since last update
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
        xdgCacheHome = QDir::homePath() + QStringLiteral("/.cache");
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
