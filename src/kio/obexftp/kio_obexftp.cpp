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

    changeCurrentFolder(url);

    QDBusPendingReply <QVariantMapList > reply = m_transfer->ListFolder();
    reply.waitForFinished();

    kDebug() << "Got answer3";

    QVariantMapList folderList = reply.value();
    kDebug() << "Content: " << folderList;
    Q_FOREACH(const QVariantMap folder, folderList) {
        KIO::UDSEntry entry = entryFromInfo(folder);
        listEntry(entry, false);
        if (!m_statMap.contains(url.prettyUrl())) {
            m_statMap.insert(url.prettyUrl(), entry);
        }
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
//     blockUntilNotBusy(url.host());
//     m_kded->deleteRemoteFile(url.host(),  url.path()).waitForFinished();
//     finished();
}

void KioFtp::mkdir(const KUrl& url, int permissions)
{
    Q_UNUSED(permissions)

    kDebug() << "MkDir: " << url.url();
//     blockUntilNotBusy(url.host());
//     m_kded->createFolder(url.host(), url.path()).waitForFinished();
//     finished();
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

void KioFtp::TransferProgress(qulonglong transfered)
{
//     processedSize(transfered);
//     wasKilledCheck();
    kDebug() << "TransferProgress: ";
}

void KioFtp::TransferCompleted()
{
    kDebug() << "TransferCompleted: ";
//     disconnect(m_kded, SIGNAL(Cancelled()), this, SLOT(TransferCancelled()));
//     disconnect(m_kded, SIGNAL(transferProgress(qulonglong)), this, SLOT(TransferProgress(qulonglong)));
//     disconnect(m_kded, SIGNAL(transferCompleted()), this, SLOT(TransferCompleted()));
//     disconnect(m_kded, SIGNAL(errorOccurred(QString,QString)), this, SLOT(ErrorOccurred(QString,QString)));
//     m_eventLoop.exit();
}

void KioFtp::TransferCancelled()
{
    kDebug() << "TransferCancelled";
//     disconnect(m_kded, SIGNAL(Cancelled()), this, SLOT(TransferCancelled()));
//     disconnect(m_kded, SIGNAL(transferProgress(qulonglong)), this, SLOT(TransferProgress(qulonglong)));
//     disconnect(m_kded, SIGNAL(transferCompleted()), this, SLOT(TransferCompleted()));
//     disconnect(m_kded, SIGNAL(errorOccurred(QString,QString)), this, SLOT(ErrorOccurred(QString,QString)));
//     error(KIO::ERR_USER_CANCELED, "");
//     m_eventLoop.exit();
}


void KioFtp::ErrorOccurred(const QString &name, const QString &msg)
{
//     disconnect(m_session, SIGNAL(TransferProgress(qulonglong)), this, SLOT(TransferProgress(qulonglong)));
//     disconnect(m_session, SIGNAL(TransferCompleted()), this, SLOT(TransferCompleted()));
//     disconnect(m_session, SIGNAL(ErrorOccurred(QString,QString)), this, SLOT(ErrorOccurred(QString,QString)));

    kDebug() << "ERROR ERROR: " << name;
    kDebug() << "ERROR ERROR: " << msg;

//     error(KIO::ERR_UNKNOWN, "");
//     if (m_eventLoop.isRunning()){
//         m_eventLoop.exit();
//     }
}

void KioFtp::sessionConnected(QString address)
{
    kDebug() << "Session connected: " << address;
/*
    if (m_settingHost) {
        m_eventLoop.exit();
    }*/
}

void KioFtp::sessionClosed(QString address)
{
    kDebug() << "Session closed: " << address;
//     if (m_eventLoop.isRunning()) {
//         m_eventLoop.exit();
//     }
//
//     if (m_settingHost) {
//         infoMessage(i18n("Can't connect to the device"));
//     } else {
//         infoMessage(i18n("Connection closed"));
//     }
//
//     if (m_counter != 0) {
//         processedSize(50);
//         m_counter = 0;
//     }
}


void KioFtp::copyHelper(const KUrl& src, const KUrl& dest)
{
//     connect(m_kded, SIGNAL(Cancelled()), this, SLOT(TransferCancelled()));
//     connect(m_kded, SIGNAL(transferProgress(qulonglong)), this, SLOT(TransferProgress(qulonglong)));
//     connect(m_kded, SIGNAL(transferCompleted()), this, SLOT(TransferCompleted()));
//     connect(m_kded, SIGNAL(errorOccurred(QString,QString)), this, SLOT(ErrorOccurred(QString,QString)));
//
//     if (src.scheme() == "obexftp" && dest.scheme() == "obexftp") {
//         error(KIO::ERR_UNSUPPORTED_ACTION, src.prettyUrl());
//         return;
//     }
//
//     if (src.scheme() == "obexftp") {
//         kDebug() << "scheme is obexftp";
//         kDebug() << src.prettyUrl();
//         //Just in case the url is not in the stat, some times happens...
//         if (!m_statMap.contains(src.prettyUrl())) {
//             kDebug() << "The url is not in the cache, stating it";
//             statHelper(src);
//         }
//
//         if (m_statMap.value(src.prettyUrl()).isDir()) {
//             kDebug() << "Skipping to copy: " << src.prettyUrl();
//             error( KIO::ERR_IS_DIRECTORY, src.prettyUrl());
//             disconnect(m_kded, SIGNAL(Cancelled()), this, SLOT(TransferCancelled()));
//             disconnect(m_kded, SIGNAL(transferProgress(qulonglong)), this, SLOT(TransferProgress(qulonglong)));
//             disconnect(m_kded, SIGNAL(transferCompleted()), this, SLOT(TransferCompleted()));
//             disconnect(m_kded, SIGNAL(errorOccurred(QString,QString)), this, SLOT(ErrorOccurred(QString,QString)));
//             return;
//         }
//
//         kDebug() << "CopyingRemoteFile....";
//
//         int size = m_statMap[src.prettyUrl()].numberValue(KIO::UDSEntry::UDS_SIZE);
//         totalSize(size);
//
//         blockUntilNotBusy(src.host());
//         m_kded->copyRemoteFile(src.host(), src.path(), dest.path());
//     } else if (dest.scheme() == "obexftp") {
//         kDebug() << "Sendingfile....";
//         QFile file(dest.url());
//         totalSize(file.size());
//         blockUntilNotBusy(dest.host());
//         m_kded->sendFile(dest.host(), src.path(), dest.directory());
//     }
//
//     m_eventLoop.exec();
//     kDebug() << "Copy end";
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
        entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );

        kDebug() << "Adding stat cached: " << url.prettyUrl();
        m_statMap[url.prettyUrl()] = entry;
        statEntry(entry);

        return;
    }

    kDebug() << "statMap does NOT contains the url";
    changeCurrentFolder(url.directory());
    QVariantMapList folderList = m_transfer->ListFolder().value();
    Q_FOREACH(const QVariantMap folder, folderList) {
        KIO::UDSEntry entry = entryFromInfo(folder);

        if (url.fileName() == folder["Name"].toString()) {
            statEntry(entry);
        }

        //Most probably the client of the kio will stat each file
        //so since we are on it, let's cache all of them.
        if (!m_statMap.contains(url.path())) {
            m_statMap.insert(url.path(), entry);
        }
    }

    kDebug() << "Finished";
}

