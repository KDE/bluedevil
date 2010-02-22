/*  This file is part of the KDE project

    Copyright (C) 2010  Alex Fiestas <alex@eyeos.org>
    Copyright (C) 2010 by Eduardo Robles Elvira <edulix@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef AGENTLISTENER_H
#define AGENTLISTENER_H

#include <QThread>
#include "agentlistenerworker.h"

/**
 * @internal
 * @short This class is only a delegate to be able to use agentlistener on a QThread (We can't inherit
 *        from 2 QObjects
 * This class is only a delegate to be able to use agentlistener on a QThread (We can't inherit
 * from 2 QObjects, so we had to create a new Class only to do the threading stuff
 * @ref AgentListenerWorker
 * @since 1.0
 */
class AgentListener : public QThread
{

Q_OBJECT
public:
    AgentListener();
    virtual ~AgentListener();
Q_SIGNALS:
    void agentReleased();

protected:
    virtual void run();

private:
    AgentListenerWorker *m_worker;
};


#endif // AGENTLISTENER_H
