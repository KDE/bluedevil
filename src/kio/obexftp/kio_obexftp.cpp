/***************************************************************************
 *   This file is part of the KDE project                                  *
 *                                                                         *
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@ufocoders.com>    *
 *   Copyright (C) 2010 UFO Coders <info@ufocoders.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "kio_obexftp.h"
#include "kdedobexftp.h"

#include <KDebug>
#include <KComponentData>
#include <KCmdLineArgs>
#include <KAboutData>
#include <KLocale>
#include <KTemporaryFile>
#include <KMimeType>
#include <KApplication>

extern "C" int KDE_EXPORT kdemain(int argc, char **argv)
{
    KAboutData about("kioobexftp","kioobexftp", ki18n("kioobexftp"), 0);
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
{
    m_settingHost = false;

    m_timer = new QTimer();
    m_timer->setInterval(100);

    m_kded = new org::kde::ObexFtp("org.kde.kded", "/modules/obexftpdaemon", QDBusConnection::sessionBus(), 0);
    connect(m_kded, SIGNAL(sessionConnected(QString)), SLOT(sessionConnected(QString)));
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

    blockUntilNotBusy(url.host());
    QDBusPendingReply<QString> folder = m_kded->listDir(url.host(), url.path());
    folder.waitForFinished();

    kDebug() << folder.value();

    int i = processXmlEntries(url, folder.value(), "listDirCallback");
    totalSize(i);
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

    kDebug() << "setHost: " << host;

    m_kded->stablishConnection(host);
    kDebug() << "Waiting to stablish the connection";
    m_settingHost = true;
    launchProgressBar();
    m_eventLoop.exec();

    m_settingHost = false;
    m_address = host;
    m_statMap.clear();
}

void KioFtp::del(const KUrl& url, bool isfile)
{
    Q_UNUSED(isfile)

    kDebug() << "Del: " << url.url();
    blockUntilNotBusy(url.host());
    m_kded->deleteRemoteFile(url.host(),  url.path()).waitForFinished();
    finished();
}

void KioFtp::mkdir(const KUrl& url, int permissions)
{
    Q_UNUSED(permissions)

    kDebug() << "MkDir: " << url.url();
    blockUntilNotBusy(url.host());
    m_kded->createFolder(url.host(), url.path()).waitForFinished();
    finished();
}

void KioFtp::stat(const KUrl &url)
{
    kDebug() << "Stat: " << url.url();
    kDebug() << "Stat Dir: " << url.directory();
    kDebug() << "Stat File: " << url.fileName();

    if (url.directory() == "/" && url.fileName().isEmpty()) {
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, QString::fromLatin1("/"));
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );
        kDebug() << "Adding stat cached: " << url.prettyUrl();
        m_statMap[url.prettyUrl()] = entry;
        statEntry(entry);

    } else {
        if (m_statMap.contains(url.prettyUrl())) {
            kDebug() << "statMap contains the url";
            statEntry(m_statMap[url.prettyUrl()]);

        } else {
            kDebug() << "statMap NOT contains the url";

            kDebug() << "RetrieveFolderListing";
            blockUntilNotBusy(url.host());
            QDBusPendingReply<QString> folder = m_kded->listDir(url.host(), url.directory());
            kDebug() << "retireve called";
            folder.waitForFinished();
            kDebug() << "RetrieveError: " << folder.error().message();
            kDebug() << folder.value();

            processXmlEntries(url, folder.value(), "statCallback");
        }
    }

    kDebug() << "Finished";
    finished();
}

int KioFtp::processXmlEntries(const KUrl& url, const QString& xml, const char* slot)
{
    QXmlStreamReader* m_xml = new QXmlStreamReader(xml);

    int i = 0;
    while(!m_xml->atEnd()) {
        m_xml->readNext();
        if(m_xml->name() != "folder" &&  m_xml->name() != "file") {
            kDebug() << "Skiping dir: " << m_xml->name();
            continue;
        }
        QXmlStreamAttributes attr = m_xml->attributes();
        if (!attr.hasAttribute("name")) {
            continue;
        }

        KUrl fullKurl = url;
        fullKurl.addPath(attr.value("name").toString());

        const QString fullPath = fullKurl.prettyUrl();

        KIO::UDSEntry entry;
        if (!m_statMap.contains(fullPath)) {
            kDebug() << "path not cached: " << fullPath;
            entry.insert(KIO::UDSEntry::UDS_NAME, attr.value("name").toString());
            entry.insert(KIO::UDSEntry::UDS_CREATION_TIME, attr.value("created").toString());
            entry.insert(KIO::UDSEntry::UDS_ACCESS, 0500);

            if (m_xml->name() == "folder") {
                entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
            } else {
                entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
                entry.insert(KIO::UDSEntry::UDS_SIZE, attr.value("size").toString().toUInt());
                entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, attr.value("modified").toString());
            }

            kDebug() << "Adding surl to map: " << fullPath;
            m_statMap[fullPath] = entry;
        } else {
            kDebug() << "Cached entry :" << fullPath;
            entry = m_statMap.value(fullPath);
        }

        QMetaObject::invokeMethod(this, slot, Q_ARG(KIO::UDSEntry, entry), Q_ARG(KUrl, url));
        ++i;
    }
    return i;
}

void KioFtp::listDirCallback(const KIO::UDSEntry& entry, const KUrl &url)
{
    Q_UNUSED(url)
    kDebug();
    listEntry(entry, false);
}

void KioFtp::statCallback(const KIO::UDSEntry& entry, const KUrl &url)
{
    kDebug() << "FileName : " << url.fileName() << "  " << entry.stringValue(KIO::UDSEntry::UDS_NAME);
    if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == url.fileName()) {
        kDebug() << "setting statEntry : ";
        statEntry(entry);
    }
}

void KioFtp::TransferProgress(qulonglong transfered)
{
    processedSize(transfered);
    kDebug() << "TransferProgress: ";
}

void KioFtp::TransferCompleted()
{
    kDebug() << "TransferCompleted: ";
    disconnect(m_kded, SIGNAL(transferProgress(qulonglong)), this, SLOT(TransferProgress(qulonglong)));
    disconnect(m_kded, SIGNAL(transferCompleted()), this, SLOT(TransferCompleted()));
    disconnect(m_kded, SIGNAL(errorOccurred(QString,QString)), this, SLOT(ErrorOccurred(QString,QString)));
    m_eventLoop.exit();
}

void KioFtp::ErrorOccurred(const QString &name, const QString &msg)
{
//     disconnect(m_session, SIGNAL(TransferProgress(qulonglong)), this, SLOT(TransferProgress(qulonglong)));
//     disconnect(m_session, SIGNAL(TransferCompleted()), this, SLOT(TransferCompleted()));
//     disconnect(m_session, SIGNAL(ErrorOccurred(QString,QString)), this, SLOT(ErrorOccurred(QString,QString)));

    kDebug() << "ERROR ERROR: " << name;
    kDebug() << "ERROR ERROR: " << msg;

    error(KIO::ERR_UNKNOWN, "");
    if (m_eventLoop.isRunning()){
        m_eventLoop.exit();
    }
}

void KioFtp::sessionConnected(QString address)
{
    kDebug() << "Session connected: " << address;

    if (m_settingHost) {
        m_eventLoop.exit();
    }
}

void KioFtp::copyHelper(const KUrl& src, const KUrl& dest)
{
    connect(m_kded, SIGNAL(transferProgress(qulonglong)), this, SLOT(TransferProgress(qulonglong)));
    connect(m_kded, SIGNAL(transferCompleted()), this, SLOT(TransferCompleted()));
    connect(m_kded, SIGNAL(errorOccurred(QString,QString)), this, SLOT(ErrorOccurred(QString,QString)));

    if (src.scheme() == "obexftp") {
        if (m_statMap.contains(src.prettyUrl())) {
            if (m_statMap.value(src.prettyUrl()).isDir()) {
                kDebug() << "Skipping to copy: " << src.prettyUrl();
                error( KIO::ERR_IS_DIRECTORY, src.prettyUrl());
                disconnect(m_kded, SIGNAL(transferProgress(qulonglong)), this, SLOT(TransferProgress(qulonglong)));
                disconnect(m_kded, SIGNAL(transferCompleted()), this, SLOT(TransferCompleted()));
                disconnect(m_kded, SIGNAL(errorOccurred(QString,QString)), this, SLOT(ErrorOccurred(QString,QString)));
                return;
            }
        }

        kDebug() << "CopyingRemoteFile....";
        blockUntilNotBusy(src.host());
        m_kded->copyRemoteFile(src.host(), src.path(), dest.path());
    } else if (dest.scheme() == "obexftp") {
        kDebug() << "Sendingfile....";
        blockUntilNotBusy(dest.host());
        m_kded->sendFile(dest.host(), src.path(), dest.directory());
    }

    connect(m_timer, SIGNAL(timeout()), this, SLOT(wasKilledCheck()));
    m_timer->start();

    m_eventLoop.exec();
    m_timer->stop();
}


void KioFtp::blockUntilNotBusy(QString address)
{
    if (m_kded->isBusy(address).value()) {
        infoMessage(i18n("The device is busy, waiting..."));
        while (m_kded->isBusy(address).value() == true) {
            kDebug() << "Blocking, kded is busy";
            sleep(1);
        }
    }
    kDebug() << "kded is free";
}

void KioFtp::wasKilledCheck()
{
    if (wasKilled()) {
        kDebug() << "slave was killed!";
        m_timer->stop();
        m_kded->Cancel(m_address).waitForFinished();;
        m_eventLoop.exit();
    }
    kDebug() << "Slave is alive";
}
