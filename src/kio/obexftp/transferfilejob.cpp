/*
 *  SPDX-FileCopyrightText: 2013 Alejandro Fiestas Fiestas <afiestas@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "transferfilejob.h"
#include "debug_p.h"
#include "kioobexftp.h"

#include <KLocalizedString>

#include <BluezQt/PendingCall>

TransferFileJob::TransferFileJob(BluezQt::ObexTransferPtr transfer, KioFtp *parent)
    : KJob(parent)
    , m_speedBytes(0)
    , m_parent(parent)
    , m_transfer(transfer)
{
}

void TransferFileJob::start()
{
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

void TransferFileJob::doStart()
{
    connect(m_transfer.data(), &BluezQt::ObexTransfer::statusChanged, this, &TransferFileJob::statusChanged);
    connect(m_transfer.data(), &BluezQt::ObexTransfer::transferredChanged, this, &TransferFileJob::transferredChanged);
}

void TransferFileJob::statusChanged(BluezQt::ObexTransfer::Status status)
{
    switch (status) {
    case BluezQt::ObexTransfer::Active:
        qCDebug(OBEXFTP) << "Transfer Active";
        m_time = QTime::currentTime();
        break;

    case BluezQt::ObexTransfer::Complete:
        qCDebug(OBEXFTP) << "Transfer Complete";
        emitResult();
        break;

    case BluezQt::ObexTransfer::Error:
        qCDebug(OBEXFTP) << "Transfer Error";
        setError(KJob::UserDefinedError);
        setErrorText(i18n("Bluetooth transfer failed"));
        emitResult();
        break;

    default:
        qCWarning(OBEXFTP) << "Not implemented status: " << status;
        break;
    }
}

void TransferFileJob::transferredChanged(quint64 transferred)
{
    // qCDebug(OBEXFTP) << "Transferred: " << transferred;

    if (m_parent->wasKilled()) {
        qCDebug(OBEXFTP) << "Kio was killed, aborting task";
        m_parent->cancelTransfer(m_transfer->objectPath().path());
        emitResult();
        return;
    }

    // If at least 1 second has passed since last update
    int secondsSinceLastTime = m_time.secsTo(QTime::currentTime());
    if (secondsSinceLastTime > 0) {
        float speed = (transferred - m_speedBytes) / secondsSinceLastTime;

        m_parent->speed(speed);
        m_time = QTime::currentTime();
        m_speedBytes = transferred;
    }

    m_parent->processedSize(transferred);
}
