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
#include "obexd_transfer.h"
#include "obexd_file_transfer.h"
#include "kdedobexftp.h"
#include "version.h"
#include <KDebug>
#include <KComponentData>
#include <KCmdLineArgs>
#include <KAboutData>
#include <KLocale>
#include <KTemporaryFile>
#include <KMimeType>
#include <KApplication>
#include "obexdtypes.h"
#include "transferfilejob.h"

#include <unistd.h>

extern "C" int KDE_EXPORT kdemain(int argc, char **argv)
{
    KAboutData about("kioobexftp", "bluedevil", ki18n("kioobexftp"), bluedevil_version);
    KCmdLineArgs::init(&about);

    KApplication app;

    if (argc != 4) {
        fprintf(stderr, "Usage: kio_obexftp protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    KioFtp slave(argv[2], argv[3]);
    slave.dispatchLoop();
    return 0;
}

static bool urlIsRoot(const KUrl &url)
{
    return (url.directory() == QLatin1String("/") || url.directory().isEmpty()) && url.fileName().isEmpty();
}

KioFtp::KioFtp(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("obexftp", pool, app)
    , m_transfer(0)
{
    m_timer = new QTimer();
    m_timer->setInterval(100);

    qDBusRegisterMetaType<QVariantMapList>();
    m_kded = new org::kde::ObexFtp("org.kde.kded", "/modules/obexftpdaemon", QDBusConnection::sessionBus(), 0);
}

KioFtp::~KioFtp()
{
    delete m_kded;
}

void KioFtp::launchProgressBar()
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateProcess()));
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
        kDebug() << reply.error().message();
        kDebug() << reply.error().name();

        delete m_transfer;
        m_transfer = 0;
        m_sessionPath.clear();
        return;
    }

    if (m_sessionPath != sessionPath) {
        m_statMap.clear();
        delete m_transfer;
        m_transfer = new org::bluez::obex::FileTransfer1("org.bluez.obex", sessionPath, QDBusConnection::sessionBus());
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
        error(KIO::ERR_COULD_NOT_CONNECT, m_host);
        return false;
    }
    return true;
}

void KioFtp::updateProcess()
{
    if (m_counter == 49) {
        disconnect(m_timer, SIGNAL(timeout()), this, SLOT(updateProcess()));
        m_timer->stop();
        return;
    }

    processedSize(m_counter);
    m_counter++;
}

void KioFtp::listDir(const KUrl &url)
{
    if (!testConnection()) {
        return;
    }

    kDebug() << "listdir: " << url;

    infoMessage(i18n("Retrieving information from remote device..."));

    kDebug() << "Asking for listFolder" << url.path();

    if (!changeFolder(url.path())) {
        return;
    }

    bool ok;
    const QList<KIO::UDSEntry> &list = listFolder(url, &ok);
    if (!ok) {
        return;
    }

    Q_FOREACH (const KIO::UDSEntry &entry, list) {
        listEntry(entry, false);
    }

    listEntry(KIO::UDSEntry(), true);
    finished();
}

void KioFtp::copy(const KUrl &src, const KUrl &dest, int permissions, KIO::JobFlags flags)
{
    Q_UNUSED(permissions)
    Q_UNUSED(flags)

    if (!testConnection()) {
        return;
    }

    kDebug() << "copy: " << src.url() << " to " << dest.url();

    copyHelper(src, dest);
}

void KioFtp::rename(const KUrl& src, const KUrl& dest, KIO::JobFlags flags)
{
    Q_UNUSED(src)
    Q_UNUSED(dest)
    Q_UNUSED(flags)

    error(KIO::ERR_UNSUPPORTED_ACTION, QString());
}

