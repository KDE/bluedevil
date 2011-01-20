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

#include "obexftpsession.h"
#include "obexftpmanager.h"

class QTimer;

class ObexSession : public OrgOpenobexSessionInterface
{
Q_OBJECT

public:
    ObexSession(const QString& service, const QString& path, const QDBusConnection& connection, QObject* parent = 0);

    enum Status {
        Connected = 0,
        Connecting = 1,
        Timeout     = 2
    };

    Status status() const;
    void setStatus(const Status&);

public Q_SLOTS:
    void resetTimer();

private Q_SLOTS:

    /**
        * The session has not been used for a while, so it has to be disconnected and deleted
        */
    void sessionTimeoutSlot();

private:
    Status m_status;
    QTimer m_timer;

Q_SIGNALS:
    void sessionTimeout();
};

#endif // OBEXSESSION_H
