/***************************************************************************
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@eyeos.org>        *
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

#include "serversession.h"
#include "filetransferjob.h"
#include "filereceiversettings.h"

#include <bluedevil/bluedevil.h>

#include <QtDBus>
#include <QDesktopServices>

#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>
#include <KFileDialog>
#include <kstatusbarjobtracker.h>
#include <KIO/JobUiDelegate>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KIO/NetAccess>
#include <KNotification>

using namespace OpenObex;

ServerSession::ServerSession(const QString& path, const QString& bluetoothAddress): QObject(0)
{
    m_path = path;
    m_notification = 0;
    m_jobFinished = true;
    m_deleteLater = false;
    m_doNotDelete = false;

    m_bluetoothDevice = BlueDevil::Manager::self()->defaultAdapter()->deviceForAddress(bluetoothAddress);
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
    if (m_notification) {
      // Here we close the notification, because otherwise it will still be shown even if it serves
      // no purpose because there's no ServerSession instance listening to its action's signals.
      // This happens for example when user doesn't select any of the notification buttons and bluez
      // timesout after 15 seconds of inactivity.
      m_notification->close();
    }
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

BlueDevil::Device *ServerSession::device()
{
    return m_bluetoothDevice;
}

void ServerSession::slotTransferStarted(const QString& filename, const QString& localPath,
    qulonglong totalBytes)
{
    m_savePath = localPath; // This is where the file is being downloaded
    m_totalBytes = totalBytes;

    //AutoAccept trusted devices
    FileReceiverSettings::self()->readConfig();
    if (FileReceiverSettings::self()->autoAccept() == 1) {
        if (m_bluetoothDevice->isTrusted()) {
            slotAccept();
            return;
        }
    }

    kDebug() << "slotTransferStarted()" << "filename" << filename << "m_savePath" << m_savePath <<
        "totalBytes" << totalBytes;

    Q_ASSERT_X(m_notification == 0, "notification", "notification should be zero, because otherwise "
        "it means that multiple transfer have been started inside one ServerSession, something that "
        "should never happen");
    m_notification = new KNotification("bluedevilIncomingFile",
        KNotification::Persistent, this);

    m_notification->setText(i18nc(
        "Show a notification asking to authorize or deny an incoming file transfer to this computer from a Bluetooth device.",
        "%1 is sending you the file %2", m_bluetoothDevice->name(), filename));
    QStringList actions;

    actions.append(i18nc("Deny the incoming file transfer", "Cancel"));
    actions.append(i18nc("Button to accept the incoming file transfer and download it in the default download directory", "Accept"));
    actions.append(i18nc("Button to accept the incoming file transfer and show a Save as... dialog that will let the user choose where will the file be downloaded to", "Save as..."));

    m_notification->setActions(actions);

    connect(m_notification, SIGNAL(action1Activated()), this, SLOT(slotCancel()));
    connect(m_notification, SIGNAL(action2Activated()), this, SLOT(slotAccept()));
    connect(m_notification, SIGNAL(action3Activated()), this, SLOT(slotSaveAs()));
    connect(m_notification, SIGNAL(closed()), this, SLOT(slotCancel()));

    m_notification->setPixmap(KIcon("preferences-system-bluetooth").pixmap(42, 42));
    m_notification->setComponentData(KComponentData("bluedevil"));
    m_notification->sendEvent();
}

void ServerSession::slotCancel()
{
    m_notification = 0;
    m_dbusServerSession->Cancel();
}

void ServerSession::slotAccept()
{
    m_notification = 0;
    FileTransferJob *fileTransferJob = new FileTransferJob(this, m_savePath, m_totalBytes);
    KIO::getJobTracker()->registerJob(fileTransferJob);
    fileTransferJob->start();
}

void ServerSession::slotJobFinished()
{
  kDebug();
  m_jobFinished = true;
}

void ServerSession::queueDelete()
{
  if (m_doNotDelete) {
    m_deleteLater = true;
  } else {
    this->deleteLater();
  }
}

void ServerSession::setDoNotDeleteFlag(bool doNotDelete)
{
  m_doNotDelete = doNotDelete;
  if (!m_doNotDelete && m_deleteLater) {
    this->deleteLater();
  }
}


void ServerSession::slotSaveAs()
{
    m_notification = 0;
    // Start file transfer
    FileTransferJob *fileTransferJob = new FileTransferJob(this, m_savePath, m_totalBytes);
    KIO::getJobTracker()->registerJob(fileTransferJob);
    m_jobFinished = false;
    fileTransferJob->start();

    setDoNotDeleteFlag(true);
    KUrl saveUrl = KFileDialog::getSaveUrl(m_savePath);
    if (!saveUrl.isEmpty()) {
      // This means that the user selected a custom file path in Save As dialog to save the incoming
      // file
      if (m_jobFinished) {
        // Move the file if it's already been transfered
        KIO::CopyJob *moveJob = KIO::move(KUrl(m_savePath),KUrl(saveUrl), KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(moveJob, NULL);
      } else {
        // Wait for it to finish
        emit customSaveUrl(saveUrl.url());
      }
    } else {
      // User cancelled the SaveAs dialog, so transfer should be cancelled
      if (m_jobFinished) {
        // Remove the file if it's already been transfered
        KIO::DeleteJob* delJob = KIO::del(m_savePath, KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(delJob, NULL);
      } else {
        // If job is still ongoing, cancel it
        slotCancel();
      }
    }
    setDoNotDeleteFlag(false);
}

