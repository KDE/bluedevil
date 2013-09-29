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

#include "obexagent.h"
#include "../BlueDevilDaemon.h"
#include "obex_transfer.h"
#include "obex_session.h"

#include <QIcon>
#include <QDBusConnection>

#include <kdebug.h>
#include <KIconLoader>
#include <KNotification>
#include <KLocalizedString>

#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>
#include <bluedevil/bluedevildevice.h>

using namespace BlueDevil;

ObexAgent::ObexAgent(QObject* parent) : QDBusAbstractAdaptor(parent)
{
    kDebug(dblue());
    if (!QDBusConnection::sessionBus().registerObject("/BlueDevil_receiveAgent", parent)) {
        kDebug() << "The dbus object can't be registered";
        return;
    }
}

ObexAgent::~ObexAgent()
{

}

QString ObexAgent::AuthorizePush(const QDBusObjectPath& path, const QDBusMessage &msg)
{
    kDebug(dblue());

    m_msg = msg;
    m_msg.setDelayedReply(true);

    org::bluez::obex::Transfer1 *transfer = new org::bluez::obex::Transfer1("org.bluez.obex", path.path(), QDBusConnection::sessionBus(), this);
    kDebug(dblue()) << transfer->name();
    kDebug(dblue()) << transfer->filename();
    kDebug(dblue()) << transfer->status();
    kDebug(dblue()) << transfer->type();
    kDebug(dblue()) << transfer->size();
    kDebug(dblue()) << transfer->transferred();

    org::bluez::obex::Session1 *session = new org::bluez::obex::Session1("org.bluez.obex", transfer->session().path(), QDBusConnection::sessionBus(), this);
    kDebug(dblue()) << session->destination();

     Device* device = Manager::self()->usableAdapter()->deviceForAddress(session->destination());
     kDebug(dblue()) << device;

     QString name = session->destination();
     if (device) {
         kDebug(dblue()) << device->name();
         name = device->name();
     }

    KNotification *m_notification = new KNotification("bluedevilIncomingFile",
        KNotification::Persistent, this);

    m_notification->setText(i18nc(
        "Show a notification asking to authorize or deny an incoming file transfer to this computer from a Bluetooth device.",
        "%1 is sending you the file %2", name, transfer->name()));

    QStringList actions;

    actions.append(i18nc("Button to accept the incoming file transfer and download it in the default download directory", "Accept"));
    actions.append(i18nc("Button to accept the incoming file transfer and show a Save as... dialog that will let the user choose where will the file be downloaded to", "Save as..."));
    actions.append(i18nc("Deny the incoming file transfer", "Cancel"));

    m_notification->setActions(actions);

    connect(m_notification, SIGNAL(action1Activated()), SLOT(slotAccept()));
    connect(m_notification, SIGNAL(action2Activated()), SLOT(slotSaveAs()));
    connect(m_notification, SIGNAL(action3Activated()), SLOT(slotCancel()));
    connect(m_notification, SIGNAL(closed()), this, SLOT(slotCancel()));

    int size = IconSize(KIconLoader::Desktop);
    m_notification->setPixmap(QIcon::fromTheme("preferences-system-bluetooth").pixmap(size, size));
    m_notification->setComponentData(KComponentData("bluedevil"));
    m_notification->sendEvent();

    return QString();
}

void ObexAgent::slotAccept()
{
    kDebug(dblue());
    QDBusMessage msg = m_msg.createReply(QString("/tmp/meh.jpg"));
    QDBusConnection::sessionBus().send(msg);
}

void ObexAgent::slotSaveAs()
{

}

void ObexAgent::slotCancel()
{
    kDebug(dblue());
    QDBusMessage msg = m_msg.createErrorReply("org.bluez.obex.Error.Rejected", "org.bluez.obex.Error.Rejected");
    QDBusConnection::sessionBus().send(msg);
}

void ObexAgent::Cancel()
{
    kDebug(dblue());
}


void ObexAgent::Release()
{
    kDebug(dblue());
}