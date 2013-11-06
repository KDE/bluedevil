/*************************************************************************************
 *  Copyright (C) 2013 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
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

#include "listdirjob.h"
#include "obexsession.h"
#include "obexd_client.h"
#include "ObexFtpDaemon.h"

#include <QDBusConnection>
#include <QDBusPendingCallWatcher>

#include <KDebug>

ListDirJob::ListDirJob(ObexSession* session, const QDBusMessage& msg, QObject* parent)
    : KJob(parent)
    , m_msg(msg)
    , m_session(session)
{

}

void ListDirJob::start()
{
    QMetaObject::invokeMethod(this, "listDirDBus");
}

void ListDirJob::listDirDBus()
{
    kDebug(dobex()) << "ASking for the shit";
    QDBusPendingReply <QVariantMapList> reply = m_session->transfer()->ListFolder();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
           SLOT(ListFolderDBusFinished(QDBusPendingCallWatcher*)));
}

void ListDirJob::ListFolderDBusFinished(QDBusPendingCallWatcher* watcher)
{
    kDebug(dobex()) << "Got the shit yo";
    QDBusPendingReply<QVariantMapList> reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        kDebug(dobex()) << "Is error: " << reply.error().type();
        kDebug(dobex()) << reply.error().message();
        setError(reply.error().type());
        setErrorText(reply.error().message());
        emitResult();
        return;
    }

    kDebug(dobex()) << "Enviando" << reply.argumentAt(0).isValid();
    QDBusMessage msg = m_msg.createReply(QVariant::fromValue<QVariantMapList>(reply.value()));
    kDebug(dobex()) << "MEH: " << QDBusConnection::sessionBus().send(msg);
    emitResult();
}