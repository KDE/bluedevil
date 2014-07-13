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

KioFtp::KioFtp(const QByteArray &pool, const QByteArray &app)
    : SlaveBase(QByteArrayLiteral("obexftp"), pool, app)
    , m_settingHost(false)
    , m_transfer(0)
{
    qDBusRegisterMetaType<QVariantMapList>();

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
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateProcess()));
    totalSize(50);
    m_counter = 0;
    m_timer->start();
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
    qCDebug(OBEXFTP) << "listdir: " << url;

    infoMessage(i18n("Retrieving information from remote device..."));

    qCDebug(OBEXFTP) << "Asking for listFolder";

    //TODO: Check if changeFolder fails
    m_transfer->ChangeFolder(url.path()).waitForFinished();

    QDBusPendingReply <QVariantMapList > reply = m_transfer->ListFolder();
    reply.waitForFinished();

    if (reply.isError()) {
        qCDebug(OBEXFTP) << reply.error().message();
        error(KIO::ERR_SLAVE_DEFINED, i18n("Bluetooth is not enabled"));
        return;
    }
    QVariantMapList folderList = reply.value();
    Q_FOREACH(const QVariantMap folder, folderList) {
        KIO::UDSEntry entry = entryFromInfo(folder);

        QUrl statUrl = url.adjusted(QUrl::RemoveFilename);
        statUrl.setPath(statUrl.path() + folder[QStringLiteral("Name")].toString());
        if (!m_statMap.contains(statUrl.toDisplayString())) {
            qCDebug(OBEXFTP) << "Stat: " << statUrl.toDisplayString() << entry.numberValue(KIO::UDSEntry::UDS_SIZE);
            m_statMap.insert(statUrl.toDisplayString(), entry);
        }

        listEntry(entry);
    }

    finished();
}

void KioFtp::copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags)
{
    Q_UNUSED(permissions)
    Q_UNUSED(flags)

    qCDebug(OBEXFTP) << "copy: " << src.url() << " to " << dest.url();

    copyHelper(src, dest);

    finished();
}

void KioFtp::rename(const QUrl& src, const QUrl& dest, KIO::JobFlags flags)
{
    Q_UNUSED(src)
    Q_UNUSED(dest)
    Q_UNUSED(flags)

    error(KIO::ERR_UNSUPPORTED_ACTION, src.toDisplayString());
}

