/*************************************************************************************
 *  Copyright (C) 2010-2012 by Alejandro Fiestas Olivares <afiestas@kde.org>         *
 *  Copyright (C) 2010 UFO Coders <info@ufocoders.com>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "kio_obexftp.h"
#include "kdedobexftp.h"
#include "version.h"
#include "transferfilejob.h"
#include "debug_p.h"

#include <unistd.h>

#include <QMimeData>
#include <QTemporaryFile>
#include <QCoreApplication>
#include <QMimeDatabase>

#include <KLocalizedString>

#include <QBluez/PendingCall>

extern "C" int Q_DECL_EXPORT kdemain(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if (argc != 4) {
        fprintf(stderr, "Usage: kio_obexftp protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    KioFtp slave(argv[2], argv[3]);
    slave.dispatchLoop();
    return 0;
}

static QString urlDirectory(const QUrl &url)
{
    const QUrl &u = url.adjusted(QUrl::StripTrailingSlash);
    return u.adjusted(QUrl::RemoveFilename).path();
}

static QString urlFileName(const QUrl &url)
{
    const QUrl &u = url.adjusted(QUrl::StripTrailingSlash);
    return u.fileName();
}

static bool urlIsRoot(const QUrl &url)
{
    return (urlDirectory(url) == QLatin1String("/") || urlDirectory(url).isEmpty()) && urlFileName(url).isEmpty();
}

KioFtp::KioFtp(const QByteArray &pool, const QByteArray &app)
    : SlaveBase(QByteArrayLiteral("obexftp"), pool, app)
    , m_transfer(0)
{
    m_timer = new QTimer();
    m_timer->setInterval(100);

    m_kded = new org::kde::ObexFtp(QStringLiteral("org.kde.kded5"), QStringLiteral("/modules/obexftpdaemon"),
                                   QDBusConnection::sessionBus(), 0);
}

KioFtp::~KioFtp()
{
    delete m_kded;
}

void KioFtp::launchProgressBar()
{
    connect(m_timer, &QTimer::timeout, this, &KioFtp::updateProcess);
    totalSize(50);
    m_counter = 0;
    m_timer->start();
}

void KioFtp::connectToHost()
{
    QDBusPendingReply<QString> reply = m_kded->session(m_host);
    reply.waitForFinished();

    const QString &sessionPath = reply.value();

    if (reply.isError() || sessionPath.isEmpty()) {
        qCDebug(OBEXFTP) << reply.error().message();
        qCDebug(OBEXFTP) << reply.error().name();

        delete m_transfer;
        m_transfer = Q_NULLPTR;
        m_sessionPath.clear();
        return;
    }

    if (m_sessionPath != sessionPath) {
        m_statMap.clear();
        delete m_transfer;
        m_transfer = new QBluez::ObexFileTransfer(QDBusObjectPath(sessionPath), this);
        m_sessionPath = sessionPath;
    }
}

bool KioFtp::testConnection()
{
    if (!m_kded->isOnline().value()) {
        error(KIO::ERR_SLAVE_DEFINED, i18n("Obexd service is not running."));
        return false;
    }

    connectToHost();

    if (!m_transfer) {
        error(KIO::ERR_CANNOT_CONNECT, m_host);
        return false;
    }
    return true;
}

void KioFtp::updateProcess()
{
    if (m_counter == 49) {
        disconnect(m_timer, &QTimer::timeout, this, &KioFtp::updateProcess);
        m_timer->stop();
        return;
    }

    processedSize(m_counter);
    m_counter++;
}

void KioFtp::listDir(const QUrl &url)
{
    if (!testConnection()) {
        return;
    }

    qCDebug(OBEXFTP) << "listdir: " << url;

    infoMessage(i18n("Retrieving information from remote device..."));

    qCDebug(OBEXFTP) << "Asking for listFolder";

    if (!changeFolder(url.path())) {
        return;
    }

    bool ok;
    const QList<KIO::UDSEntry> &list = listFolder(url, &ok);
    if (!ok) {
        return;
    }

    Q_FOREACH (const KIO::UDSEntry &entry, list) {
        listEntry(entry);
    }

    finished();
}

void KioFtp::copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags)
{
    if (!testConnection()) {
        return;
    }

    Q_UNUSED(permissions)
    Q_UNUSED(flags)

    qCDebug(OBEXFTP) << "copy: " << src.url() << " to " << dest.url();

    copyHelper(src, dest);

    finished();
}

void KioFtp::rename(const QUrl &src, const QUrl &dest, KIO::JobFlags flags)
{
    Q_UNUSED(src)
    Q_UNUSED(dest)
    Q_UNUSED(flags)

    error(KIO::ERR_UNSUPPORTED_ACTION, QString());
}

void KioFtp::get(const QUrl &url)
{
    if (!testConnection()) {
        return;
    }

    QTemporaryFile tempFile(QString(QStringLiteral("%1/kioftp_XXXXXX.%2")).arg(QDir::tempPath(), urlFileName(url)));
    tempFile.open();
    qCDebug(OBEXFTP) << tempFile.fileName();

    copyHelper(url, QUrl(tempFile.fileName()));

    qCDebug(OBEXFTP) << "Getting mimetype";

    QMimeDatabase mimeDatabase;
    const QMimeType &mime = mimeDatabase.mimeTypeForFile(tempFile.fileName());
    mimeType(mime.name());
    qCDebug(OBEXFTP) << "Mime: " << mime.name();

    qCDebug(OBEXFTP) << tempFile.size();
    totalSize(tempFile.size());

    data(tempFile.readAll());

    finished();
}

bool KioFtp::cancelTransfer(QBluez::ObexTransfer *transfer)
{
    return m_kded->cancelTransfer(transfer->objectPath()).value();
}

void KioFtp::setHost(const QString &host, quint16 port, const QString &user, const QString &pass)
{
    Q_UNUSED(port)
    Q_UNUSED(user)
    Q_UNUSED(pass)

    m_host = host;

    infoMessage(i18n("Connecting to the device"));

    connectToHost();
}

void KioFtp::del(const QUrl& url, bool isfile)
{
    if (!testConnection()) {
        return;
    }

    Q_UNUSED(isfile)

    qCDebug(OBEXFTP) << "Del: " << url.url();

    if (!changeFolder(urlDirectory(url))) {
        return;
    }
    if (!deleteFile(urlFileName(url))) {
        return;
    }

    finished();
}

void KioFtp::mkdir(const QUrl& url, int permissions)
{
    if (!testConnection()) {
        return;
    }

    Q_UNUSED(permissions)

    qCDebug(OBEXFTP) << "MkDir: " << url.url();

    if (!changeFolder(urlDirectory(url))) {
        return;
    }
    if (!createFolder(urlFileName(url))) {
        return;
    }

    finished();
}

void KioFtp::stat(const QUrl &url)
{
    if (!testConnection()) {
        return;
    }

    qCDebug(OBEXFTP) << "Stat: " << url.url();
    qCDebug(OBEXFTP) << "Stat Dir: " << urlDirectory(url);
    qCDebug(OBEXFTP) << "Stat File: " << urlFileName(url);
    qCDebug(OBEXFTP) << "Empty Dir: " << urlDirectory(url).isEmpty();

    statHelper(url);

    qCDebug(OBEXFTP) << "Finished";
    finished();
}

void KioFtp::copyHelper(const QUrl &src, const QUrl &dest)
{
    if (src.scheme() == QLatin1String("obexftp") && dest.scheme() == QLatin1String("obexftp")) {
        copyWithinObexftp(src, dest);
        return;
    }

    if (src.scheme() == QLatin1String("obexftp")) {
        copyFromObexftp(src, dest);
        return;
    }

    if (dest.scheme() == QLatin1String("obexftp")) {
        copyToObexftp(src, dest);
        return;
    }

    qCDebug(OBEXFTP) << "This shouldn't happen...";
}

void KioFtp::copyWithinObexftp(const QUrl &src, const QUrl &dest)
{
    qCDebug(OBEXFTP) << "Source: " << src << "Dest:" << dest;
}

void KioFtp::copyFromObexftp(const QUrl &src, const QUrl &dest)
{
    qCDebug(OBEXFTP) << "Source: " << src << "Dest:" << dest;

    if (!changeFolder(urlDirectory(src))) {
        return;
    }

    QBluez::PendingCall *call = m_transfer->getFile(dest.path(), urlFileName(src));
    call->waitForFinished();

    int size = m_statMap[src.toDisplayString()].numberValue(KIO::UDSEntry::UDS_SIZE);
    totalSize(size);

    QBluez::ObexTransfer *transfer = call->value().value<QBluez::ObexTransfer*>();
    TransferFileJob *getFile = new TransferFileJob(transfer, this);
    getFile->exec();
}

void KioFtp::copyToObexftp(const QUrl &src, const QUrl &dest)
{
    qCDebug(OBEXFTP) << "Source:" << src << "Dest:" << dest;

    if (!changeFolder(urlDirectory(dest))) {
        return;
    }

    QBluez::PendingCall *call = m_transfer->putFile(src.path(), urlFileName(dest));
    call->waitForFinished();

    int size = QFile(src.path()).size();
    totalSize(size);

    QBluez::ObexTransfer *transfer = call->value().value<QBluez::ObexTransfer*>();
    TransferFileJob *putFile = new TransferFileJob(transfer, this);
    putFile->exec();
}

void KioFtp::statHelper(const QUrl &url)
{
    qCDebug(OBEXFTP) << url;

    if (m_statMap.contains(url.toDisplayString())) {
        qCDebug(OBEXFTP) << "statMap contains the url";
        statEntry(m_statMap[url.toDisplayString()]);
        return;
    }

    if (urlIsRoot(url)) {
        qCDebug(OBEXFTP) << "Url is root";
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, QString::fromLatin1("/"));
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        entry.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
        entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));

        qCDebug(OBEXFTP) << "Adding stat cached: " << url.toDisplayString();
        m_statMap[url.toDisplayString()] = entry;
        statEntry(entry);
        return;
    }

    qCDebug(OBEXFTP) << "statMap does NOT contains the url";

    if (!changeFolder(urlDirectory(url))) {
        return;
    }

    bool ok;
    const QList<KIO::UDSEntry> &list = listFolder(url, &ok);
    if (!ok) {
        return;
    }

    Q_FOREACH (const KIO::UDSEntry &entry, list) {
        statEntry(entry);
    }

    qCDebug(OBEXFTP) << "Finished";
}

QList<KIO::UDSEntry> KioFtp::listFolder(const QUrl &url, bool *ok)
{
    QList<KIO::UDSEntry> list;

    QBluez::PendingCall *call = m_transfer->listFolder();
    call->waitForFinished();

    if (call->error()) {
        qCDebug(OBEXFTP) << call->errorText();
        error(KIO::ERR_CANNOT_OPEN_FOR_READING, urlDirectory(url));
        *ok = false;
        return list;
    }

    const QList<QBluez::ObexFileTransfer::Item> &items = call->value().value<QList<QBluez::ObexFileTransfer::Item> >();

    Q_FOREACH (const QBluez::ObexFileTransfer::Item &item, items) {
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, item.name);
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, item.label);
        entry.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
        entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, item.modified.toTime_t());
        entry.insert(KIO::UDSEntry::UDS_SIZE, item.size);
        if (item.type == QBluez::ObexFileTransfer::Item::Folder) {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
            entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
        } else {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
        }
        if (urlIsRoot(url)) {
            updateRootEntryIcon(entry, item.memoryType);
        }
        list.append(entry);

        // Most probably the client of the kio will stat each file
        // so since we are on it, let's cache all of them.
        QUrl statUrl = url.adjusted(QUrl::RemoveFilename);
        statUrl.setPath(statUrl.path() + item.name);
        if (!m_statMap.contains(statUrl.toDisplayString())) {
            qCDebug(OBEXFTP) << "Stat: " << statUrl.toDisplayString() << entry.stringValue(KIO::UDSEntry::UDS_NAME) << entry.numberValue(KIO::UDSEntry::UDS_SIZE);
            m_statMap.insert(statUrl.toDisplayString(), entry);
        }
    }

    *ok = true;
    return list;
}

bool KioFtp::changeFolder(const QString &folder)
{
    QBluez::PendingCall *call = m_transfer->changeFolder(folder);
    call->waitForFinished();

    if (call->error()) {
        error(KIO::ERR_CANNOT_ENTER_DIRECTORY, folder);
        return false;
    }
    return true;
}

bool KioFtp::createFolder(const QString &folder)
{
    QBluez::PendingCall *call = m_transfer->createFolder(folder);
    call->waitForFinished();

    if (call->error()) {
        error(KIO::ERR_CANNOT_MKDIR, folder);
        return false;
    }
    return true;
}

bool KioFtp::renameFile(const QString &src, const QString &dest)
{
    // Not implemented yet in org.bluez.obex

    QBluez::PendingCall *call = m_transfer->moveFile(src, dest);
    call->waitForFinished();

    if (call->error()) {
        error(KIO::ERR_CANNOT_RENAME, src);
        return false;
    }
    return true;
}

bool KioFtp::deleteFile(const QString &file)
{
    QBluez::PendingCall *call = m_transfer->deleteFile(file);
    call->waitForFinished();

    if (call->error()) {
        error(KIO::ERR_CANNOT_DELETE, file);
        return false;
    }
    return true;
}

void KioFtp::updateRootEntryIcon(KIO::UDSEntry &entry, const QString &memoryType)
{
    const QString &path = entry.stringValue(KIO::UDSEntry::UDS_NAME);

    // Nokia (mount-points are C: D: E: ...)
    if (path.size() == 2 && path.at(1) == QLatin1Char(':')) {
        if (memoryType.startsWith(QLatin1String("DEV"))) {
            entry.insert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("drive-removable-media"));
        } else if (memoryType == QLatin1String("MMC")) {
            entry.insert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("media-flash-sd-mmc"));
        }
    }
    // Android
    if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == QLatin1String("PHONE_MEMORY")) {
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("Phone memory"));
        entry.insert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("smartphone"));
    } else if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == QLatin1String("EXTERNAL_MEMORY")) {
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("External memory"));
        entry.insert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("media-flash-sd-mmc"));
    }
}
