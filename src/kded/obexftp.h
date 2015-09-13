/*************************************************************************************
 *  Copyright (C) 2013 by Alejandro Fiestas Fiestas <afiestas@kde.org>               *
 *  Copyright (C) 2014-2015 David Rosca <nowrep@gmail.com>                           *
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

#ifndef OBEXFTP_H
#define OBEXFTP_H

#include <QHash>
#include <QDBusMessage>
#include <QDBusAbstractAdaptor>

#include <BluezQt/Manager>

class QDBusPendingCallWatcher;

class BlueDevilDaemon;

class Q_DECL_EXPORT ObexFtp : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.BlueDevil.ObexFtp")

public:
    explicit ObexFtp(BlueDevilDaemon *daemon);

    Q_SCRIPTABLE bool isOnline();
    Q_SCRIPTABLE QString preferredTarget(const QString &address);
    Q_SCRIPTABLE QString session(const QString &address, const QString &target, const QDBusMessage &msg);
    Q_SCRIPTABLE bool cancelTransfer(const QString &transfer, const QDBusMessage &msg);

private Q_SLOTS:
    void createSessionFinished(BluezQt::PendingCall *call);
    void cancelTransferFinished(QDBusPendingCallWatcher *watcher);
    void sessionRemoved(BluezQt::ObexSessionPtr session);

private:
    BlueDevilDaemon *m_daemon;
    QHash<QString, QString> m_sessionMap;
    QHash<QString, QList<QDBusMessage> > m_pendingSessions;
};

#endif // OBEXFTP_H
