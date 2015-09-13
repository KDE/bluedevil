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

#include "filereceiver.h"
#include "obexagent.h"
#include "debug_p.h"

#include <BluezQt/PendingCall>
#include <BluezQt/InitObexManagerJob>

FileReceiver::FileReceiver(BluezQt::ManagerPtr manager, QObject *parent)
    : QObject(parent)
{
    m_agent = new ObexAgent(manager, this);

    m_manager = new BluezQt::ObexManager(this);
    BluezQt::InitObexManagerJob *initJob = m_manager->init();
    initJob->start();
    connect(initJob, &BluezQt::InitObexManagerJob::result, this, &FileReceiver::initJobResult);
}

FileReceiver::~FileReceiver()
{
    if (m_agent) {
        qCDebug(BLUEDAEMON) << "Unregistering ObexAgent";
        m_manager->unregisterAgent(m_agent);
    }
}

void FileReceiver::initJobResult(BluezQt::InitObexManagerJob *job)
{
    if (job->error()) {
        qCWarning(BLUEDAEMON) << "Error initializing ObexManager!";
        return;
    }

    // Make sure to register agent when obexd starts
    operationalChanged(m_manager->isOperational());
    connect(m_manager, &BluezQt::ObexManager::operationalChanged, this, &FileReceiver::operationalChanged);
}

void FileReceiver::agentRegistered(BluezQt::PendingCall *call)
{
    if (call->error()) {
        qCWarning(BLUEDAEMON) << "Error registering ObexAgent" << call->errorText();
    } else {
        qCDebug(BLUEDAEMON) << "ObexAgent registered";
    }
}

void FileReceiver::operationalChanged(bool operational)
{
    qCDebug(BLUEDAEMON) << "ObexManager operational changed" << operational;

    if (operational) {
        BluezQt::PendingCall *call = m_manager->registerAgent(m_agent);
        connect(call, &BluezQt::PendingCall::finished, this, &FileReceiver::agentRegistered);
    } else {
        // Attempt to start obexd
        BluezQt::ObexManager::startService();
    }
}
