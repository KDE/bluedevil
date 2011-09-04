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

#ifndef OBEXDAGENT_H
#define OBEXDAGENT_H

#include <QString>
#include <QDBusObjectPath>
#include <QDBusAbstractAdaptor>
#include <QDBusMessage>
#include <QDBusContext>
#include <QVariantMap>

class KNotification;

class ObexdAgent : public QDBusAbstractAdaptor, public QDBusContext
{

Q_OBJECT
Q_CLASSINFO("D-Bus Interface", "org.openobex.Agent")

public:
    ObexdAgent(QObject* parent);

    QVariantMap info() const;
public Q_SLOTS:

    QString Authorize(const QDBusObjectPath &transfer, const QString &bt_address,
                    const QString &name, const QString &type, int length, int time, const QDBusMessage &msg);
    void Cancel();

private Q_SLOTS:
    void fileAccepted();
    void fileSaveAs();
    void fileCanceled();

private:
    KNotification *m_notification;
    QDBusMessage m_pendingMessage;

    QVariantMap     m_info;
};

#endif // OBEXDAGENT_H
