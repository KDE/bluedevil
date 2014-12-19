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

#ifndef KIO_GET_FILE_JOB_H
#define KIO_GET_FILE_JOB_H

#include <QTime>
#include <QVariant>

#include <KJob>

class KioFtp;
class OrgBluezObexTransfer1Interface;
class OrgFreedesktopDBusPropertiesInterface;

class TransferFileJob : public KJob
{
    Q_OBJECT
public:
    explicit TransferFileJob(const QString &path, KioFtp* parent = 0);

    virtual void start();
    virtual bool doKill();

    virtual ~TransferFileJob();

private Q_SLOTS:
    void createObjects();
    void propertiesChanged(const QString &interface , const QVariantMap &properties , const QStringList &invalidProps);
    void statusChanged(const QVariant& value);
    void transferChanged(const QVariant& value);

private:
    QTime m_time;
    const QString m_path;
    qlonglong m_speedBytes;
    KioFtp *m_parent;
    OrgBluezObexTransfer1Interface *m_transfer;
    OrgFreedesktopDBusPropertiesInterface *m_properties;

};

#endif //KIO_GET_FILE_JOB_H
