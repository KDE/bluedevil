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

#ifndef OBEX_AGENT_H
#define OBEX_AGENT_H

#include <BluezQt/ObexAgent>

class KJob;

class ObexAgent : public BluezQt::ObexAgent
{
    Q_OBJECT

public:
    explicit ObexAgent(BluezQt::ManagerPtr manager, QObject *parent = Q_NULLPTR);

    BluezQt::ManagerPtr manager() const;

    bool shouldAutoAcceptTransfer(const QString &address) const;

    QDBusObjectPath objectPath() const Q_DECL_OVERRIDE;
    void authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void receiveFileJobFinished(KJob *job);

private:
    BluezQt::ManagerPtr m_manager;
    QHash<QString, QDateTime> m_transferTimes;
};

#endif // OBEX_AGENT_H
