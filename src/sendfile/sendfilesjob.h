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

#include <QTime>
#include <QList>
#include <QStringList>

#include <KJob>

#include <QBluez/ObexTransfer>

namespace QBluez {
    class Device;
    class ObexTransfer;
    class ObexObjectPush;
}

class SendFilesJob : public KJob
{
    Q_OBJECT

public:
    SendFilesJob(const QStringList &files, QBluez::Device *device, QObject *parent = 0);

    void start() Q_DECL_OVERRIDE;
    bool doKill() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void doStart();
    void nextJob();
    void jobDone();
    void transferredChanged(quint64 transferred);
    void statusChanged(QBluez::ObexTransfer::Status status);

private:
    void progress(quint64 transferBytes);

    QTime m_time;
    QStringList m_filesToSend;
    QList <quint64> m_filesToSendSize;
    QString m_currentFile;
    quint64 m_progress;
    quint64 m_totalSize;
    qulonglong m_speedBytes;
    quint64 m_currentFileSize;
    quint64 m_currentFileProgress;

    QBluez::Device *m_device;
    QBluez::ObexTransfer *m_transfer;
    QBluez::ObexObjectPush *m_objectPush;
};

#endif // SENDFILESJOB_H
