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

KioFtp::KioFtp(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("obexftp", pool, app)
    , m_transfer(0)
{
    m_settingHost = false;

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
    kDebug() << "listdir: " << url;

    infoMessage(i18n("Retrieving information from remote device..."));

    kDebug() << "Asking for listFolder";

    //TODO: Check if changeFolder fails
    m_transfer->ChangeFolder(url.path()).waitForFinished();

    QDBusPendingReply <QVariantMapList > reply = m_transfer->ListFolder();
    reply.waitForFinished();

    if (reply.isError()) {
        kDebug() << reply.error().message();
        error(KIO::ERR_SLAVE_DEFINED, i18n("Bluetooth is not enabled"));
        finished();
        return;
    }
    QVariantMapList folderList = reply.value();
    Q_FOREACH(const QVariantMap folder, folderList) {
        KIO::UDSEntry entry = entryFromInfo(folder);

        KUrl statUrl(url);
        statUrl.setFileName(folder["Name"].toString());
        if (!m_statMap.contains(statUrl.prettyUrl())) {
            kDebug() << "Stat: " << statUrl.prettyUrl() << entry.numberValue(KIO::UDSEntry::UDS_SIZE);
            m_statMap.insert(statUrl.prettyUrl(), entry);
        }

        listEntry(entry, false);
    }

    listEntry(KIO::UDSEntry(), true);
    finished();
}

void KioFtp::copy(const KUrl &src, const KUrl &dest, int permissions, KIO::JobFlags flags)
{
    Q_UNUSED(permissions)
    Q_UNUSED(flags)

    kDebug() << "copy: " << src.url() << " to " << dest.url();

    copyHelper(src, dest);

    finished();
}

void KioFtp::rename(const KUrl& src, const KUrl& dest, KIO::JobFlags flags)
{
    Q_UNUSED(src)
    Q_UNUSED(dest)
    Q_UNUSED(flags)

    error(KIO::ERR_UNSUPPORTED_ACTION, src.prettyUrl());
    finished();
}

void KioFtp::get(const KUrl& url)
{
    KTemporaryFile tempFile;
    tempFile.setSuffix(url.fileName());
    tempFile.open();//Create the file
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


void KioFtp::setHost(const QString &host, quint16 port, const QString &user, const QString &pass)
{
    Q_UNUSED(port)
    Q_UNUSED(user)
    Q_UNUSED(pass)

    infoMessage(i18n("Connecting to the device"));

    kDebug() << "setHost: " << host;

    kDebug() << "Waiting to stablish the connection 2";
    QDBusPendingReply <QString > reply = m_kded->session(host);
    reply.waitForFinished();

    kDebug() << "AFTER" << reply.isError();
    if (reply.isError()) {
        kDebug() << reply.error().message();
        kDebug() << reply.error().name();
    }

    kDebug() << "Got a path" << reply.value();

    m_address = host;
    m_sessionPath = reply.value();
    m_transfer = new org::bluez::obex::FileTransfer1("org.bluez.obex", m_sessionPath, QDBusConnection::sessionBus());
    m_statMap.clear();
}

void KioFtp::del(const KUrl& url, bool isfile)
{
    Q_UNUSED(isfile)

    kDebug() << "Del: " << url.url();
    m_transfer->ChangeFolder(url.directory()).waitForFinished();
    m_transfer->Delete(url.fileName()).waitForFinished();
    finished();
}

void KioFtp::mkdir(const KUrl& url, int permissions)
{
    Q_UNUSED(permissions)

    kDebug() << "MkDir: " << url.url();
    m_transfer->ChangeFolder(url.directory()).waitForFinished();
    m_transfer->CreateFolder(url.fileName()).waitForFinished();
    finished();
}

void KioFtp::stat(const KUrl &url)
{
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
        error(KIO::ERR_UNSUPPORTED_ACTION, src.prettyUrl());
        //TODO: with obexd this seems possible, we should at least try
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
    finished();
}

void KioFtp::copyFromObexftp(const KUrl& src, const KUrl& dest)
{
    kDebug() << "Source: " << src << "Dest:" << dest;

    //Just in case the url is not in the stat, some times happens...
    if (!m_statMap.contains(src.prettyUrl())) {
        kDebug() << "The url is not in the cache, stating it";
        statHelper(src);
    }

    if (m_statMap.value(src.prettyUrl()).isDir()) {
        kDebug() << "Skipping to copy: " << src.prettyUrl();
        //TODO: Check if dir copying works with obexd
        error(KIO::ERR_IS_DIRECTORY, src.prettyUrl());
        return;
    }

    kDebug() << "Changing dir:" << src.directory();
    m_transfer->ChangeFolder(src.directory()).waitForFinished();

    QString dbusPath = m_transfer->GetFile(dest.path(), src.fileName()).value().path();
    kDebug() << "Path from GetFile:" << dbusPath;

    int size = m_statMap[src.prettyUrl()].numberValue(KIO::UDSEntry::UDS_SIZE);
    TransferFileJob *getFile = new TransferFileJob(dbusPath, this);
    getFile->setSize(size);
    getFile->exec();

    finished();
}

void KioFtp::copyToObexftp(const KUrl& src, const KUrl& dest)
{
    kDebug() << "Source:" << src << "Dest:" << dest;

    kDebug() << "Changing folder: " << dest.directory();
    m_transfer->ChangeFolder(dest.directory());
    QString dbusPath = m_transfer->PutFile(src.path(), dest.fileName()).value().path();
    kDebug() << "Path from PutFile: " << dbusPath;

    QFile file(src.path());
    TransferFileJob *putFile = new TransferFileJob(dbusPath, this);
    putFile->setSize(file.size());
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

    if ((url.directory() == "/" || url.directory().isEmpty()) && url.fileName().isEmpty()) {
        kDebug() << "Url is root";
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, QString::fromLatin1("/"));
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        entry.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
        entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );

        kDebug() << "Adding stat cached: " << url.prettyUrl();
        m_statMap[url.prettyUrl()] = entry;
        statEntry(entry);

        return;
    }

    kDebug() << "statMap does NOT contains the url";
    //TODO: Check if changeFolder fails
    m_transfer->ChangeFolder(url.directory()).waitForFinished();
    QVariantMapList folderList = m_transfer->ListFolder().value();
    kDebug() << url.directory() << folderList.count();
    Q_FOREACH(const QVariantMap folder, folderList) {
        KIO::UDSEntry entry = entryFromInfo(folder);

        QString fileName = folder["Name"].toString();
        if (url.fileName() == fileName) {
            statEntry(entry);
        }

        //Most probably the client of the kio will stat each file
        //so since we are on it, let's cache all of them.
        KUrl statUrl(url);
        statUrl.setFileName(fileName);
        if (!m_statMap.contains(statUrl.prettyUrl())) {
            kDebug() << "Stat: " << statUrl.prettyUrl() << entry.stringValue(KIO::UDSEntry::UDS_NAME) <<  entry.numberValue(KIO::UDSEntry::UDS_SIZE);
            m_statMap.insert(statUrl.prettyUrl(), entry);
        }
    }

    kDebug() << "Finished";
}

KIO::UDSEntry KioFtp::entryFromInfo(const QVariantMap& info)
{
    kDebug() << info;

    KIO::UDSEntry entry;
    entry.insert(KIO::UDSEntry::UDS_NAME, info["Name"].toString());
    entry.insert(KIO::UDSEntry::UDS_CREATION_TIME, info["Created"].toString());
    entry.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
    entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, info["Modified"].toString());

    if (info["Type"].toString() == QLatin1String("folder")) {
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    } else {
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
        entry.insert(KIO::UDSEntry::UDS_SIZE, info["Size"].toLongLong());
    }

    return entry;
}