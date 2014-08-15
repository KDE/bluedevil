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

#ifndef KIO_OBEXFTP_H
#define KIO_OBEXFTP_H

#include "kdedobexftp.h"

#include <QObject>

#include <kio/slavebase.h>

#include <QBluez/ObexFileTransfer>

namespace QBluez {
    class ObexTransfer;
}

class KioFtp : public QObject, public KIO::SlaveBase
{
    Q_OBJECT

public:
    KioFtp(const QByteArray &pool, const QByteArray &app);
    ~KioFtp();

    void copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags) Q_DECL_OVERRIDE;
    void listDir(const QUrl &url) Q_DECL_OVERRIDE;
    void setHost(const QString &host, quint16 port, const QString &user, const QString &pass) Q_DECL_OVERRIDE;
    void stat(const QUrl &url) Q_DECL_OVERRIDE;
    void del(const QUrl &url, bool isfile) Q_DECL_OVERRIDE;
    void mkdir(const QUrl &url, int permissions) Q_DECL_OVERRIDE;
    void rename(const QUrl &src, const QUrl &dest, KIO::JobFlags flags) Q_DECL_OVERRIDE;
    void get(const QUrl &url) Q_DECL_OVERRIDE;

    bool cancelTransfer(QBluez::ObexTransfer *transfer);

private Q_SLOTS:
    void updateProcess();

private:
    void copyHelper(const QUrl &src, const QUrl &dest);
    void copyWithinObexftp(const QUrl &src, const QUrl &dest);
    void copyFromObexftp(const QUrl &src, const QUrl &dest);
    void copyToObexftp(const QUrl &src, const QUrl &dest);
    void statHelper(const QUrl &url);

    QList<KIO::UDSEntry> listFolder(const QUrl &url, bool *ok);
    bool changeFolder(const QString &folder);
    bool createFolder(const QString &folder);
    bool renameFile(const QString &src, const QString &dest);
    bool deleteFile(const QString &file);

    void updateRootEntryIcon(KIO::UDSEntry &entry, const QString &memoryType);
    void connectToHost();
    bool testConnection();

    void launchProgressBar();

    int m_counter;
    QMap<QString, KIO::UDSEntry> m_statMap;
    QString m_host;
    QString m_sessionPath;
    QTimer *m_timer;
    org::kde::ObexFtp *m_kded;
    QBluez::ObexFileTransfer *m_transfer;
};

#endif // KIO_OBEXFTP_H
