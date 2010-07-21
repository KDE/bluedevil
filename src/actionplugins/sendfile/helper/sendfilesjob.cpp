/***************************************************************************
 *   This file is part of the KDE project                                  *
 *                                                                         *
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@ufocoders.com>    *
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

#include "sendfilesjob.h"
#include "obexagent.h"
#include "sendjob.h"

#include "obex_client.h"

#include <KFileItemList>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;
SendFilesJob::SendFilesJob(KFileItemList list, Device *device, ObexAgent *agent, QObject* parent): KJob(parent)
{
    m_agent = agent;
    m_device = device;

    m_totalSize = 0;
    m_progress = 0;

    Q_FOREACH(const KFileItem &item, list) {
        if (item.isLocalFile()) {
            m_filesToSend << item.url().path();
            qDebug() << "Adding size : " << item.size();
            m_totalSize += item.size();
        }
    }
    connect(m_agent, SIGNAL(request()), this, SLOT(nextJob()));
    connect(m_agent, SIGNAL(completed()), this, SLOT(jobDone()));
    connect(m_agent, SIGNAL(progress(quint64)), this, SLOT(progress(quint64)));
    connect(m_agent, SIGNAL(error(QString)), this, SLOT(error(QString)));

    setCapabilities(Killable);
}


void SendFilesJob::start()
{
    QVariantMap map;
    map.insert("Destination", QVariant(m_device->address()));

    OrgOpenobexClientInterface *client = new OrgOpenobexClientInterface("org.openobex.client", "/", QDBusConnection::sessionBus(), this);
    client->SendFiles(map, m_filesToSend, QDBusObjectPath("/BlueDevil_sendAgent"));

    setTotalAmount(Bytes, m_totalSize);
    setProcessedAmount(Bytes, 0);
    emit description(this, "Sending file over bluetooth", QPair<QString, QString>("From", m_filesToSend.first()), QPair<QString, QString>("To", m_device->name()));
}

void SendFilesJob::nextJob()
{
    m_currentFile = m_filesToSend.takeFirst();
    m_currentFileProgress = 0;
    emit description(this, "Receiving file over bluetooth", QPair<QString, QString>("From", m_currentFile), QPair<QString, QString>("To", m_device->name()));
}

void SendFilesJob::jobDone()
{
    m_currentFileProgress = 0;
    if (m_currentFile.isEmpty()) {
        emitResult();
    }
}

void SendFilesJob::progress(quint64 transfer)
{
    quint64 toAdd = transfer - m_currentFileProgress;
    m_currentFileProgress = transfer;
    m_progress += toAdd;
    setProcessedAmount(Bytes, m_progress);
}

void SendFilesJob::error(const QString& error)
{
    setErrorText(error);
    emitResult();
}
