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

#include "kio_obexftp.h"
#include "transferfilejob.h"
#include "debug_p.h"

#include <KLocalizedString>

#include <QBluez/PendingCall>

TransferFileJob::TransferFileJob(QBluez::ObexTransfer *transfer, KioFtp *parent)
    : KJob(parent)
    , m_speedBytes(0)
    , m_parent(parent)
    , m_transfer(transfer)
{
    setCapabilities(Killable);
}

void TransferFileJob::start()
{
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

bool TransferFileJob::doKill()
{
    // FIXME: Call to cancel will fail with NotAuthorized because obexftp deamon is owner of this transfer
    QBluez::PendingCall *call = m_transfer->cancel();
    call->waitForFinished();
    return !call->error();
}

void TransferFileJob::doStart()
{
    connect(m_transfer, &QBluez::ObexTransfer::statusChanged, this, &TransferFileJob::statusChanged);
    connect(m_transfer, &QBluez::ObexTransfer::transferredChanged, this, &TransferFileJob::transferredChanged);
}

void TransferFileJob::statusChanged(QBluez::ObexTransfer::Status status)
{
    qCDebug(OBEXFTP) << status;

    switch (status) {
    case QBluez::ObexTransfer::Active:
        m_time = QTime::currentTime();
        break;

    case QBluez::ObexTransfer::Complete:
        emitResult();
        break;

    case QBluez::ObexTransfer::Error:
        setError(KJob::UserDefinedError);
        emitResult();
        break;

    default:
        qCDebug(OBEXFTP) << "Not implemented status: " << status;
        break;
    }
}

void TransferFileJob::transferredChanged(quint64 transferred)
{
    qCDebug(OBEXFTP) << "Transferred: " << transferred;

    if (m_parent->wasKilled()) {
        qCDebug(OBEXFTP) << "Kio was killed, aborting task";
        m_transfer->cancel()->waitForFinished();
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
