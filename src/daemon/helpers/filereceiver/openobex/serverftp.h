/***************************************************************************
 *   Copyright (C) 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org> *
 *   Copyright (C) 2010-2011 UFO Coders <info@ufocoders.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef OPENOBEX_SERVERFTP_H
#define OPENOBEX_SERVERFTP_H

#include <QtCore/QObject>
#include <QtDBus>

class OrgOpenobexServerInterface;
namespace OpenObex
{

class ServerFtp : public QObject
{
    Q_OBJECT
public:
    ServerFtp(const QString &addr);
    virtual ~ServerFtp();

protected Q_SLOTS:
    void slotErrorOccured(const QString &errorName, const QString &errorMessage);
    void slotSessionCreated(const QDBusObjectPath &path);
    void slotSessionRemoved(const QDBusObjectPath &path);

    void serverCreated(const QDBusObjectPath &path);
    void serverCreatedError(const QDBusError &error);

private:
    bool serviceStarted();

private:
    OrgOpenobexServerInterface *m_dbusServer;
};

}

#endif // OPENOBEX_SERVERFTP_H
