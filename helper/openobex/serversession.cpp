/*
    This file is part of the KDE project

    Copyright (C) 2010 by Eduardo Robles Elvira <edulix@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "serversession.h"
#include "filetransferjob.h"

#include <QtDBus>
#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>
#include <KFileDialog>
#include <KNotification>
#include <kstatusbarjobtracker.h>
#include <KIO/JobUiDelegate>

using namespace OpenObex;

ServerSession::ServerSession(const QString& path, const QString& bluetoothAddress): QObject(0)
{
    m_path = path;

    Solid::Control::BluetoothManager &bluetoothManager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface* bluetoothInterface = new Solid::Control::BluetoothInterface(bluetoothManager.defaultInterface());
    m_bluetoothDevice = bluetoothInterface->findBluetoothRemoteDeviceAddr(bluetoothAddress);
    QDBusConnection* dbus = new QDBusConnection("dbus");
    QDBusConnection dbusConnection = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");
    m_dbusServerSession = new org::openobex::ServerSession("org.openobex", path, dbusConnection,
        this);

    connect(m_dbusServerSession, SIGNAL(Cancelled()),
        this, SLOT(slotCancelled()));
    connect(m_dbusServerSession, SIGNAL(Disconnected()),
        this, SLOT(slotDisconnected()));
    connect(m_dbusServerSession, SIGNAL(TransferStarted(const QString&, const QString&, qulonglong)),
        this, SLOT(slotTransferStarted(const QString&, const QString&, qulonglong)));
}

ServerSession::~ServerSession()
{
    kDebug();
    disconnect();
    m_dbusServerSession->deleteLater();
}

QString ServerSession::path()
{
    return m_path;
}

void ServerSession::slotCancelled()
{
    kDebug() << "slotCancelled()";
}

void ServerSession::slotDisconnected()
{
    kDebug() << "slotDisconnected()";
}

org::openobex::ServerSession* ServerSession::dbusServerSession()
{
    return m_dbusServerSession;
}

void ServerSession::slotTransferStarted(const QString& filename, const QString& localPath,
    qulonglong totalBytes)
{
    m_filename = filename;
    m_localDir = KUrl(localPath).directory();
    m_totalBytes = totalBytes;

    kDebug() << "slotTransferStarted()" << "filename" << filename << "m_localDir" << m_localDir <<
        "totalBytes" << totalBytes;
    KNotification *notification = new KNotification("bluedevilIncomingFile",
        KNotification::Persistent, this);

    notification->setText(i18nc(
        "Show a notification asking for authorize or deny an incoming file transfer to this computer from a Bluetooth device.",
        "%1 is sending you the file %2", m_bluetoothDevice.name(), filename));
    QStringList actions;

    actions.append(i18nc("Deny the incoming file transfer", "Cancel"));
    actions.append(i18nc("Button to accept the incoming file transfer and download it in the default download directory", "Accept"));
    actions.append(i18nc("Button to accept the incoming file transfer and show a SaveAs.. dialog that will let the user choose where will the file be downloaded to", "Save as.."));

    notification->setActions(actions);

    connect(notification, SIGNAL(action1Activated()), this, SLOT(slotCancel()));
    connect(notification, SIGNAL(action2Activated()), this, SLOT(slotAccept()));
    connect(notification, SIGNAL(action3Activated()), this, SLOT(slotSaveAs()));

    notification->setPixmap(KIcon("preferences-system-bluetooth").pixmap(42, 42));
    notification->sendEvent();
}

void ServerSession::slotCancel()
{
    m_dbusServerSession->Cancel();
}

void ServerSession::slotAccept()
{
    KUrl saveUrl;
    saveUrl.setDirectory(m_localDir);
    saveUrl.setFileName(m_filename);
    FileTransferJob *fileTransferJob = new FileTransferJob(this, saveUrl, m_totalBytes);
    KIO::getJobTracker()->registerJob(fileTransferJob);
    fileTransferJob->start();
}

void ServerSession::slotSaveAs()
{
    KUrl saveUrl = m_localDir;
    saveUrl = KFileDialog::getSaveUrl(saveUrl);

    if (!saveUrl.isEmpty()) {
      // Update save Url
      KConfigGroup group = KGlobal::config()->group("ObexServer");
      group.writeEntry("savePath", saveUrl.directory());
      m_localDir = saveUrl.directory();
      kDebug() << "saveUrl" << saveUrl;
      kDebug() << "m_localDir" << m_localDir;

      FileTransferJob *fileTransferJob = new FileTransferJob(this, saveUrl, m_totalBytes);
      KIO::getJobTracker()->registerJob(fileTransferJob);
      fileTransferJob->start();
    } else {
      slotCancel();
    }
}

