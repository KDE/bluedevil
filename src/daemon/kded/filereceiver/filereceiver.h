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

#ifndef FILE_RECEIVER_H
#define FILE_RECEIVER_H

#include <QObject>
#include <KComponentData>

#include "obex_agent_manager.h"

class QDBusPendingCallWatcher;
class FileReceiver : public QObject
{
    Q_OBJECT
    public:
        explicit FileReceiver(const KComponentData &componentData, QObject* parent = 0);
        virtual ~FileReceiver();

    private Q_SLOTS:
        void registerAgent();
        void agentRegistered(QDBusPendingCallWatcher* call);

    private:
        org::bluez::obex::AgentManager1 *m_agentManager;
};

#endif //FILE_RECEIVER_H
