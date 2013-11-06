/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "obexsession.h"
#include "ObexFtpDaemon.h"

#include "obexd_client.h"
#include "obexd_file_transfer.h"

#include <QTimer>
#include <KDebug>
#include <KLocalizedString>

ObexSession::ObexSession(const QString& address, OrgBluezObexClient1Interface* client, const QDBusMessage& msg, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_address(address)
    , m_status(ObexSession::Connecting)
{
    connect(m_client, SIGNAL(destroyed(QObject*)), SLOT(deleteLater()));

    QVariantMap args;
    args["Target"] = "ftp";
    QDBusPendingReply <QDBusObjectPath > reply = m_client->CreateSession(address, args);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(sessionCreated(QDBusPendingCallWatcher*)));

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(sessionTimeoutSlot()));
    m_timer.setInterval(120000);

    kDebug(dobex()) << msg.service() << msg.path();
    msg.setDelayedReply(true);
    m_msgs.append(msg);

    kDebug(dobex()) << m_msgs.first().service() << m_msgs.first().path();
}

void ObexSession::sessionCreated(QDBusPendingCallWatcher* watcher)
{
    const QDBusPendingReply<QDBusObjectPath> reply = *watcher;
    watcher->deleteLater();
    if (reply.isError()) {
        kDebug(dobex()) << "Error:";
        kDebug(dobex()) << reply.error().name();
        kDebug(dobex()) << reply.error().message();
        Q_FOREACH(const QDBusMessage &msg, m_msgs) {
            kDebug(dobex()) << msg.service() << msg.path();
            QDBusMessage errorMsg = msg.createErrorReply("org.kde.kded.Error", i18n("Can't stablish connection"));
            QDBusConnection::sessionBus().send(errorMsg);
        }
        m_status = Error;
        deleteLater();
        return;
    }

    m_status = Connected;
    QString path = reply.value().path();
    kDebug(dobex()) << "Got a patch !" << path;
    m_transfer = new OrgBluezObexFileTransfer1Interface("org.bluez.obex", path, QDBusConnection::sessionBus(), this);

    Q_FOREACH(const QDBusMessage &msg, m_msgs) {
        kDebug(dobex()) << "Sending reply" << msg.service() << msg.path();
        QDBusMessage reply = msg.createReply();
        QDBusConnection::sessionBus().asyncCall(reply);
        kDebug(dobex()) << "AFTER";
    }
}

void ObexSession::addMessage(const QDBusMessage& msg)
{
    msg.setDelayedReply(true);
    m_msgs.append(msg);
}

QString ObexSession::address() const
{
    return m_address;
}

ObexSession::Status ObexSession::status() const
{
    return m_status;
}

void ObexSession::setStatus(const ObexSession::Status& status)
{
    m_status = status;
    if (status == Connected) {
        m_timer.start();
    }
}

void ObexSession::resetTimer()
{
    kDebug(dobex()) << "Resetting the timer";
    m_timer.stop();
    m_timer.start();
}

void ObexSession::sessionTimeoutSlot()
{
    kDebug(dobex());
    m_status = ObexSession::Timeout;
    m_timer.stop();

    emit sessionTimeout();
}

OrgBluezObexFileTransfer1Interface* ObexSession::transfer() const
{
    return m_transfer;
}