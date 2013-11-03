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

#include "../../daemon/obexftpkded/obexftp/filetransfertypes.h"

#include "obextest.h"
#include "types.h"

#include <QDebug>

#include <KUrl>
#include <QDebug>
#include <QVariantMap>

#include <unistd.h>

#include "obexd_client.h"
#include "obexd_file_transfer.h"

#include <bluedevil/bluedevil.h>
ObexTest::ObexTest(QObject* parent): QObject(parent)
{
    qDBusRegisterMetaType<QVariantMapList>();
    OrgBluezObexClient1Interface *
    m_client = new OrgBluezObexClient1Interface("org.bluez.obex", "/org/bluez/obex", QDBusConnection::sessionBus(), this);

     BlueDevil::Device* device = BlueDevil::Manager::self()->usableAdapter()->deviceForAddress("44:C1:5C:BB:01:42");
    qDebug() << device->name();
    qDebug() << device->UUIDs();
    QVariantMap map;
    map["Target"] = "ftp";
    QDBusPendingReply <QDBusObjectPath > reply = m_client->CreateSession("44:C1:5C:BB:01:42", map);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(createSessionS(QDBusPendingCallWatcher*)));
}

void ObexTest::createSessionS(QDBusPendingCallWatcher* watcher)
{
    const QDBusPendingReply<QDBusObjectPath> reply = *watcher;
    watcher->deleteLater();
    if (reply.isError()) {
        qDebug() << "Error:";
        qDebug() << reply.error().name();
        qDebug() << reply.error().message();
        return;
    }

    QString path = reply.value().path();
    qDebug() << "Got a patch !" << path;
    OrgBluezObexFileTransfer1Interface* transfer = new OrgBluezObexFileTransfer1Interface("org.bluez.obex", path, QDBusConnection::sessionBus(), this);
    qDebug() << transfer->ListFolder().value();

}