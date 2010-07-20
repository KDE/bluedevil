/*  This file is part of the KDE libraries

    Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>
    Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
    Copyright (C) 2010 UFO Coders <info@ufocoders.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kio_obexftp.h"

#include "qobexftp.h"

#include <KDebug>
#include <KComponentData>
#include <KCmdLineArgs>
#include <KAboutData>
#include <solid/control/bluetoothmanager.h>

#include <KApplication>
#include <KLocale>
#include <QTest>
#include <solid/control/bluetoothremotedevice.h>
#include <QUrlInfo>

extern "C" int KDE_EXPORT kdemain(int argc, char **argv)
{
    KAboutData about("kio_obexftp", 0, ki18n("kio_obexftp"), 0);
    KCmdLineArgs::init(&about);

    KApplication app;
    if (argc != 4) {
        fprintf(stderr, "Usage: kio_obexftp protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    KioObexFtp slave(argv[2], argv[3]);
    slave.dispatchLoop();
    return 0;
}

class KioObexFtpPrivate
{
public:
    KioObexFtpPrivate(KioObexFtp *q);

public:
    QObexFtp   *m_obexFtp;
    KioObexFtp *m_q;
};

KioObexFtpPrivate::KioObexFtpPrivate(KioObexFtp *q)
  : m_q(q)
{
}

KioObexFtp::KioObexFtp(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("obexftp", pool, app)
    , d(new KioObexFtpPrivate(this))
{
    d->m_obexFtp = new QObexFtp(this);
}

KioObexFtp::~KioObexFtp()
{
    delete d->m_obexFtp;
    delete d;
}


void KioObexFtp::listDir(const KUrl &url)
{
    if (!d->m_obexFtp->isConnected()) {
        listEntry(KIO::UDSEntry(), true);
        finished();
        return;
    }

    QList<QUrlInfo> fileList = d->m_obexFtp->list(url.path(KUrl::AddTrailingSlash));
    Q_FOREACH (QUrlInfo fileItem, fileList) {
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, fileItem.name());
        qDebug() << entry.stringValue(KIO::UDSEntry::UDS_NAME);
        entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, fileItem.lastModified().toTime_t());

        int perms = 0;
        if (fileItem.isReadable()) {
            perms = S_IRUSR | S_IRGRP | S_IROTH;
        } else {
            perms = S_IWUSR | S_IWGRP | S_IWOTH;
        }
        entry.insert(KIO::UDSEntry::UDS_ACCESS, perms);

        if (fileItem.isDir()) {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE,S_IFDIR);
        } else {
            entry.insert(KIO::UDSEntry::UDS_SIZE, fileItem.size());
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
        }
        listEntry(entry, false);
    }
    listEntry(KIO::UDSEntry(), true);
    finished();
}

void KioObexFtp::get(const KUrl &url)
{
    if (!d->m_obexFtp->isConnected()) {
        error(KIO::ERR_SLAVE_DEFINED, i18n("Not connected"));
        return;
    }

    QByteArray fileData = d->m_obexFtp->get(url.path());
    data(fileData);
    if (!fileData.isEmpty()) {
        data(QByteArray());
    }
    finished();
}

void KioObexFtp::setHost(const QString &constHostname, quint16 port, const QString &user,
                         const QString &pass)
{
//     if (d->m_obexFtp->host() == constHostname) {
//         return;
//     }

    QString hostname = constHostname;
    hostname = hostname.replace('-',':').toUpper();
    if (hostname.isEmpty() && d->m_obexFtp->isConnected()) {
        d->m_obexFtp->close();
    } else {
        d->m_obexFtp->connectToHost(hostname);
        slaveStatus(constHostname, d->m_obexFtp->isConnected());
    }
}

void KioObexFtp::del(const KUrl& url, bool isfile)
{
//     Q_UNUSED(isfile);
//     qDebug() << url  << url.path();
// 
//     if (!d->obexFtp->isConnected()) {
//         error(KIO::ERR_SLAVE_DEFINED, i18n("Not connected"));
//         return;
//     }
// 
//     int result = d->obexFtp->remove(url.path());
//     if (result < 0) {
//         error(KIO::ERR_SLAVE_DEFINED, i18n("Could not delete file"));
//         return;
//     }
//     finished();
}

void KioObexFtp::mkdir(const KUrl& url, int permissions)
{
//     Q_UNUSED(permissions);
//     qDebug() << url << url.path();
// 
//     if (!d->obexFtp->isConnected()) {
//         error(KIO::ERR_SLAVE_DEFINED, i18n("Not connected"));
//         return;
//     }
// 
//     int result = d->obexFtp->mkdir(url.path());
//     if (result < 0) {
//         error(KIO::ERR_SLAVE_DEFINED, i18n("Could not create directory"));
//         return;
//     }
//     finished();
}

void KioObexFtp::slave_status()
{
    QString host = d->m_obexFtp->host().replace(":", "-");
    slaveStatus(host, d->m_obexFtp->isConnected());
}

void KioObexFtp::stat(const KUrl &url)
{
    if (!d->m_obexFtp->isConnected()) {
        error(KIO::ERR_SLAVE_DEFINED, "Not connected");
        return;
    }

    QUrlInfo fileItem = d->m_obexFtp->stat(url.path());
    if (fileItem == QUrlInfo()) {
        error(KIO::ERR_SLAVE_DEFINED, "Could not stat file");
        return;
    }
    KIO::UDSEntry entry;
    entry.insert(KIO::UDSEntry::UDS_NAME, fileItem.name());
    entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, fileItem.lastModified().toTime_t());
    entry.insert(KIO::UDSEntry::UDS_ACCESS, S_IRWXU|S_IRWXG|S_IRWXO);
    if (fileItem.isDir()) {
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE,S_IFDIR);
    } else {
        entry.insert(KIO::UDSEntry::UDS_SIZE, fileItem.size());
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
    }
    statEntry(entry);
    finished();
}

#include "kio_obexftp.moc"