void KioFtp::get(const QUrl& url)
{
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


void KioFtp::setHost(const QString &host, quint16 port, const QString &user, const QString &pass)
{
    Q_UNUSED(port)
    Q_UNUSED(user)
    Q_UNUSED(pass)

    infoMessage(i18n("Connecting to the device"));

    qCDebug(OBEXFTP) << "setHost: " << host;

    qCDebug(OBEXFTP) << "Waiting to stablish the connection 2";
    QDBusPendingReply <QString > reply = m_kded->session(host);
    reply.waitForFinished();

    qCDebug(OBEXFTP) << "AFTER" << reply.isError();
    if (reply.isError()) {
        qCDebug(OBEXFTP) << reply.error().message();
        qCDebug(OBEXFTP) << reply.error().name();
    }

    qCDebug(OBEXFTP) << "Got a path" << reply.value();

    m_address = host;
    m_sessionPath = reply.value();
    m_transfer = new org::bluez::obex::FileTransfer1("org.bluez.obex", m_sessionPath, QDBusConnection::sessionBus());
    m_statMap.clear();
}

void KioFtp::del(const QUrl& url, bool isfile)
{
    Q_UNUSED(isfile)

    qCDebug(OBEXFTP) << "Del: " << url.url();
    m_transfer->ChangeFolder(urlDirectory(url)).waitForFinished();
    m_transfer->Delete(urlFileName(url)).waitForFinished();
    finished();
}

void KioFtp::mkdir(const QUrl& url, int permissions)
{
    Q_UNUSED(permissions)

    qCDebug(OBEXFTP) << "MkDir: " << url.url();
    m_transfer->ChangeFolder(urlDirectory(url)).waitForFinished();
    m_transfer->CreateFolder(urlFileName(url)).waitForFinished();
    finished();
}

void KioFtp::stat(const QUrl &url)
{
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
        error(KIO::ERR_UNSUPPORTED_ACTION, src.toDisplayString());
        //TODO: with obexd this seems possible, we should at least try
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
    finished();
}

void KioFtp::copyFromObexftp(const QUrl& src, const QUrl& dest)
{
    qCDebug(OBEXFTP) << "Source: " << src << "Dest:" << dest;

    //Just in case the url is not in the stat, some times happens...
    if (!m_statMap.contains(src.toDisplayString())) {
        qCDebug(OBEXFTP) << "The url is not in the cache, stating it";
        statHelper(src);
    }

    if (m_statMap.value(src.toDisplayString()).isDir()) {
        qCDebug(OBEXFTP) << "Skipping to copy: " << src.toDisplayString();
        //TODO: Check if dir copying works with obexd
        error(KIO::ERR_IS_DIRECTORY, src.toDisplayString());
        return;
    }

    qCDebug(OBEXFTP) << "Changing dir:" << urlDirectory(src);
    m_transfer->ChangeFolder(urlDirectory(src)).waitForFinished();

    QString dbusPath = m_transfer->GetFile(dest.path(), urlFileName(src)).value().path();
    qCDebug(OBEXFTP) << "Path from GetFile:" << dbusPath;

    int size = m_statMap[src.toDisplayString()].numberValue(KIO::UDSEntry::UDS_SIZE);
    TransferFileJob *getFile = new TransferFileJob(dbusPath, this);
    getFile->setSize(size);
    getFile->exec();

    finished();
}

void KioFtp::copyToObexftp(const QUrl& src, const QUrl& dest)
{
    qCDebug(OBEXFTP) << "Source:" << src << "Dest:" << dest;

    qCDebug(OBEXFTP) << "Changing folder: " << urlDirectory(dest);
    m_transfer->ChangeFolder(urlDirectory(dest));
    QString dbusPath = m_transfer->PutFile(src.path(), urlFileName(dest)).value().path();
    qCDebug(OBEXFTP) << "Path from PutFile: " << dbusPath;

    QFile file(src.path());
    TransferFileJob *putFile = new TransferFileJob(dbusPath, this);
    putFile->setSize(file.size());
    putFile->exec();

    finished();
}

void KioFtp::statHelper(const QUrl& url)
{
    qCDebug(OBEXFTP) << url;

    if (m_statMap.contains(url.toDisplayString())) {
        qCDebug(OBEXFTP) << "statMap contains the url";
        statEntry(m_statMap[url.toDisplayString()]);
        return;
    }

    if ((urlDirectory(url) == QLatin1String("/") || urlDirectory(url).isEmpty()) && urlFileName(url).isEmpty()) {
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
    //TODO: Check if changeFolder fails
    m_transfer->ChangeFolder(urlDirectory(url)).waitForFinished();
    QVariantMapList folderList = m_transfer->ListFolder().value();
    qCDebug(OBEXFTP) << urlDirectory(url) << folderList.count();
    Q_FOREACH(const QVariantMap folder, folderList) {
        KIO::UDSEntry entry = entryFromInfo(folder);

        QString fileName = folder[QStringLiteral("Name")].toString();
        if (urlFileName(url) == fileName) {
            statEntry(entry);
        }

        //Most probably the client of the kio will stat each file
        //so since we are on it, let's cache all of them.
        QUrl statUrl = url.adjusted(QUrl::RemoveFilename);
        statUrl.setPath(statUrl.path() + fileName);
        if (!m_statMap.contains(statUrl.toDisplayString())) {
            qCDebug(OBEXFTP) << "Stat: " << statUrl.toDisplayString() << entry.stringValue(KIO::UDSEntry::UDS_NAME) <<  entry.numberValue(KIO::UDSEntry::UDS_SIZE);
            m_statMap.insert(statUrl.toDisplayString(), entry);
        }
    }

    qCDebug(OBEXFTP) << "Finished";
}

KIO::UDSEntry KioFtp::entryFromInfo(const QVariantMap& info)
{
    qCDebug(OBEXFTP) << info;

    KIO::UDSEntry entry;
    entry.insert(KIO::UDSEntry::UDS_NAME, info[QStringLiteral("Name")].toString());
    entry.insert(KIO::UDSEntry::UDS_CREATION_TIME, info[QStringLiteral("Created")].toString());
    entry.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
    entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, info[QStringLiteral("Modified")].toString());

    if (info[QStringLiteral("Type")].toString() == QLatin1String("folder")) {
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    } else {
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
        entry.insert(KIO::UDSEntry::UDS_SIZE, info[QStringLiteral("Size")].toLongLong());
    }

    return entry;
}
