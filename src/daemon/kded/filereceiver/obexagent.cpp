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

ObexAgent::ObexAgent(QObject *parent)
    : QBluez::ObexAgent(parent)
{
    qCDebug(BLUEDAEMON) << "ObexAgent created";
}

ObexAgent::~ObexAgent()
{
}

QDBusObjectPath ObexAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/BlueDevil_ReceiveAgent"));
}

void ObexAgent::authorizePush(QBluez::ObexTransfer *transfer, const QBluez::Request<QString> &request)
{
    qCDebug(BLUEDAEMON) << "Agent-AuthorizePush";

    ReceiveFileJob *job = new ReceiveFileJob(request, transfer, this);
    job->start();
}

void ObexAgent::release()
{
    qCDebug(BLUEDAEMON) << "Agent-Release";
}

void ObexAgent::cancel()
{
    qCDebug(BLUEDAEMON) << "Agent-Cancel";
}
