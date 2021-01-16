/*
 *  SPDX-FileCopyrightText: 2010-2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *  SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kioobexftp.h"
#include "debug_p.h"
#include "transferfilejob.h"
#include "version.h"

#include <unistd.h>

#include <QCoreApplication>
#include <QMimeData>
#include <QMimeDatabase>
#include <QTemporaryFile>

#include <KLocalizedString>

#include <BluezQt/ObexTransfer>
#include <BluezQt/PendingCall>

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

static QUrl urlUpDir(const QUrl &url)
{
    const QUrl &u = url.adjusted(QUrl::StripTrailingSlash);
    return u.adjusted(QUrl::RemoveFilename);
}

static bool urlIsRoot(const QUrl &url)
{
    const QString &directory = urlDirectory(url);
    return (directory.isEmpty() || directory == QLatin1String("/")) && urlFileName(url).isEmpty();
}

KioFtp::KioFtp(const QByteArray &pool, const QByteArray &app)
    : SlaveBase(QByteArrayLiteral("obexftp"), pool, app)
    , m_transfer(nullptr)
{
    m_kded = new org::kde::BlueDevil::ObexFtp(QStringLiteral("org.kde.kded5"), QStringLiteral("/modules/bluedevil"), QDBusConnection::sessionBus(), this);
}

void KioFtp::connectToHost()
{
    const QString &target = m_kded->preferredTarget(m_host);

    if (target != QLatin1String("ftp")) {
        if (createSession(target)) {
            return;
        }
        // Fallback to ftp
    }

    createSession(QStringLiteral("ftp"));
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

bool KioFtp::createSession(const QString &target)
{
    QDBusPendingReply<QString> reply = m_kded->session(m_host, target);
    reply.waitForFinished();

    const QString &sessionPath = reply.value();

    if (reply.isError() || sessionPath.isEmpty()) {
        qCDebug(OBEXFTP) << "Create session error" << reply.error().name() << reply.error().message();
        delete m_transfer;
        m_transfer = nullptr;
        m_sessionPath.clear();
        return false;
    }

    if (m_sessionPath != sessionPath) {
        m_statMap.clear();
        delete m_transfer;
        m_transfer = new BluezQt::ObexFileTransfer(QDBusObjectPath(sessionPath));
        m_sessionPath = sessionPath;
    }

    return true;
}

void KioFtp::listDir(const QUrl &url)
{
    if (!testConnection()) {
        return;
    }

    qCDebug(OBEXFTP) << "listdir: " << url;

    infoMessage(i18n("Retrieving information from remote device..."));

    qCDebug(OBEXFTP) << "Asking for listFolder" << url.path();

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
    Q_UNUSED(permissions)
    Q_UNUSED(flags)

    if (!testConnection()) {
        return;
    }

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

    qCDebug(OBEXFTP) << "get" << url;

    QTemporaryFile tempFile(QStringLiteral("%1/kioftp_XXXXXX.%2").arg(QDir::tempPath(), urlFileName(url)));
    tempFile.open();

    copyHelper(url, QUrl::fromLocalFile(tempFile.fileName()));

    QMimeDatabase mimeDatabase;
    const QMimeType &mime = mimeDatabase.mimeTypeForFile(tempFile.fileName());
    mimeType(mime.name());
    qCDebug(OBEXFTP) << "Mime: " << mime.name();

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
    m_host = m_host.replace(QLatin1Char('-'), QLatin1Char(':')).toUpper();

    infoMessage(i18n("Connecting to the device"));

    connectToHost();
}

void KioFtp::del(const QUrl &url, bool isfile)
{
    Q_UNUSED(isfile)

    if (!testConnection()) {
        return;
    }

    qCDebug(OBEXFTP) << "Del: " << url.url();

    if (!changeFolder(urlDirectory(url))) {
        return;
    }

    if (!deleteFile(urlFileName(url))) {
        return;
    }

    finished();
}

void KioFtp::mkdir(const QUrl &url, int permissions)
{
    Q_UNUSED(permissions)

    if (!testConnection()) {
        return;
    }

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

    if (!changeFolder(urlDirectory(src))) {
        return;
    }

    BluezQt::PendingCall *call = m_transfer->copyFile(src.path(), dest.path());
    call->waitForFinished();

    if (call->error()) {
        // Copying files within obexftp is currently not implemented in obexd
        if (call->errorText() == QLatin1String("Not Implemented")) {
            error(KIO::ERR_UNSUPPORTED_ACTION, src.path());
        } else {
            error(KIO::ERR_CANNOT_WRITE, src.path());
        }
        return;
    }

    finished();
}

void KioFtp::copyFromObexftp(const QUrl &src, const QUrl &dest)
{
    qCDebug(OBEXFTP) << "Source: " << src << "Dest:" << dest;

    if (!changeFolder(urlDirectory(src))) {
        return;
    }

    if (!m_statMap.contains(src.toDisplayString())) {
        bool ok;
        listFolder(urlUpDir(src), &ok);
    }

    BluezQt::PendingCall *call = m_transfer->getFile(dest.path(), urlFileName(src));
    call->waitForFinished();

    int size = m_statMap.value(src.toDisplayString()).numberValue(KIO::UDSEntry::UDS_SIZE);
    totalSize(size);

    BluezQt::ObexTransferPtr transfer = call->value().value<BluezQt::ObexTransferPtr>();
    TransferFileJob *getFile = new TransferFileJob(transfer, this);
    getFile->exec();
}

void KioFtp::copyToObexftp(const QUrl &src, const QUrl &dest)
{
    qCDebug(OBEXFTP) << "Source:" << src << "Dest:" << dest;

    if (!changeFolder(urlDirectory(dest))) {
        return;
    }

    BluezQt::PendingCall *call = m_transfer->putFile(src.path(), urlFileName(dest));
    call->waitForFinished();

    int size = QFile(src.path()).size();
    totalSize(size);

    BluezQt::ObexTransferPtr transfer = call->value().value<BluezQt::ObexTransferPtr>();
    TransferFileJob *putFile = new TransferFileJob(transfer, this);
    putFile->exec();
}

void KioFtp::statHelper(const QUrl &url)
{
    if (m_statMap.contains(url.toDisplayString())) {
        qCDebug(OBEXFTP) << "statMap contains the url";
        statEntry(m_statMap.value(url.toDisplayString()));
        return;
    }

    if (urlIsRoot(url)) {
        qCDebug(OBEXFTP) << "Url is root";
        KIO::UDSEntry entry;
        entry.fastInsert(KIO::UDSEntry::UDS_NAME, QStringLiteral("/"));
        entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, 0700);

        qCDebug(OBEXFTP) << "Adding stat cache" << url.toDisplayString();
        m_statMap.insert(url.toDisplayString(), entry);
        statEntry(entry);
        return;
    }

    qCDebug(OBEXFTP) << "statMap does not contains the url";

    if (!changeFolder(urlDirectory(url))) {
        return;
    }

    bool ok;
    listFolder(urlUpDir(url), &ok);
    if (!ok) {
        return;
    }

    if (!m_statMap.contains(url.toDisplayString())) {
        qCWarning(OBEXFTP) << "statMap still does not contains the url!";
    }

    statEntry(m_statMap.value(url.toDisplayString()));
}

QList<KIO::UDSEntry> KioFtp::listFolder(const QUrl &url, bool *ok)
{
    QList<KIO::UDSEntry> list;

    BluezQt::PendingCall *call = m_transfer->listFolder();
    call->waitForFinished();

    if (call->error()) {
        qCDebug(OBEXFTP) << "List folder error" << call->errorText();
        error(KIO::ERR_CANNOT_OPEN_FOR_READING, url.path());
        *ok = false;
        return list;
    }

    const QList<BluezQt::ObexFileTransferEntry> &items = call->value().value<QList<BluezQt::ObexFileTransferEntry>>();

    Q_FOREACH (const BluezQt::ObexFileTransferEntry &item, items) {
        if (!item.isValid()) {
            continue;
        }

        KIO::UDSEntry entry;
        entry.fastInsert(KIO::UDSEntry::UDS_NAME, item.name());
        entry.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, item.label());
        entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, 0700);
        entry.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, item.modificationTime().toSecsSinceEpoch());
        entry.fastInsert(KIO::UDSEntry::UDS_SIZE, item.size());

        if (item.type() == BluezQt::ObexFileTransferEntry::Folder) {
            entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        } else if (item.type() == BluezQt::ObexFileTransferEntry::File) {
            entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
        }

        if (urlIsRoot(url)) {
            updateRootEntryIcon(entry, item.memoryType());
        }

        list.append(entry);

        // Most probably the client of the kio will stat each file
        // so since we are on it, let's cache all of them.
        QUrl statUrl = url;

        if (statUrl.path().endsWith('/')) {
            statUrl.setPath(statUrl.path() + item.name());
        } else {
            statUrl.setPath(statUrl.path() + QLatin1Char('/') + item.name());
        }

        if (!m_statMap.contains(statUrl.toDisplayString())) {
            qCDebug(OBEXFTP) << "Stat:" << statUrl.toDisplayString() << entry.stringValue(KIO::UDSEntry::UDS_NAME)
                             << entry.numberValue(KIO::UDSEntry::UDS_SIZE);
            m_statMap.insert(statUrl.toDisplayString(), entry);
        }
    }

    *ok = true;
    return list;
}

bool KioFtp::changeFolder(const QString &folder)
{
    BluezQt::PendingCall *call = m_transfer->changeFolder(folder);
    call->waitForFinished();

    if (call->error()) {
        error(KIO::ERR_CANNOT_ENTER_DIRECTORY, folder);
        return false;
    }
    return true;
}

bool KioFtp::createFolder(const QString &folder)
{
    BluezQt::PendingCall *call = m_transfer->createFolder(folder);
    call->waitForFinished();

    if (call->error()) {
        error(KIO::ERR_CANNOT_MKDIR, folder);
        return false;
    }
    return true;
}

bool KioFtp::deleteFile(const QString &file)
{
    BluezQt::PendingCall *call = m_transfer->deleteFile(file);
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
            entry.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("drive-removable-media"));
        } else if (memoryType == QLatin1String("MMC")) {
            entry.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("media-flash-sd-mmc"));
        }
    }

    // Android
    if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == QLatin1String("PHONE_MEMORY")) {
        entry.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("Phone memory"));
        entry.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("drive-removable-media"));
    } else if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == QLatin1String("EXTERNAL_MEMORY")) {
        entry.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("External memory"));
        entry.fastInsert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("media-flash-sd-mmc"));
    }
}
