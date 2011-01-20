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
#include <QTimer>
#include <KDebug>

ObexSession::ObexSession(const QString& service, const QString& path, const QDBusConnection& connection, QObject* parent)
: OrgOpenobexSessionInterface(service, path, connection, parent)
{
    m_status = ObexSession::Connecting;

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(sessionTimeoutSlot()));
    connect(this, SIGNAL(TransferStarted(QString,QString,qulonglong)), this, SLOT(resetTimer()));
    connect(this, SIGNAL(TransferProgress(qulonglong)), this, SLOT(resetTimer()));
    connect(this, SIGNAL(TransferCompleted()), this, SLOT(resetTimer()));
    m_timer.setInterval(120000);
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
    kDebug() << "Resetting the timer";
    m_timer.stop();
    m_timer.start();
}

void ObexSession::sessionTimeoutSlot()
{
    kDebug();
    m_status = ObexSession::Timeout;
    m_timer.stop();
    //We can't relay on the Disconnect or Close obex-data-server signals because if two sessions
    //are removed in a short period of time, only 1 session will emit signals.
    //Because of that, we're forced to implement our own.

    disconnect(SIGNAL(Closed()));
    disconnect(SIGNAL(Disconnected()));
    disconnect(SIGNAL(Cancelled()));
    disconnect(SIGNAL(TransferCompleted()));
    disconnect(SIGNAL(TransferProgress(qulonglong)));
    disconnect(SIGNAL(ErrorOccurred(QString,QString)));
    Disconnect();
    Close();

    emit sessionTimeout();
}
