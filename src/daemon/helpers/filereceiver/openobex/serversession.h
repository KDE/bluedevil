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

#ifndef OPENOBEX_SERVERSESSION_H
#define OPENOBEX_SERVERSESSION_H

#include <QObject>
#include "server_session_interface.h"

class KNotification;

namespace BlueDevil {
    class Device;
};

namespace OpenObex {

class FileTransferJob;

/**
 * A Server Session is created for each incoming file transfer, it shows a KNotification asking the
 * user if he wants to accept the file transfer, and creates the file transfer job if it's accepted.
 */
class ServerSession : public QObject
{
    Q_OBJECT
public:
    ServerSession(const QString& path, const QString& bluetoothAddress);
    virtual ~ServerSession();

    /**
     * Path of the ServerSession dbus object.
     */
    QString path();

    /**
     * Dbus server session object. Used by the file transfer job to administrate the file transfer.
     */
    org::openobex::ServerSession* dbusServerSession();

    /**
     * @return The remote bluetooth device.
     */
    BlueDevil::Device *device();

    /**
     * Call this function if you want to delete this object, never call to deleteLater() or
     * directly delete serverSessionObject.
     *
     * This function assures that the object is not deleted while showing the saveAs dialog.
     */
    void queueDelete();

public Q_SLOTS:
    void slotCancelled();
    void slotDisconnected();

    /**
     * Called when a new petition for incoming file transfer is received. Shows a KNotification
     * asking the user if he wants to accept the file transfer here.
     */
    void slotTransferStarted(const QString& filename, const QString& localPath,
        qulonglong totalBytes);

protected Q_SLOTS:
    friend class FileTransferJob;
    /**
     * Only called by FileTransferJob. This function sets m_jobFinished to true, so that in
     * slotSaveAs(), after showing the SaveAs dialog, we can handle differently the file depending
     * on if the FileTransferJob is still ongoing or not.
     */
    void slotJobFinished();

private Q_SLOTS:
    /**
     * Called when the user clicks the Accept button in "bluedevilIncomingFile" dialog. It starts
     * the file transfer, creating a FileTransferJob.
     */
    void slotAccept();

    /**
     * Called when the user clicks the Cancel button in "bluedevilIncomingFile" dialog. It cancels
     * the file transfer.
     */
    void slotCancel();

    /**
     * Called when the user clicks the SaveAs button in "bluedevilIncomingFile" dialog. It starts
     * the file transfer, creating a FileTransferJob, while the user chooses where will the file
     * be stored locally. If the user finally cancels the SaveAs dialog, the transfer is cancelled.
     */
    void slotSaveAs();

Q_SIGNALS:
    /**
     * Emitted when in a save dialog the user finially chooses where he wants the file to be
     * received but the file transfer is still ongoing (see @a slotSaveAs for details), so that the
     * file transfer itself can detect this signals and move the file there when it finishes.
     */
    void customSaveUrl(const QString& saveUrl);

private:
    /**
     * Sets the do not delete flag, and calls to deleteLater() if necessary.
     * @internal
     */
    void setDoNotDeleteFlag(bool arg1);

    /// Path of the ServerSession dbus object
    QString m_path;

    /// ServerSession dbus object
    org::openobex::ServerSession* m_dbusServerSession;

    /// Remote Bluetooth device
    BlueDevil::Device *m_bluetoothDevice;

    /**
     * These vars store information received when the slotTransferStarted gets called and
     * needs to be stored so that it's available when the user decides to accept the file transfer
     */
    /// Indicates where the file is being downloaded
    QString m_savePath;

    /// Indicates where the path the user chose in case he clicked "Save As.."
    QString m_customSavePath;

    /**
     * Reported size of the incoming file being transfered in bytes.
     *
     * @note some devices report a wrong (zero) file size, so this file size cannot be completely
     * trusted.
     */
    qulonglong m_totalBytes;

    /// bluedevilIncomingFile notification dialog.
    KNotification* m_notification;

    /// Indicates if the job has finished or not
    bool m_jobFinished;

    /// When m_doNotDelete is true, the object shouldn't be deleted.
    bool m_doNotDelete;

    /// Set when someone calls to queueDelete();
    bool m_deleteLater;
};

}

#endif // OPENOBEX_SERVERSESSION_H
