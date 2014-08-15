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

#include <KJob>

#include <QBluez/Request>
#include <QBluez/ObexTransfer>

namespace QBluez {
    class InitManagerJob;
}

class ReceiveFileJob : public KJob
{
    Q_OBJECT
public:
    explicit ReceiveFileJob(const QBluez::Request<QString> &req, QBluez::ObexTransfer *transfer, QObject *parent = 0);
    ~ReceiveFileJob();

    void start() Q_DECL_OVERRIDE;

protected:
    bool doKill() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void init();
    void initJobResult(QBluez::InitManagerJob *job);
    void showNotification();
    void slotCancel();
    void slotAccept();
    void slotSaveAs();
    void moveFinished(KJob *job);

    void statusChanged(QBluez::ObexTransfer::Status status);
    void transferredChanged(quint64 transferred);

private:
    QString createTempPath(const QString &fileName) const;

    QTime m_time;
    qulonglong m_speedBytes;
    QString m_tempPath;
    QString m_originalFileName;
    QString m_deviceName;
    QBluez::Request<QString> m_request;
    QBluez::ObexTransfer *m_transfer;
};

#endif //RECEIVE_FILE_JOB_H

