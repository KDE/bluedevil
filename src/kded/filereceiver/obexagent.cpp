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

#include "obexagent.h"
#include "receivefilejob.h"
#include "debug_p.h"

#include <QDBusObjectPath>

ObexAgent::ObexAgent(BluezQt::ManagerPtr manager, QObject *parent)
    : BluezQt::ObexAgent(parent)
    , m_manager(manager)
{
}

BluezQt::ManagerPtr ObexAgent::manager() const
{
    return m_manager;
}

bool ObexAgent::shouldAutoAcceptTransfer(const QString &address) const
{
    if (!m_transferTimes.contains(address)) {
        return false;
    }

    // Auto-accept transfers from the same device within 2 seconds from last finished transfer
    const int timeout = 2;
    return m_transferTimes.value(address).secsTo(QDateTime::currentDateTime()) < timeout;
}

QDBusObjectPath ObexAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/BlueDevilObexAgent"));
}

void ObexAgent::authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request)
{
    qCDebug(BLUEDAEMON) << "Agent-AuthorizePush";

    ReceiveFileJob *job = new ReceiveFileJob(request, transfer, session, this);
    connect(job, &ReceiveFileJob::finished, this, &ObexAgent::receiveFileJobFinished);
    job->start();
}

void ObexAgent::receiveFileJobFinished(KJob *job)
{
    Q_ASSERT(qobject_cast<ReceiveFileJob*>(job));
    ReceiveFileJob *j = static_cast<ReceiveFileJob*>(job);

    if (j->error()) {
        m_transferTimes.remove(j->deviceAddress());
        return;
    }

    m_transferTimes[j->deviceAddress()] = QDateTime::currentDateTime();
}
