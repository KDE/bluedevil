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

#ifndef SENDFILESJOB_H
#define SENDFILESJOB_H

#include <obex_transfer.h>

#include <QStringList>
#include <QList>
#include <QDBusObjectPath>

#include <kcompositejob.h>
#include <KFileItemList>

namespace BlueDevil
{
    class Device;
}
class ObexAgent;
using namespace BlueDevil;

class SendFilesJob : public KJob
{
Q_OBJECT
public:
    SendFilesJob(KFileItemList list, BlueDevil::Device* device, ObexAgent* agent, QObject* parent = 0);

    virtual void start();
    virtual bool doKill();

private Q_SLOTS:
    void doStart();
    void sendFileResult(QDBusPendingCallWatcher *call);
    void nextJob(OrgOpenobexTransferInterface *transferObj);
    void jobDone(QDBusObjectPath transfer);
    void progress(QDBusObjectPath transfer, quint64 transferBytes);
    void error(QDBusObjectPath transfer, const QString& error);

private:
    ObexAgent       *m_agent;
    QStringList     m_filesToSend;
    QList <quint64> m_filesToSendSize;
    Device          *m_device;
    QString         m_currentFile;
    QDBusObjectPath m_currentFileDBusPath;
    quint64         m_totalSize;
    quint64         m_progress;
    quint64         m_currentFileProgress;
    quint64         m_currentFileSize;

    OrgOpenobexTransferInterface *m_currentTransferJob;
};

#endif // SENDFILESJOB_H
