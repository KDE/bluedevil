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
#include "obex_transfer.h"

#include "obex_client.h"

#include <KFileItemList>
#include <KDebug>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;
SendFilesJob::SendFilesJob(KFileItemList list, Device *device, ObexAgent *agent, QObject* parent): KJob(parent)
,m_currentTransferJob(0)
{
    m_agent = agent;
    m_device = device;

    m_totalSize = 0;
    m_progress = 0;

    Q_FOREACH(const KFileItem &item, list) {
        if (item.isLocalFile()) {
            m_filesToSend << item.url().path();
            kDebug() << "Adding size : " << item.size();
            m_filesToSendSize << item.size();
            m_totalSize += item.size();
        }
    }

    connect(m_agent, SIGNAL(request(OrgOpenobexTransferInterface *)), this, SLOT(nextJob(OrgOpenobexTransferInterface*)));
    connect(m_agent, SIGNAL(completed(QDBusObjectPath)), this, SLOT(jobDone(QDBusObjectPath)));
    connect(m_agent, SIGNAL(progress(QDBusObjectPath, quint64)), this, SLOT(progress(QDBusObjectPath, quint64)));
    connect(m_agent, SIGNAL(error(QDBusObjectPath, QString)), this, SLOT(error(QDBusObjectPath, QString)));

    setCapabilities(Killable);
}

bool SendFilesJob::doKill()
{
    if(m_currentTransferJob) {
        m_currentTransferJob->Cancel();
    }
    m_agent->setKilled();
    return true;
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

void SendFilesJob::nextJob(OrgOpenobexTransferInterface *transferObj)
{
    m_currentFile = m_filesToSend.takeFirst();
    m_currentFileProgress = 0;
    m_currentFileSize = m_filesToSendSize.takeFirst();
    m_currentTransferJob = transferObj;
    emit description(this, "Sending file over bluetooth", QPair<QString, QString>("From", m_currentFile), QPair<QString, QString>("To", m_device->name()));
}

void SendFilesJob::jobDone(QDBusObjectPath transfer)
{
    Q_UNUSED(transfer);

    m_currentFileProgress = 0;
    m_currentFileSize = 0;
    if (m_filesToSend.isEmpty()) {
        emitResult();
    }
}

void SendFilesJob::progress(QDBusObjectPath transfer, quint64 transferBytes)
{
    Q_UNUSED(transfer);

    quint64 toAdd = transferBytes - m_currentFileProgress;
    m_currentFileProgress = transferBytes;
    m_progress += toAdd;
    setProcessedAmount(Bytes, m_progress);
}

void SendFilesJob::error(QDBusObjectPath transfer, const QString& error)
{
    Q_UNUSED(error)

    //if this is the last file, do not complete it
    if (!m_filesToSend.isEmpty()) {
        quint64 toAdd = m_currentFileSize - m_currentFileProgress;
        m_progress += toAdd;
        setProcessedAmount(Bytes, m_progress);
    }

    jobDone(transfer);
}
