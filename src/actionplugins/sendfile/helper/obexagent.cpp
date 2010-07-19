/***************************************************************************
 *   This file is part of the KDE project                                  *
 *                                                                         *
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@ufocoders.com>    *
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

#include "obexagent.h"

#include <QtCore/QDebug>

#include <QDBusConnection>

ObexAgent::ObexAgent(QObject* parent): QDBusAbstractAdaptor(parent)
{
    if (!QDBusConnection::sessionBus().registerObject("/BlueDevil_sendAgent", parent)) {
        qDebug() << "The dbus object can't be registered";
        return;
    }
}

void ObexAgent::Release() const
{
    qDebug() << "Agent released";
}

QString ObexAgent::Request(QDBusObjectPath transfer)
{
    transfer.path();
    Q_UNUSED(transfer);
    qDebug() << "Agent Request";

    return QString();
}

void ObexAgent::Progress(QDBusObjectPath transfer, quint64 transferred)
{
    Q_UNUSED(transfer);
    Q_UNUSED(transferred);
    qDebug() << "Agent Progress";
}


void ObexAgent::Complete(QDBusObjectPath transfer)
{
    Q_UNUSED(transfer);
    qDebug() << "Agent Compelte";
}

void ObexAgent::Error(QDBusObjectPath transfer, const QString& message)
{
    Q_UNUSED(transfer);
    Q_UNUSED(message);
    qDebug() << "Agent Error";
}
