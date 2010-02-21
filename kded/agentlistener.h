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
