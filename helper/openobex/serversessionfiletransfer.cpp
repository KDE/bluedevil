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

#include "openobex/serversessionfiletransfer.h"
#include "openobex/serversession.h"

#include <QTimer>
#include <qdbusconnection.h>
#include <qdbusinterface.h>

#include <KDebug>
#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothremotedevice.h>

using namespace OpenObex;

ServerSessionFileTransfer::ServerSessionFileTransfer(OpenObex::ServerSession* serverSession,
    const QString& filename, const QString& path, qulonglong size) : KJob(serverSession)
{
    setAutoDelete(false);
    m_serverSession = serverSession;
    m_fileName = filename;
    if (path.length() != 0) {
        m_localPath = path;
    } else {
        m_localPath = m_serverSession->path() + "/" + filename;
    }
    m_totalFileSize = size;
    setCapabilities(Killable);

    qDebug() << "m_localPath" << m_localPath;
    Solid::Control::BluetoothManager &bluetoothManager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface* bluetoothInterface = new Solid::Control::BluetoothInterface(bluetoothManager.defaultInterface());
    bluetoothDevice = bluetoothInterface->findBluetoothRemoteDeviceAddr(serverSession->bluetoothAddress());

    m_remoteName =  bluetoothDevice.name();
    m_remoteAddr = bluetoothDevice.address();
}

ServerSessionFileTransfer::~ServerSessionFileTransfer()
{
}

void ServerSessionFileTransfer::start()
{
    QTimer::singleShot(1200, this, SLOT(receiveFiles()));
}

void ServerSessionFileTransfer::reject()
{
    m_serverSession->reject();
    doKill();
}

void ServerSessionFileTransfer::receiveFiles() {
    emit description(this, "Receiving file over bluetooth", QPair<QString, QString>("From", bluetoothDevice.name()), QPair<QString, QString>("To", m_localPath));

    org::openobex::ServerSession* serverSession = m_serverSession->dbusServerSession();
    connect(serverSession, SIGNAL(TransferProgress(qulonglong)),
        this, SLOT(slotTransferProgress(qulonglong)));
    connect(serverSession, SIGNAL(ErrorOccurred(const QString&, const QString&)),
        this, SLOT(slotErrorOccurred(const QString&, const QString&)));
    connect(serverSession, SIGNAL(TransferCompleted()),
        this, SLOT(slotTransferCompleted()));
    kDebug() << "Transfer started ...";
    setTotalAmount(Bytes, m_totalFileSize);
    setProcessedAmount(Bytes, 0);
    m_serverSession->accept();
    m_time = QTime::currentTime();
    m_procesedBytes = 0;
}

void ServerSessionFileTransfer::slotTransferProgress(qulonglong transferred) {
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

void ServerSessionFileTransfer::slotTransferCompleted() {
    kDebug() << "Transfer completed";
    setProcessedAmount(Bytes, m_totalFileSize);
    emitResult();
}

void ServerSessionFileTransfer::slotErrorOccured(const QString& reason1, const QString& reason2) {
    kDebug() << "Transfer disconnected";
    kDebug() << reason1;
    kDebug() << reason2;
    emitResult();
}

void ServerSessionFileTransfer::feedbackReceived()
{
    emitResult();
}

bool ServerSessionFileTransfer::doKill() {
    kDebug() << "doKill called";
    m_serverSession->disconnect();
    m_serverSession->cancel();
    return true;
}

QString ServerSessionFileTransfer::fileName() {
    return m_fileName;
}

QString ServerSessionFileTransfer::localPath() {
    return m_localPath;
}

QString ServerSessionFileTransfer::remoteName() const{
    return m_remoteName;
}

QString ServerSessionFileTransfer::remoteAddr() const{
    return m_remoteAddr;
}

void ServerSessionFileTransfer::setLocalPath(const QString& newPath) {
    m_localPath = newPath;
}

qulonglong ServerSessionFileTransfer::fileSize() {
    return m_totalFileSize;
}
