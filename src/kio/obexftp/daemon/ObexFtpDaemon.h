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

#ifndef OBEXFTPDAEMON_H
#define OBEXFTPDAEMON_H

#include "../obexdtypes.h"

#include <QDBusObjectPath>
#include <kdedmodule.h>

class KJob;
class QDBusMessage;
class QDBusPendingCallWatcher;

class KDE_EXPORT ObexFtpDaemon : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ObexFtp")

public:
    ObexFtpDaemon(QObject *parent, const QList<QVariant>&);
    virtual ~ObexFtpDaemon();

public Q_SLOTS:
    Q_SCRIPTABLE bool isOnline();
    Q_SCRIPTABLE QString session(QString address, const QDBusMessage &msg);
    Q_SCRIPTABLE bool cancelTransfer(const QString &transfer);

private Q_SLOTS:
    void sessionCreated(KJob* job);
    void serviceRegistered();
    void serviceUnregistered();
    void interfaceRemoved(const QDBusObjectPath &path, const QStringList &interfaces);

private:
    void onlineMode();
    void offlineMode();

    struct Private;
    Private *d;
};

extern int dobex();
#endif /*OBEXFTPDAEMON_H*/
