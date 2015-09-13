/*************************************************************************************
 *  Copyright (C) 2013 by Alejandro Fiestas Fiestas <afiestas@kde.org>               *
 *  Copyright (C) 2014-2015 David Rosca <nowrep@gmail.com>                           *
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

#ifndef RECEIVEFILEJOB_H
#define RECEIVEFILEJOB_H

#include <QUrl>
#include <QTime>

#include <KJob>

#include <BluezQt/Request>
#include <BluezQt/ObexTransfer>

class ObexAgent;

class ReceiveFileJob : public KJob
{
    Q_OBJECT

public:
    explicit ReceiveFileJob(const BluezQt::Request<QString> &req, BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, ObexAgent *parent);

    QString deviceAddress() const;

    void start() Q_DECL_OVERRIDE;
    bool doKill() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void init();
    void showNotification();
    void slotCancel();
    void slotAccept();
    void moveFinished(KJob *job);

    void statusChanged(BluezQt::ObexTransfer::Status status);
    void transferredChanged(quint64 transferred);

private:
    QString createTempPath(const QString &fileName) const;

    QTime m_time;
    qulonglong m_speedBytes;
    QString m_tempPath;
    QString m_deviceName;
    QString m_deviceAddress;
    QUrl m_targetPath;

    ObexAgent *m_agent;
    BluezQt::ObexTransferPtr m_transfer;
    BluezQt::ObexSessionPtr m_session;
    BluezQt::Request<QString> m_request;
};

#endif // RECEIVEFILEJOB_H

