/***************************************************************************
 *   Copyright (C) 2008  Tom Patzig <tpatzig@suse.de>                      *
 *   Copyright (C) 2008  Alex Fiestas <alex@eyeos.org>                     *
 *   Copyright (C) 2010  Eduardo Robles Elvira <edulix@gmail.com>          *
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

#include "openobex/filetransfer.h"
#include "openobex/serversession.h"

#include <QTimer>
#include <qdbusconnection.h>
#include <qdbusinterface.h>

#include <KDebug>
#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothremotedevice.h>

using namespace OpenObex;

FileTransfer::FileTransfer(OpenObex::ServerSession* serverSession,
    const QString& filename, const QString& path, qulonglong size) : KJob(serverSession)
{
    setAutoDelete(false);
    m_serverSession = serverSession;
    m_fileName = filename;
    m_localPath = path;
    m_totalFileSize = size;
    setCapabilities(Killable);

    qDebug() << "m_localPath" << m_localPath;
    Solid::Control::BluetoothManager &bluetoothManager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface* bluetoothInterface = new
      Solid::Control::BluetoothInterface(bluetoothManager.defaultInterface());
    m_remoteBluetoothDevice = bluetoothInterface->findBluetoothRemoteDeviceAddr(serverSession->bluetoothAddress());

    m_remoteName =  m_remoteBluetoothDevice.name();
    m_remoteAddr = m_remoteBluetoothDevice.address();
    kDebug() << "m_remoteName" << m_remoteName << "m_remoteAddr" << m_remoteAddr;

    // Connect signals
    connect(serverSession->dbusServerSession(), SIGNAL(TransferProgress(qulonglong)),
        this, SLOT(slotTransferProgress(qulonglong)));
    connect(serverSession->dbusServerSession(), SIGNAL(ErrorOccurred(const QString&, const QString&)),
        this, SLOT(slotErrorOccurred(const QString&, const QString&)));
    connect(serverSession->dbusServerSession(), SIGNAL(TransferCompleted()),
        this, SLOT(slotTransferCompleted()));
    connect(serverSession->dbusServerSession(), SIGNAL(Cancelled()),
        this, SLOT(slotCancelled()));
    connect(serverSession->dbusServerSession(), SIGNAL(Disconnected()),
        this, SLOT(slotDisconnected()));
}

FileTransfer::~FileTransfer()
{
    kDebug() << "~ServerSessionFileTransfer";
}

void FileTransfer::start()
{
    kDebug();
    receiveFiles();
}

void FileTransfer::reject()
{
    kDebug();
    m_serverSession->reject();
    doKill();
}

void FileTransfer::receiveFiles()
{
    kDebug() << "Transfer started ...";
    kDebug() << m_remoteBluetoothDevice.name() << m_localPath;
    emit description(this, "Receiving file over bluetooth", QPair<QString, QString>("From", m_remoteBluetoothDevice.name()), QPair<QString, QString>("To", m_localPath));

    QDBusConnection dbusConnection = m_serverSession->dbusServerSession()->connection();
    dbusConnection.connect("", "", m_serverSession->dbusServerSession()->interface(), "TransferProgress", this, SLOT(slotTransferProgress(qulonglong)));

    setTotalAmount(Bytes, m_totalFileSize);
    setProcessedAmount(Bytes, 0);
    m_serverSession->dbusServerSession()->Accept();
    m_time = QTime::currentTime();
    m_procesedBytes = 0;
}

void FileTransfer::slotTransferProgress(qulonglong transferred)
{
    kDebug() << "Transfer progress ..." << transferred;

    QTime currentTime = QTime::currentTime();
    int time = m_time.secsTo(currentTime);
    if (time != 0) {
        qulonglong diffBytes = transferred - m_procesedBytes;
        float speed = diffBytes / time;
        kDebug() << "Bytes: " << diffBytes << " Speed: " << speed;
        emitSpeed(speed);
        m_time = currentTime;
        m_procesedBytes = transferred;
    }
    setProcessedAmount(Bytes, transferred);
}

void FileTransfer::slotTransferCompleted()
{
    kDebug() << "Transfer completed";
    setProcessedAmount(Bytes, m_totalFileSize);
    emitResult();
}

void FileTransfer::slotErrorOccurred(const QString& reason1, const QString& reason2)
{
    kDebug() << "Transfer disconnected" << reason1 << reason2;
    emitResult();
}


void FileTransfer::slotCancelled()
{
    kDebug() << "slotCancelled()";
    m_serverSession->reject();
    doKill();
}

void FileTransfer::slotDisconnected()
{
    kDebug() << "slotDisconnected()";
    m_serverSession->reject();
    doKill();
}

void FileTransfer::feedbackReceived()
{
    kDebug();
    emitResult();
}

bool FileTransfer::doKill()
{
    kDebug() << "doKill called";
    m_serverSession->disconnect();
    m_serverSession->cancel();
    return true;
}

QString FileTransfer::fileName()
{
    kDebug() << m_fileName;
    return m_fileName;
}

QString FileTransfer::localPath()
{
    kDebug() << m_localPath;
    return m_localPath;
}


void FileTransfer::setLocalPath(const QString& newPath)
{
    m_localPath = newPath;
}

QString FileTransfer::remoteName() const
{
    kDebug() << m_remoteName;
    return m_remoteName;
}

QString FileTransfer::remoteAddr() const
{
    kDebug() << m_remoteAddr;
    return m_remoteAddr;
}

qulonglong FileTransfer::fileSize()
{
    kDebug() << m_totalFileSize;
    return m_totalFileSize;
}
