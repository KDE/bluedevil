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

#include <QBluez/ObexAgent>

class ObexAgent : public QBluez::ObexAgent
{
    Q_OBJECT

public:
    explicit ObexAgent(QObject *parent);
    virtual ~ObexAgent();

    QDBusObjectPath objectPath() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    void authorizePush(QBluez::ObexTransfer *transfer, const QBluez::Request<QString> &request) Q_DECL_OVERRIDE;
    void release();
    void cancel();
};

#endif //OBEX_AGENT_H