void KioFtp::get(const KUrl& url)
{
    if (!testConnection()) {
        return;
    }

    KTemporaryFile tempFile;
    tempFile.setSuffix(url.fileName());
    tempFile.open(); // Create the file
    kDebug() << tempFile.fileName();

    copyHelper(url, KUrl(tempFile.fileName()));

    kDebug() << "Getting mimetype";
    KMimeType::Ptr mime = KMimeType::findByPath(tempFile.fileName());
    mimeType(mime->name());
    kDebug() << "Mime: " << mime->name();

    kDebug() << tempFile.size();
    totalSize(tempFile.size());

    data(tempFile.readAll());

    finished();
}

bool KioFtp::cancelTransfer(const QString &transfer)
{
    return m_kded->cancelTransfer(transfer);
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

void KioFtp::del(const KUrl& url, bool isfile)
{
    Q_UNUSED(isfile)

    if (!testConnection()) {
        return;
    }
    if (!changeFolder(url.directory())) {
        return;
    }
    if (!deleteFile(url.fileName())) {
        return;
    }

    finished();
}

void KioFtp::mkdir(const KUrl& url, int permissions)
{
    Q_UNUSED(permissions)

    if (!testConnection()) {
        return;
    }

    kDebug() << "MkDir: " << url.url();

    if (!changeFolder(url.directory())) {
        return;
    }
    if (!createFolder(url.fileName())) {
        return;
    }

    finished();
}

void KioFtp::stat(const KUrl &url)
{
    if (!testConnection()) {
        return;
    }

    kDebug() << "Stat: " << url.url();
    kDebug() << "Stat Dir: " << url.directory();
    kDebug() << "Stat File: " << url.fileName();
    kDebug() << "Empty Dir: " << url.directory().isEmpty();

    statHelper(url);

    kDebug() << "Finished";
    finished();
}

void KioFtp::copyHelper(const KUrl& src, const KUrl& dest)
{
    if (src.scheme() == "obexftp" && dest.scheme() == "obexftp") {
        copyWithinObexftp(src, dest);
        return;
    }

    if (src.scheme() == "obexftp") {
        copyFromObexftp(src, dest);
        return;
    }

    if (dest.scheme() == "obexftp") {
        copyToObexftp(src, dest);
        return;
    }

    kDebug() << "This shouldn't happen...";
}

void KioFtp::copyWithinObexftp(const KUrl &src, const KUrl &dest)
{
    kDebug() << "Source: " << src << "Dest:" << dest;

    if (!copyFile(src.path(), dest.path())) {
        return;
    }

    finished();
}

void KioFtp::copyFromObexftp(const KUrl& src, const KUrl& dest)
{
    kDebug() << "Source: " << src << "Dest:" << dest;

    if (!changeFolder(src.directory())) {
        return;
    }

    QString dbusPath = m_transfer->GetFile(dest.path(), src.fileName()).value().path();
    kDebug() << "Path from GetFile:" << dbusPath;

    int size = m_statMap[src.prettyUrl()].numberValue(KIO::UDSEntry::UDS_SIZE);
    totalSize(size);

    TransferFileJob *getFile = new TransferFileJob(dbusPath, this);
    getFile->exec();

    finished();
}

void KioFtp::copyToObexftp(const KUrl& src, const KUrl& dest)
{
    kDebug() << "Source:" << src << "Dest:" << dest;

    if (!changeFolder(dest.directory())) {
        return;
    }

    QString dbusPath = m_transfer->PutFile(src.path(), dest.fileName()).value().path();
    kDebug() << "Path from PutFile: " << dbusPath;

    int size = QFile(src.path()).size();
    totalSize(size);

    TransferFileJob *putFile = new TransferFileJob(dbusPath, this);
    putFile->exec();

    finished();
}

void KioFtp::statHelper(const KUrl& url)
{
    kDebug() << url;

    if (m_statMap.contains(url.prettyUrl())) {
        kDebug() << "statMap contains the url";
        statEntry(m_statMap[url.prettyUrl()]);
        return;
    }

    if (urlIsRoot(url)) {
        kDebug() << "Url is root";
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, QLatin1String("/"));
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        entry.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
        entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );

        kDebug() << "Adding stat cached: " << url.prettyUrl();
        m_statMap[url.prettyUrl()] = entry;
        statEntry(entry);
        return;
    }

    kDebug() << "statMap does NOT contains the url";

    if (!changeFolder(url.directory())) {
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

    kDebug() << "Finished";
}

