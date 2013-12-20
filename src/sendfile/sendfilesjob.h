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

#include <QList>
#include <QStringList>
#include <QDBusObjectPath>

#include <kcompositejob.h>
#include <KFileItemList>

namespace BlueDevil
{
    class Device;
}

class QDBusPendingCallWatcher;
class OrgBluezObexClient1Interface;
class OrgBluezObexObjectPush1Interface;
class OrgFreedesktopDBusPropertiesInterface;

using namespace BlueDevil;
class SendFilesJob : public KJob
{
Q_OBJECT
public:
    SendFilesJob(const QStringList &files, BlueDevil::Device* device, QObject* parent = 0);

    virtual void start();
    virtual bool doKill();

private Q_SLOTS:
    void doStart();
    void createSessionSlot(QDBusPendingCallWatcher *call);
    void nextJob();
    void jobDone();
    void progress(quint64 transferBytes);
    void error(const QDBusObjectPath& transfer, const QString& error);
    void propertiesChangedSlot(const QString& interface, const QVariantMap &props, const QStringList &invalidProps);
    void sendFileSlot(QDBusPendingCallWatcher* watcher);

private:
    void transferChanged(const QVariant &value);
    void statusChanged(const QVariant &value);

    QTime m_time;
    QStringList     m_filesToSend;
    QList <quint64> m_filesToSendSize;
    QString         m_currentFile;
    QDBusObjectPath m_currentFileDBusPath;
    quint64         m_progress;
    quint64         m_totalSize;
    qulonglong m_speedBytes;
    Device          *m_device;
    quint64         m_currentFileSize;
    quint64         m_currentFileProgress;


    OrgBluezObexClient1Interface *m_client;
    OrgBluezObexObjectPush1Interface *m_push;
    OrgFreedesktopDBusPropertiesInterface *m_props;
};

#endif // SENDFILESJOB_H
