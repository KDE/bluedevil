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

#include "createsessionjob.h"
#include "obexd_client.h"
#include "ObexFtpDaemon.h"

#include <QString>

#include <KDebug>

// class
CreateSessionJob::CreateSessionJob(const QString& address, const QDBusMessage& msg, QObject* parent)
    : KJob(parent)
    , m_address(address)
    , m_client(0)
{
    m_messages.append(msg);
}

void CreateSessionJob::start()
{
    QMetaObject::invokeMethod(this, "createSession", Qt::QueuedConnection);
}

QString CreateSessionJob::path()
{
    return m_path;
}

const QString CreateSessionJob::address() const
{
    return m_address;
}

void CreateSessionJob::addMessage(const QDBusMessage& msg)
{
    m_messages.append(msg);
}

const QList< QDBusMessage > CreateSessionJob::messages() const
{
    return m_messages;
}

void CreateSessionJob::createSession()
{
    kDebug(dobex());
    QVariantMap args;
    args["Target"] = "ftp";
    m_client = new OrgBluezObexClient1Interface("org.bluez.obex",
                                                "/org/bluez/obex",
                                                QDBusConnection::sessionBus(), this);

    QDBusPendingReply <QDBusObjectPath > reply = m_client->CreateSession(m_address, args);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(sessionCreated(QDBusPendingCallWatcher*)));
}

void CreateSessionJob::sessionCreated(QDBusPendingCallWatcher* watcher)
{
    kDebug(dobex());
    QDBusPendingReply <QDBusObjectPath > reply = *watcher;
    watcher->deleteLater();
    if (reply.isError()) {
        kDebug(dobex()) << "Error:";
        kDebug(dobex()) << reply.error().name();
        kDebug(dobex()) << reply.error().message();
        setError(reply.error().type());
        setErrorText(reply.error().message());
        emitResult();
        return;
    }

    m_path = reply.value().path();
    emitResult();
}