QList<KIO::UDSEntry> KioFtp::listFolder(const KUrl &url, bool *ok)
{
    QList<KIO::UDSEntry> list;

    QDBusPendingReply<QVariantMapList> reply = m_transfer->ListFolder();
    reply.waitForFinished();

    if (reply.isError()) {
        error(KIO::ERR_CANNOT_OPEN_FOR_READING, url.directory());
        *ok = false;
        return list;
    }

    Q_FOREACH (const QVariantMap &item, reply.value()) {
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, item["Name"].toString());
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, item["Label"].toString());
        entry.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
        entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, QDateTime::fromString(item["Modified"].toString(), "yyyyMMddThhmmssZ").toTime_t());
        entry.insert(KIO::UDSEntry::UDS_SIZE, item["Size"].toLongLong());
        if (item["Type"] == QLatin1String("folder")) {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
            entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, "inode/directory");
        } else {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
        }
        if (urlIsRoot(url)) {
            updateRootEntryIcon(entry, item["Mem-type"].toString());
        }
        list.append(entry);

        // Most probably the client of the kio will stat each file
        // so since we are on it, let's cache all of them.
        KUrl statUrl(url);
        statUrl.setFileName(item["Name"].toString());
        if (!m_statMap.contains(statUrl.prettyUrl())) {
            kDebug() << "Stat: " << statUrl.prettyUrl() << entry.stringValue(KIO::UDSEntry::UDS_NAME) << entry.numberValue(KIO::UDSEntry::UDS_SIZE);
            m_statMap.insert(statUrl.prettyUrl(), entry);
        }
    }

    *ok = true;
    return list;
}

bool KioFtp::changeFolder(const QString &folder)
{
    QDBusPendingReply<> reply = m_transfer->ChangeFolder(folder);
    reply.waitForFinished();

    if (reply.isError()) {
        error(KIO::ERR_CANNOT_ENTER_DIRECTORY, folder);
        return false;
    }
    return true;
}

bool KioFtp::createFolder(const QString &folder)
{
    QDBusPendingReply<> reply = m_transfer->CreateFolder(folder);
    reply.waitForFinished();

    if (reply.isError()) {
        error(KIO::ERR_COULD_NOT_MKDIR, folder);
        return false;
    }
    return true;
}

bool KioFtp::copyFile(const QString &src, const QString &dest)
{
    QDBusPendingReply<> reply = m_transfer->CopyFile(src, dest);
    reply.waitForFinished();

    if (reply.isError()) {
        kDebug() << reply.error().message();
        // Copying files within obexftp is currently not implemented in obexd
        if (reply.error().message() == QLatin1String("Not Implemented")) {
            error(KIO::ERR_UNSUPPORTED_ACTION, src);
        } else {
            error(KIO::ERR_COULD_NOT_WRITE, src);
        }
        return false;
    }
    return true;
}

bool KioFtp::deleteFile(const QString &file)
{
    QDBusPendingReply<> reply = m_transfer->Delete(file);
    reply.waitForFinished();

    if (reply.isError()) {
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
            entry.insert(KIO::UDSEntry::UDS_ICON_NAME, "drive-removable-media");
        } else if (memoryType == QLatin1String("MMC")) {
            entry.insert(KIO::UDSEntry::UDS_ICON_NAME, "media-flash-sd-mmc");
        }
    }
    // Android
    if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == QLatin1String("PHONE_MEMORY")) {
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("Phone memory"));
        entry.insert(KIO::UDSEntry::UDS_ICON_NAME, "smartphone");
    } else if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == QLatin1String("EXTERNAL_MEMORY")) {
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("External memory"));
        entry.insert(KIO::UDSEntry::UDS_ICON_NAME, "media-flash-sd-mmc");
    }
}

