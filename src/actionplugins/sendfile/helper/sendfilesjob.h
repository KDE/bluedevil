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
class OrgBluezObexClient1Interface;
class OrgBluezObexObjectPush1Interface;
class OrgFreedesktopDBusPropertiesInterface;

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
    void createSessionSlot(QDBusPendingCallWatcher *call);
    void nextJob();
    void jobDone();
    void progress(quint64 transferBytes);
    void error(QDBusObjectPath transfer, const QString& error);
    void propertiesChangedSlot(const QString& interface, const QVariantMap &props, const QStringList &invalidProps);
    void sendFileSlot(QDBusPendingCallWatcher* watcher);

private:
    void transferChanged(const QVariant &value);
    void statusChanged(const QVariant &value);

    qulonglong m_speedBytes;
    QTime m_time;
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


    OrgBluezObexClient1Interface *m_client;
    OrgBluezObexObjectPush1Interface *m_push;
    OrgFreedesktopDBusPropertiesInterface *m_props;
    OrgOpenobexTransferInterface *m_currentTransferJob;
};

#endif // SENDFILESJOB_H
