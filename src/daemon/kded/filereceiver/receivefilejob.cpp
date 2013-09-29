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

#include "receivefilejob.h"
#include "../BlueDevilDaemon.h"
#include "obex_transfer.h"
#include "obex_session.h"

#include <QIcon>
#include <QDBusConnection>

#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>
#include <bluedevil/bluedevildevice.h>

#include <KDebug>
#include <KIconLoader>
#include <KNotification>
#include <KLocalizedString>

using namespace BlueDevil;

ReceiveFileJob::ReceiveFileJob(const QDBusMessage& msg, const QString &path, QObject* parent)
    : KJob(parent)
    , m_path(path)
    , m_msg(msg)
{

}

ReceiveFileJob::~ReceiveFileJob()
{

}

void ReceiveFileJob::start()
{
    QMetaObject::invokeMethod(this, "showNotification", Qt::QueuedConnection);
}

void ReceiveFileJob::showNotification()
{
    m_transfer = new org::bluez::obex::Transfer1("org.bluez.obex", m_path, QDBusConnection::sessionBus(), this);
    kDebug(dblue()) << m_transfer->name();
    kDebug(dblue()) << m_transfer->filename();
    kDebug(dblue()) << m_transfer->status();
    kDebug(dblue()) << m_transfer->type();
    kDebug(dblue()) << m_transfer->size();
    kDebug(dblue()) << m_transfer->transferred();

    m_session = new org::bluez::obex::Session1("org.bluez.obex", m_transfer->session().path(), QDBusConnection::sessionBus(), this);
    kDebug(dblue()) << m_session->destination();

     Device* device = Manager::self()->usableAdapter()->deviceForAddress(m_session->destination());
     kDebug(dblue()) << device;

     QString name = m_session->destination();
     if (device) {
         kDebug(dblue()) << device->name();
         name = device->name();
     }

    KNotification *m_notification = new KNotification("bluedevilIncomingFile",
        KNotification::Persistent, this);

    m_notification->setText(i18nc(
        "Show a notification asking to authorize or deny an incoming file transfer to this computer from a Bluetooth device.",
        "%1 is sending you the file %2", name, m_transfer->name()));

    QStringList actions;

    actions.append(i18nc("Button to accept the incoming file transfer and download it in the default download directory", "Accept"));
    actions.append(i18nc("Button to accept the incoming file transfer and show a Save as... dialog that will let the user choose where will the file be downloaded to", "Save as..."));
    actions.append(i18nc("Deny the incoming file transfer", "Cancel"));

    m_notification->setActions(actions);

    connect(m_notification, SIGNAL(action1Activated()), SLOT(slotAccept()));
    connect(m_notification, SIGNAL(action2Activated()), SLOT(slotSaveAs()));
    connect(m_notification, SIGNAL(action3Activated()), SLOT(slotCancel()));
    connect(m_notification, SIGNAL(closed()), SLOT(slotCancel()));

    int size = IconSize(KIconLoader::Desktop);
    m_notification->setPixmap(QIcon::fromTheme("preferences-system-bluetooth").pixmap(size, size));
    m_notification->setComponentData(KComponentData("bluedevil"));
    m_notification->sendEvent();
}

void ReceiveFileJob::slotAccept()
{
    kDebug(dblue());
    QDBusMessage msg = m_msg.createReply(QString("/tmp/meh.jpg"));
    QDBusConnection::sessionBus().send(msg);
}

void ReceiveFileJob::slotSaveAs()
{

}

void ReceiveFileJob::slotCancel()
{
    kDebug(dblue());
    QDBusMessage msg = m_msg.createErrorReply("org.bluez.obex.Error.Rejected", "org.bluez.obex.Error.Rejected");
    QDBusConnection::sessionBus().send(msg);
}