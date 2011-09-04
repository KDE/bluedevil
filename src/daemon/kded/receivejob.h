/***************************************************************************
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <afiestas@kde.org>      *
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


#ifndef RECEIVEJOB_H
#define RECEIVEJOB_H

#include <KJob>
#include <fcntl.h>
#include <QTime>

class QDBusObjectPath;
class OrgOpenobexTransferInterface;

class ReceiveJob : public KJob
{
Q_OBJECT
public:
    ReceiveJob(const QString &path, const QString &dest, const QString &from, const int length, QObject *parent);

    virtual void start();

    void failed();
    void completed();

public Q_SLOTS:
    void Progress(int total, int transfer);
    void doStart();

protected:
    virtual bool doKill();

private:
    OrgOpenobexTransferInterface *m_transfer;
    QString m_from;
    QString m_dest;
    int     m_length;
    QTime m_time;
};

#endif // RECEIVEJOB_H