void KioFtp::blockUntilNotBusy(QString address)
{
//     if (m_kded->isBusy(address).value()) {
//         infoMessage(i18n("The device is busy, waiting..."));
//         while (m_kded->isBusy(address).value() == true) {
//             kDebug() << "Blocking, kded is busy";
//             sleep(1);
//         }
//         infoMessage("");
//     }
    kDebug() << "kded is free";
}

void KioFtp::wasKilledCheck()
{
//     if (wasKilled()) {
//         kDebug() << "slave was killed!";
//         m_kded->Cancel(m_address).waitForFinished();;
//         m_eventLoop.exit();
//     }
    kDebug() << "Slave is alive";
}


KIO::UDSEntry KioFtp::entryFromInfo(const QVariantMap& info)
{
    kDebug() << "Uayu: " << info;

    KIO::UDSEntry entry;
    entry.insert(KIO::UDSEntry::UDS_NAME, info["Name"].toString());
    entry.insert(KIO::UDSEntry::UDS_CREATION_TIME, info["Created"].toString());
    entry.insert(KIO::UDSEntry::UDS_ACCESS, 0500);
    entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, info["Modified"].toString());

    if (info["Type"].toString() == QLatin1String("folder")) {
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    } else {
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
        entry.insert(KIO::UDSEntry::UDS_SIZE, 10);
    }

    return entry;
}

void KioFtp::changeCurrentFolder(const KUrl& url)
{
    kDebug() << url;
    QString path = url.path(KUrl::KUrl::RemoveTrailingSlash);
    kDebug() << path;
    QStringList dirList = path.split("/");

    if (dirList.isEmpty()) {
        return;
    }

    kDebug() << dirList;
    m_transfer->ChangeFolder("/"); //Reset it back to root
    Q_FOREACH(const QString &dir, dirList) {
        if (dir.isEmpty()) continue;

        kDebug() << "Changes" << dir;
        m_transfer->ChangeFolder(dir);
    }
}