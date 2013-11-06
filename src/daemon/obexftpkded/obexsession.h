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

#ifndef OBEXSESSION_H
#define OBEXSESSION_H

#include "obexd_file_transfer.h"

class QTimer;
class QDBusPendingCallWatcher;
class OrgBluezObexClient1Interface;
class OrgBluezObexFileTransfer1Interface;

class ObexSession : public QObject
{
    Q_OBJECT

public:
    explicit ObexSession(const QString& address, OrgBluezObexClient1Interface* client, const QDBusMessage &msg, QObject* parent = 0);

    enum Status {
        Connected,
        Connecting,
        Timeout,
        Error,
    };

    QString address() const;
    Status status() const;
    void addMessage(const QDBusMessage &msg);
    void setStatus(const Status& state);
    OrgBluezObexFileTransfer1Interface *transfer() const;

public Q_SLOTS:
    void resetTimer();

private Q_SLOTS:
    /**
        * The session has not been used for a while, so it has to be disconnected and deleted
        */
    void sessionTimeoutSlot();
    void sessionCreated(QDBusPendingCallWatcher* watcher);


private:
    Status m_status;
    QTimer m_timer;
    QString m_address;

    QList<QDBusMessage> m_msgs;
    OrgBluezObexClient1Interface *m_client;
    OrgBluezObexFileTransfer1Interface *m_transfer;

Q_SIGNALS:
    void sessionTimeout();
};

#endif // OBEXSESSION_H
