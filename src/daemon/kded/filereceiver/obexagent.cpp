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
#include "../BlueDevilDaemon.h"

#include <QIcon>
#include <QDBusConnection>

#include <kdebug.h>

ObexAgent::ObexAgent(const KComponentData &componentData, QObject* parent)
    : QDBusAbstractAdaptor(parent)
    , m_componentData(componentData)
{
    kDebug(dblue());
    if (!QDBusConnection::sessionBus().registerObject("/BlueDevil_receiveAgent", parent)) {
        kDebug() << "The dbus object can't be registered";
        return;
    }
}

ObexAgent::~ObexAgent()
{

}

QString ObexAgent::AuthorizePush(const QDBusObjectPath& path, const QDBusMessage &msg)
{
    kDebug(dblue());

    msg.setDelayedReply(true);

    ReceiveFileJob *job = new ReceiveFileJob(msg, path.path(), m_componentData, this);
    job->start();

    return QString();
}

void ObexAgent::Cancel()
{
    kDebug(dblue());
}


void ObexAgent::Release()
{
    kDebug(dblue());
}