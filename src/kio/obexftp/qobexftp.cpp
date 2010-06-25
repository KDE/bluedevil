/*
    Implementation of the client side of the OBEX FTP protocol.
    Copyright (c) 2010 Eduardo Robles Elvira <edulix@gmail.com>

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

class QObexFtpPrivate
{
public:
    obexftp_client_t *cli;
    bool connected;
    QString host;
};

QObexFtp::QObexFtp(QObject *parent)
    : QObject(parent), d(new QObexFtpPrivate())
{
    d->cli = 0;
}

QObexFtp::~QObexFtp()
{
    if (d) {
        close();
    }
}

QUrlInfo QObexFtp::stat(const QString& path)
{
    Q_ASSERT(d->cli != 0);

    QByteArray ba = path.toLatin1();
    kDebug() << "1";
    stat_entry_t *stat = obexftp_stat(d->cli, ba);
    kDebug() << "2";
    if (stat == NULL) {
        // For some reason, some devices do not like to list "/" dir, so if it's that dir we will
        // send a fake answer here. What else could we do..?
        if (path == "/") {
            QUrlInfo currentItem;
            currentItem.setName("/");
            currentItem.setDir(true);
            currentItem.setFile(false);
            return currentItem;
        }
        kDebug() << "error stating dir " << path;
        return QUrlInfo();
    }
    QUrlInfo currentItem;
    currentItem.setName(stat->name);
    currentItem.setLastModified(QDateTime::fromTime_t(stat->mtime));

    kDebug() << "3";
    if (stat->mode & S_IFDIR) {
        currentItem.setDir(true);
        currentItem.setFile(false);
    } else if (stat->mode & S_IFREG) {
        currentItem.setDir(false);
        currentItem.setFile(true);
    }
    kDebug() << "4";
    currentItem.setSize(stat->size);

    return currentItem;
}

int QObexFtp::connectToHost(const QString& host)
{
    kDebug() << host;
    // Closes the current connection if any
    if (d->connected) {
        close();
    }

    QByteArray ba = host.toLatin1();
    d->cli = obexftp_open(OBEX_TRANS_BLUETOOTH, 0, 0, 0);
    int ret = obexftp_connect(d->cli, ba.data(), 0);
    d->connected = ret >= 0;

    if (d->connected) {
        d->host = host;
    }
    kDebug() << ret;
    return ret;
}

void QObexFtp::close()
{
    if (d->cli != 0) {
        obexftp_close(d->cli);
        d->cli = 0;
    }
    d->host = QString();
}

QList<QUrlInfo> QObexFtp::list(const QString& pathToDir)
{
    Q_ASSERT(d->cli != 0);
    QList<QUrlInfo> fileList;
    QByteArray ba = pathToDir.toLatin1();
    int listres = obexftp_list(d->cli, 0, ba.data());

    // There was some kind of error, so return an empty list
    // TODO: Improve error handling in this case
    if (listres < 0) {
        kDebug() << "error listing: " << listres;
        return fileList;
    }

    QString dirlist(QString::fromUtf8((const char*)d->cli->buf_data));
    kDebug() << dirlist;
    QDomDocument doc;
    doc.setContent(dirlist);

    QDomNode listing = doc.elementsByTagName("folder-listing").item(0);
    for (int i = 0; i < listing.childNodes().size(); i++) {
        QDomNode node = listing.childNodes().item(i);
        if (node.nodeName()=="folder" || node.nodeName()=="file") {
            QUrlInfo currentItem;
            currentItem.setName(node.attributes().namedItem("name").nodeValue());

            if (node.attributes().contains("modified")) {
                kDebug() << node.attributes().namedItem("modified").nodeValue();
                QDateTime date = QDateTime::fromString(
                    node.attributes().namedItem("modified").nodeValue(), "yyyyMMddTHHmmss");
                currentItem.setLastModified(date);
            }

            if (node.attributes().contains("created")) {
                kDebug() << node.attributes().namedItem("created").nodeValue();
                QDateTime date = QDateTime::fromString(
                    node.attributes().namedItem("created").nodeValue(), "yyyyMMddTHHmmss");
                currentItem.setLastModified(date);
            }

            if (node.attributes().contains("user-perm")) {
                QString userPerm = node.attributes().namedItem("user-perm").nodeValue().toUpper();
                if (userPerm.contains("R")) {
                    currentItem.setReadable(true);
                }
                if (userPerm.contains("W")) {
                    currentItem.setWritable(true);
                }
            }

            if (node.nodeName() == "folder") {
                currentItem.setDir(true);
            } else {
                currentItem.setFile(true);
                currentItem.setSize(node.attributes().namedItem("size").nodeValue().toInt());
            }
            fileList.push_back(currentItem);
        }
    }

    return fileList;
}

QByteArray QObexFtp::get(const QString &pathToFile)
{
    Q_ASSERT(d->cli != 0);

    QByteArray ba = pathToFile.toLatin1();
    int ret = obexftp_get(d->cli, NULL, ba);

    // There was some kind of error, so return an empty list
    // TODO: Improve error handling in this case
    if (ret < 0) {
        return QByteArray();
    }

    return QByteArray((const char*)d->cli->buf_data, d->cli->buf_size);
}

int QObexFtp::put(const QByteArray &data, const QString &pathToFile)
{
    QByteArray ba = pathToFile.toLatin1();
    return obexftp_put_data(d->cli, data, data.size(), ba);
}

int QObexFtp::remove(const QString &path)
{
    Q_ASSERT(d->cli != 0);

    QByteArray ba = path.toLatin1();
    return obexftp_del(d->cli, ba);
}

int QObexFtp::rename(const QString &originPath, const QString &destinationPath)
{
    Q_ASSERT(d->cli != 0);

    QByteArray ba = originPath.toLatin1();
    QByteArray ba2 = destinationPath.toLatin1();
    return obexftp_rename(d->cli, ba, ba2);
}

bool QObexFtp::isConnected() const
{
    return d->connected;
}

int QObexFtp::mkdir(const QString &path)
{
    Q_ASSERT(d->cli != 0);

    QByteArray ba = path.toLatin1();
    return obexftp_mkpath(d->cli, ba);
}

QString QObexFtp::host() const
{
    return d->host;
}


#include "qobexftp.moc"
