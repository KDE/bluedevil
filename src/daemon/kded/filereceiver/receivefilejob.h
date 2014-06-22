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


#ifndef RECEIVE_FILE_JOB_H
#define RECEIVE_FILE_JOB_H

#include <QTime>
#include <QDBusMessage>

#include <KJob>
#include <KComponentData>

class OrgBluezObexSession1Interface;
class OrgBluezObexTransfer1Interface;
class OrgFreedesktopDBusPropertiesInterface;
class ReceiveFileJob : public KJob
{
    Q_OBJECT
    public:
        explicit ReceiveFileJob(const QDBusMessage &msg, const QString &path, const KComponentData &componentData, QObject* parent = 0);
        virtual ~ReceiveFileJob();

        virtual void start();
    protected:
        virtual bool doKill();

    private Q_SLOTS:
        void init();
        void showNotification();
        void slotCancel();
        void slotAccept();
        void slotSaveAs();
        void transferPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProperties);
        void moveFinished(KJob* job);

    private:
        void transferChanged(const QVariant &value);
        void statusChanged(const QVariant &value);
        QString createTempPath(const QString &fileName) const;

        QTime m_time;
        qulonglong m_speedBytes;
        QString m_path;
        QString m_tempPath;
        QString m_originalFileName;
        QString m_deviceName;
        QDBusMessage m_msg;
        KComponentData m_componentData;
        OrgBluezObexSession1Interface *m_session;
        OrgBluezObexTransfer1Interface *m_transfer;
        OrgFreedesktopDBusPropertiesInterface *m_transferProps;
};

#endif //RECEIVE_FILE_JOB_H

