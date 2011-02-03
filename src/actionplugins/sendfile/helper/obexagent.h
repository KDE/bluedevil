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

#ifndef OBEXAGENT_H
#define OBEXAGENT_H

#include <obex_transfer.h>

#include <QDBusAbstractAdaptor>
#include <QString>
#include <QDBusObjectPath>

class ObexAgent : public QDBusAbstractAdaptor
{

Q_OBJECT
Q_CLASSINFO("D-Bus Interface", "org.openobex.Agent")

public:
    ObexAgent(QObject* parent);

    void setKilled();

public Q_SLOTS:

    /**
     *   This method gets called when the service daemon
     *   unregisters the agent. An agent can use it to do
     *   cleanup tasks. There is no need to unregister the
     *   agent, because when this method gets called it has
     *   already been unregistered.
     */
    void Release() const;

    /**
     * Accept or reject a new transfer (client and server)
     *   and provide the filename for it.
     *
     *   In case of incoming transfers it is the filename
     *   where to store the file and for outgoing transfers
     *   it is the filename to show the remote device. If left
     *   empty it will be calculated automatically.
     *
     *   Possible errors: org.openobex.Error.Rejected
     *                    org.openobex.Error.Canceled
     */
    QString Request(QDBusObjectPath transferPath);

    /**
     * Progress within the transfer has been made. The
     *   number of transferred bytes is given as second
     *   argument for convenience.
     */
    void Progress(QDBusObjectPath transfer, quint64 transferred);

    /**
    * Informs that the transfer has completed sucessfully.
    */
    void Complete(QDBusObjectPath transfer);

    /**
    * Informs that the transfer has been terminated because
    * of some error.
    */
    void Error(QDBusObjectPath transfer, const QString &message);

private:
    bool            m_killed;

Q_SIGNALS:
    void request(OrgOpenobexTransferInterface* transfer);
    void progress(QDBusObjectPath transfer, quint64 transferred);
    void completed(QDBusObjectPath transfer);
    void error(QDBusObjectPath transfer, QString message);
};

#endif // OBEXAGENT_H
