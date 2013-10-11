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

#ifndef OBEX_AGENT_H
#define OBEX_AGENT_H

#include <QDBusMessage>
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

#include <KComponentData>

class QDBusMessage;
class ObexAgent : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.Agent1")

    public:
        explicit ObexAgent(const KComponentData &componentData, QObject* parent);
        virtual ~ObexAgent();

    public Q_SLOTS:
        QString AuthorizePush(const QDBusObjectPath &path, const QDBusMessage &msg);

        void Release();
        void Cancel();
    private:
        KComponentData m_componentData;
};

#endif //OBEX_AGENT_H