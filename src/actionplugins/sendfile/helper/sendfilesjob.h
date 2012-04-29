/*****************************************************************************
 * This file is part of the KDE project                                      *
 *                                                                           *
 * Copyright (C) 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>     *
 * Copyright (C) 2010-2011 UFO Coders <info@ufocoders.com>                   *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#ifndef SENDFILESJOB_H
#define SENDFILESJOB_H

#include <obex_transfer.h>

#include <QList>
#include <QStringList>
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
    SendFilesJob(const QStringList &files, BlueDevil::Device* device, ObexAgent* agent, QObject* parent = 0);

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
