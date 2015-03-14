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
#include "obexdtypes.h"
#include "transferfilejob.h"
#include "debug_p.h"

#include <QMimeData>
#include <QTemporaryFile>
#include <QCoreApplication>
#include <QMimeDatabase>

#include <KLocalizedString>

#include <unistd.h>

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
    const QString &directory = urlDirectory(url);
    return (directory.isEmpty() || directory == QLatin1String("/")) && urlFileName(url).isEmpty();
}

KioFtp::KioFtp(const QByteArray &pool, const QByteArray &app)
    : SlaveBase(QByteArrayLiteral("obexftp"), pool, app)
    , m_transfer(0)
{
    m_timer = new QTimer();
    m_timer->setInterval(100);

    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<QVariantMapList>();
    m_kded = new org::kde::ObexFtp(QStringLiteral("org.kde.kded5"), QStringLiteral("/modules/obexftpdaemon"),
                                   QDBusConnection::sessionBus(), 0);
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
    // Prefer pcsuite target on S60 devices
    if (m_uuids.contains(QLatin1String("00005005-0000-1000-8000-0002EE000001"))) {
        if (createSession("pcsuite")) {
            return;
        }
        // Fallback to ftp
    }

    createSession("ftp");
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

bool KioFtp::createSession(const QString &target)
{
    QDBusPendingReply<QString> reply = m_kded->session(m_host, target);
    reply.waitForFinished();

    const QString &sessionPath = reply.value();

    if (reply.isError() || sessionPath.isEmpty()) {
        qCDebug(OBEXFTP) << reply.error().message();
        qCDebug(OBEXFTP) << reply.error().name();

        delete m_transfer;
        m_transfer = 0;
        m_sessionPath.clear();
        return false;
    }

    if (m_sessionPath != sessionPath) {
        m_statMap.clear();
        delete m_transfer;
        m_transfer = new org::bluez::obex::FileTransfer1(QStringLiteral("org.bluez.obex"), sessionPath, QDBusConnection::sessionBus());
        m_sessionPath = sessionPath;
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

void KioFtp::rename(const QUrl& src, const QUrl& dest, KIO::JobFlags flags)
{
    Q_UNUSED(src)
    Q_UNUSED(dest)
    Q_UNUSED(flags)

    error(KIO::ERR_UNSUPPORTED_ACTION, QString());
}

void KioFtp::get(const QUrl& url)
{
    if (!testConnection()) {
        return;
    }

    QTemporaryFile tempFile(QString(QStringLiteral("%1/kioftp_XXXXXX.%2")).arg(QDir::tempPath(), urlFileName(url)));
    tempFile.open();//Create the file
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

    QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded5"),
                            QStringLiteral("/modules/bluedevil"),
                            QStringLiteral("org.kde.BlueDevil"),
                            QStringLiteral("device"));
    call << m_host;
    QDBusReply<DeviceInfo> reply = QDBusConnection::sessionBus().call(call);
    DeviceInfo info = reply.value();

    m_uuids = info[QStringLiteral("UUIDs")];

    infoMessage(i18n("Connecting to the device"));

    connectToHost();
}

void KioFtp::del(const QUrl& url, bool isfile)
{
    Q_UNUSED(isfile)

    if (!testConnection()) {
        return;
    }
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

void KioFtp::copyHelper(const QUrl& src, const QUrl& dest)
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

    copyFile(src.path(), dest.path());
}

void KioFtp::copyFromObexftp(const QUrl& src, const QUrl& dest)
{
    qCDebug(OBEXFTP) << "Source: " << src << "Dest:" << dest;

    if (!changeFolder(urlDirectory(src))) {
        return;
    }

    const QString &dbusPath = m_transfer->GetFile(dest.path(), urlFileName(src)).value().path();
    qCDebug(OBEXFTP) << "Path from GetFile:" << dbusPath;

    int size = m_statMap[src.toDisplayString()].numberValue(KIO::UDSEntry::UDS_SIZE);
    totalSize(size);

    TransferFileJob *getFile = new TransferFileJob(dbusPath, this);
    getFile->exec();
}

void KioFtp::copyToObexftp(const QUrl& src, const QUrl& dest)
{
    qCDebug(OBEXFTP) << "Source:" << src << "Dest:" << dest;

    if (!changeFolder(urlDirectory(dest))) {
        return;
    }

    const QString &dbusPath = m_transfer->PutFile(src.path(), urlFileName(dest)).value().path();
    qCDebug(OBEXFTP) << "Path from PutFile: " << dbusPath;

    int size = QFile(src.path()).size();
    totalSize(size);

    TransferFileJob *putFile = new TransferFileJob(dbusPath, this);
    putFile->exec();
}

void KioFtp::statHelper(const QUrl& url)
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
        entry.insert(KIO::UDSEntry::UDS_NAME, QStringLiteral("/"));
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        entry.insert(KIO::UDSEntry::UDS_ACCESS, 0700);

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

    QDBusPendingReply<QVariantMapList> reply = m_transfer->ListFolder();
    reply.waitForFinished();

    if (reply.isError()) {
        error(KIO::ERR_CANNOT_OPEN_FOR_READING, urlDirectory(url));
        *ok = false;
        return list;
    }

    Q_FOREACH (const QVariantMap &item, reply.value()) {
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, item[QStringLiteral("Name")].toString());
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, item[QStringLiteral("Label")].toString());
        entry.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
        entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, QDateTime::fromString(item[QStringLiteral("Modified")].toString(), "yyyyMMddThhmmssZ").toTime_t());
        entry.insert(KIO::UDSEntry::UDS_SIZE, item[QStringLiteral("Size")].toLongLong());
        if (item[QStringLiteral("Type")] == QLatin1String("folder")) {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        } else {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
        }
        if (urlIsRoot(url)) {
            updateRootEntryIcon(entry, item[QStringLiteral("Mem-type")].toString());
        }
        list.append(entry);

        //Most probably the client of the kio will stat each file
        //so since we are on it, let's cache all of them.
        QUrl statUrl = url.adjusted(QUrl::RemoveFilename);
        statUrl.setPath(statUrl.path() + item[QStringLiteral("Name")].toString());
        if (!m_statMap.contains(statUrl.toDisplayString())) {
            qCDebug(OBEXFTP) << "Stat: " << statUrl.toDisplayString() << entry.stringValue(KIO::UDSEntry::UDS_NAME) <<  entry.numberValue(KIO::UDSEntry::UDS_SIZE);
            m_statMap.insert(statUrl.toDisplayString(), entry);
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
        qCDebug(OBEXFTP) << reply.error().message();
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
            entry.insert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("drive-removable-media"));
        } else if (memoryType == QLatin1String("MMC")) {
            entry.insert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("media-flash-sd-mmc"));
        }
    }

    // Android
    if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == QLatin1String("PHONE_MEMORY")) {
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("Phone memory"));
        entry.insert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("drive-removable-media"));
    } else if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == QLatin1String("EXTERNAL_MEMORY")) {
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("External memory"));
        entry.insert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("media-flash-sd-mmc"));
    }
}
