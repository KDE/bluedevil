/***************************************************************************
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@eyeos.org>        *
 *   Copyright (C) 2010 UFO Coders <info@ufocoders.com>                    *
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

#ifndef OPENOBEX_SERVER_H
#define OPENOBEX_SERVER_H

#include <QtCore/QObject>
#include <QtDBus>

namespace OpenObex
{

class Server : public QObject
{
    Q_OBJECT
public:
    Server(const QString& addr);
    virtual ~Server();

protected Q_SLOTS:
    void slotErrorOccured(const QString& errorName, const QString& errorMessage);
    void slotSessionCreated(QDBusObjectPath path);
    void slotSessionRemoved(QDBusObjectPath path);

    void serverCreated(QDBusObjectPath path);
    void serverCreatedError(QDBusError error);
    QMap<QString,QString> getServerSessionInfo(QDBusObjectPath path);

private:
    bool serviceStarted();
    void checkDestinationDir();

private:
    struct Private;
    Private *d;
};

}

#endif // OPENOBEX_SERVER_H
