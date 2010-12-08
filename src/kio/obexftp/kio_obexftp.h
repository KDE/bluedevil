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

#include <QtCore/QObject>
#include <QDBusObjectPath>
#include <QEventLoop>

#include <kio/slavebase.h>

class KioFtp
    : public QObject
    , public KIO::SlaveBase
{

Q_OBJECT
public:
    KioFtp(const QByteArray &pool, const QByteArray &app);
    virtual ~KioFtp();

    int processXmlEntries(const KUrl& url, const QString& xml, const char* slot);

    virtual void copy(const KUrl &src, const KUrl &dest, int permissions, KIO::JobFlags flags);
    virtual void listDir(const KUrl &url);
    virtual void setHost(const QString &host, quint16 port, const QString &user, const QString &pass);
    virtual void stat(const KUrl &url);
    virtual void del(const KUrl &url, bool isfile);
    virtual void mkdir(const KUrl&url, int permissions);
    virtual void rename(const KUrl& src, const KUrl& dest, KIO::JobFlags flags);
    virtual void get(const KUrl& url);

private Q_SLOTS:
    void TransferProgress(qulonglong transfered);
    void TransferCompleted();
    void TransferCancelled();
    void ErrorOccurred(const QString&, const QString&);

    void listDirCallback(const KIO::UDSEntry& entry, const KUrl& url);
    void statCallback(const KIO::UDSEntry &entry, const KUrl& url);

    void updateProcess();
    void sessionConnected(QString address);
    void sessionClosed(QString address);

    void wasKilledCheck();
private:
    void copyHelper(const KUrl &src, const KUrl &dest);
    void statHelper(const KUrl &url);
    void launchProgressBar();
    void blockUntilNotBusy(QString address);

private:
    int                          m_counter;
    bool                         m_settingHost;
    QEventLoop                   m_eventLoop;
    QMap<QString, KIO::UDSEntry> m_statMap;
    QString                      m_address;
    QTimer                      *m_timer;
    org::kde::ObexFtp           *m_kded;
};

#endif // KIO_OBEXFTP_H
