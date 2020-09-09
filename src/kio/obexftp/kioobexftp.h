/*
 *   This file is part of the KDE project
 *
 *   SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <alex@ufocoders.com>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *   SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIO_OBEXFTP_H
#define KIO_OBEXFTP_H

#include "kdedobexftp.h"

#include <QObject>

#include <KIO/SlaveBase>

#include <BluezQt/ObexFileTransfer>

class KioFtp : public QObject, public KIO::SlaveBase
{
    Q_OBJECT

public:
    KioFtp(const QByteArray &pool, const QByteArray &app);

    void copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags) override;
    void listDir(const QUrl &url) override;
    void setHost(const QString &host, quint16 port, const QString &user, const QString &pass) override;
    void stat(const QUrl &url) override;
    void del(const QUrl &url, bool isfile) override;
    void mkdir(const QUrl &url, int permissions) override;
    void rename(const QUrl &src, const QUrl &dest, KIO::JobFlags flags) override;
    void get(const QUrl &url) override;

    bool cancelTransfer(const QString &transfer);

private:
    void copyHelper(const QUrl &src, const QUrl &dest);
    void copyWithinObexftp(const QUrl &src, const QUrl &dest);
    void copyFromObexftp(const QUrl &src, const QUrl &dest);
    void copyToObexftp(const QUrl &src, const QUrl &dest);
    void statHelper(const QUrl &url);

    QList<KIO::UDSEntry> listFolder(const QUrl &url, bool *ok);
    bool changeFolder(const QString &folder);
    bool createFolder(const QString &folder);
    bool deleteFile(const QString &file);

    void updateRootEntryIcon(KIO::UDSEntry &entry, const QString &memoryType);
    bool createSession(const QString &target);
    void connectToHost();
    bool testConnection();

private:
    QMap<QString, KIO::UDSEntry> m_statMap;
    QString m_host;
    QString m_sessionPath;
    org::kde::BlueDevil::ObexFtp *m_kded;
    BluezQt::ObexFileTransfer *m_transfer;
};

#endif // KIO_OBEXFTP_H
