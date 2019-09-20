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

#ifndef OBEXAGENT_H
#define OBEXAGENT_H

#include <BluezQt/ObexAgent>
#include <QHash>
class KJob;

class BlueDevilDaemon;

class ObexAgent : public BluezQt::ObexAgent
{
    Q_OBJECT

public:
    explicit ObexAgent(BlueDevilDaemon *daemon);

    BluezQt::Manager *manager() const;

    bool shouldAutoAcceptTransfer(const QString &address) const;

    QDBusObjectPath objectPath() const override;
    void authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request) override;

private Q_SLOTS:
    void receiveFileJobFinished(KJob *job);

private:
    BluezQt::Manager *m_manager;
    QHash<QString, QDateTime> m_transferTimes;
};

#endif // OBEXAGENT_H
