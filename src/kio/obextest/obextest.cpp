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

#include "obextest.h"
#include "types.h"

#include "obexftpmanager.h"
#include "obexftpsession.h"

#include <KUrl>
#include <QDebug>
#include <QVariantMap>

#include <unistd.h>

ObexTest::ObexTest(QObject* parent): QObject(parent)
{
    m_manager = new org::openobex::Manager("org.openobex", "/org/openobex", QDBusConnection::sessionBus(), 0);
    QDBusPendingReply <QDBusObjectPath > rep = m_manager->CreateBluetoothSession("A8:7E:33:5D:6F:4E", "00:24:2C:B0:30:84", "ftp");
    rep.waitForFinished();

    qDebug() << "SessionError: " << rep.error().message();
    qDebug() << "SessionPath: " << rep.value().path();

    const QString sessioPath = rep.value().path();
    m_session = new org::openobex::Session("org.openobex", sessioPath, QDBusConnection::sessionBus(), 0);

    QString error("Not connected");
    while(error == "Not connected") {
        sleep(1);
        qDebug() << "LOOP";
        QDBusPendingReply <QString > a =  m_session->RetrieveFolderListing();
        a.waitForFinished();
        error = a.error().message();
        qDebug() << "Error: " << error;
    }

    QDBusPendingReply <QString > a =  m_session->RetrieveFolderListing();
    qDebug() << "Outside the wheeel";
    qDebug() << a.value();

    QDBusPendingReply <void > f = m_session->ChangeCurrentFolder(KUrl::encode_string("Imágenes"));
//    QDBusPendingReply <void > f = m_session->ChangeCurrentFolder("Imágenes");
   f.waitForFinished();
   qDebug() << "Change error: " << f.error().message();

    qDebug() << m_session->RetrieveFolderListing().value();
}
