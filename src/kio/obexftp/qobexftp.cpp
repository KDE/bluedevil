/*
    Implementation of the client side of the OBEX FTP protocol.
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

#include "qobexftp.h"

// obexftp
#include <obexftp/client.h>

// Qt
#include <QtNetwork/QUrlInfo>
#include <QtXml/QDomDocument>

#include <KDebug>

class QObexFtp::Private
{
public:
    Private();
    ~Private();

    obexftp_client_t *m_client;
    bool              m_connected;
    QString           m_host;
};

QObexFtp::Private::Private()
    : m_client(obexftp_open(OBEX_TRANS_BLUETOOTH, 0, 0, 0))
    , m_connected(false)
{
}

QObexFtp::Private::~Private()
{
    if (m_connected) {
        obexftp_disconnect(m_client);
    }
    obexftp_close(m_client);
}

QObexFtp::QObexFtp(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

QObexFtp::~QObexFtp()
{
    delete d;
}

QUrlInfo QObexFtp::stat(const QString& path)
{
    stat_entry_t *const stat = obexftp_stat(d->m_client, path.toUtf8().data());

    QUrlInfo res;

    if (!stat) {
        if (path == "/") {
            QUrlInfo currentItem;
            currentItem.setName("/");
            currentItem.setDir(true);
            currentItem.setFile(false);
            return currentItem;
        }
        return res;
    }

    res.setName(QString::fromUtf8(stat->name));
    res.setLastModified(QDateTime::fromTime_t(stat->mtime));
    res.setDir(stat->mode & S_IFDIR);
    res.setFile(stat->mode & S_IFREG);
    res.setSize(stat->size);

    return res;
}

int QObexFtp::connectToHost(const QString& host)
{
    if (d->m_connected && d->m_host == host) {
        return 0;
    }

    if (d->m_connected) {
        obexftp_disconnect(d->m_client);
    }

    const int res = obexftp_connect(d->m_client, host.toUtf8().data(), 0);
    d->m_connected = res >= 0;
    d->m_host = host;

    return res;
}

QList<QUrlInfo> QObexFtp::list(const QString& pathToDir)
{
    void *dir = obexftp_opendir(d->m_client, pathToDir.toUtf8().data());

    QList<QUrlInfo> res;

    if (!dir) {
        return res;
    }

    stat_entry_t *entry;
    while ((entry = obexftp_readdir(dir))) {
        QUrlInfo info;
        info.setName(QString::fromUtf8(entry->name));
        info.setLastModified(QDateTime::fromTime_t(entry->mtime));
        info.setDir(entry->mode & S_IFDIR);
        info.setFile(entry->mode & S_IFREG);
        info.setSize(entry->size);
        res << info;
    }

    obexftp_closedir(dir);

    return res;
}

QByteArray QObexFtp::get(const QString &pathToFile)
{
    const int res = obexftp_get(d->m_client, 0, pathToFile.toUtf8().data());

    if  (res < 0) {
        return QByteArray();
    }

    return QByteArray((const char*) d->m_client->buf_data, d->m_client->buf_size);
}

int QObexFtp::put(const QByteArray &data, const QString &pathToFile)
{
    return obexftp_put_data(d->m_client, data, data.size(), pathToFile.toUtf8().data());
}

int QObexFtp::remove(const QString &path)
{
    return obexftp_del(d->m_client, path.toUtf8().data());
}

int QObexFtp::rename(const QString &originPath, const QString &destinationPath)
{
    return obexftp_rename(d->m_client, originPath.toUtf8().data(), destinationPath.toUtf8().data());
}

bool QObexFtp::isConnected() const
{
    return d->m_connected;
}

int QObexFtp::mkdir(const QString &path)
{
    return obexftp_mkpath(d->m_client, path.toUtf8().data());
}

QString QObexFtp::host() const
{
    return d->m_host;
}

void QObexFtp::close()
{
    d->m_connected = false;
    obexftp_disconnect(d->m_client);
    d->m_host.clear();
}

#include "qobexftp.moc"
