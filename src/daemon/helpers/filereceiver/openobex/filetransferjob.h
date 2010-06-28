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

#ifndef OPENOBEX_FILETRANSFERJOB_H
#define OPENOBEX_FILETRANSFERJOB_H

#include "openobex/serversession.h"
#include <KJob>
#include <KUrl>

class QDBusInterface;

namespace BlueDevil {
    class Device;
};

namespace OpenObex {

class FileTransferJob : public KJob
{
Q_OBJECT
public:
    FileTransferJob(OpenObex::ServerSession *serverSession, const KUrl &saveUrl, qulonglong size);
    ~FileTransferJob();

    void start();
    void reject();
    qulonglong fileSize();

protected:
    bool doKill();


private Q_SLOTS:
    void receiveFiles();
    void slotTransferProgress(qulonglong);
    void slotTransferCompleted();
    void slotErrorOccured(const QString &, const QString &);

    /**
     * Sets the custom save url, s that when the file transfer finishes the file is moved there.
     * See @ref ServerSession::customSaveUrl for details.
     */
    void setCustomSaveUrl(const QString &customSaveUrl);

    /**
     * Because plasma won't show notifications for less than 1.2 seconds, we'll wait to send the
     * emitResult() signal for successfully completed transfers at least for 1.2 seconds. This
     * function will be called 1.2 seconds after the start() function was called to see if the job
     * has already finished, and it will finish if m_transferCompleted is true, or else it will set
     * m_canFinish to true so that when transferCompleted() it will directly emitResult().
     */
    void checkFinish();

    /**
     * If m_canFinish is true, this function will directly call to emitResult(). Else, it will set
     * m_transferCompleted to true.
     *
     * @see checkFinish()
     */
    void transferCompleted();

private Q_SLOTS:
    void Cancelled();
    void Disconnected();

private:
    OpenObex::ServerSession *m_serverSession;
    qulonglong m_totalFileSize;
    QString m_customSaveUrl;
    QTime m_time;
    qlonglong m_procesedBytes;
    KUrl m_url;
    bool m_canFinish;
    bool m_transferCompleted;
};

}

#endif // OPENOBEX_FILETRANSFERJOB_H
