/*  This file is part of the KDE libraries

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

#include "kio_obexftp.h"
#include "obexftpmanager.h"
#include "obexftpsession.h"

#include <KDebug>
#include <KComponentData>
#include <KCmdLineArgs>
#include <KAboutData>
#include <KLocale>
#include <KApplication>

#define ENSURE_SESSION_CREATED(url) if (!m_session) {       \
                                        createSession(url); \
                                    }

extern "C" int KDE_EXPORT kdemain(int argc, char **argv)
{
    KAboutData about("kio_obexftp", 0, ki18n("kio_obexftp"), 0);
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
    : SlaveBase("obexftp", pool, app), m_session(0), m_manager(0)
{
}

KioFtp::~KioFtp()
{
    m_session->Disconnect().waitForFinished();
    m_session->Close().waitForFinished();
    delete m_manager;
    delete m_session;
}

void KioFtp::createSession(const KUrl &address)
{
    infoMessage(i18n("Connecting to the remote device..."));

    m_address = address.path().mid(1, 17);
    kDebug() << "Got address: " << m_address;

    m_manager = new org::openobex::Manager("org.openobex", "/org/openobex", QDBusConnection::sessionBus(), 0);
    connect(m_manager, SIGNAL(SessionConnected(QDBusObjectPath)), this, SLOT(sessionCreated(QDBusObjectPath)));

    QDBusPendingReply <QDBusObjectPath > rep = m_manager->CreateBluetoothSession(QString(m_address).replace("-", ":"), "00:00:00:00:00:00", "ftp");

    m_eventLoop.exec();

    kDebug() << "SessionError: " << rep.error().message();
    kDebug() << "SessionPath: " << rep.value().path();

    const QString sessioPath = rep.value().path();
    m_session = new org::openobex::Session("org.openobex", sessioPath, QDBusConnection::sessionBus(), 0);

    kDebug() << "Private Ctor Ends";
}

void KioFtp::changeDirectory(const KUrl& url)
{
    kDebug() << "ChangeUrl: " << url.path();
    QStringList list = url.path().split("/");

    kDebug() << "List of itens: " << list;

    m_session->ChangeCurrentFolderToRoot().waitForFinished();
    kDebug() << "We're in root now";

    Q_FOREACH(const QString &dir, list) {
        if (!dir.isEmpty() && dir != m_address) {
            kDebug() << "Changing to: " << dir;
            QDBusPendingReply <void > a = m_session->ChangeCurrentFolder(dir);
            a.waitForFinished();
            kDebug()  << "Change Error: " << a.error().message();
        } else {
            kDebug() << "Skyping" << dir;
        }
    }
}

void KioFtp::listDir(const KUrl &url)
{
    kDebug() << "listdir: " << url;
    ENSURE_SESSION_CREATED(url)

    infoMessage(i18n("Retrieving information from remote device..."));

    if (url.directory() != "/") {
        changeDirectory(url);
    }

    QDBusPendingReply<QString> folder = m_session->RetrieveFolderListing();
    folder.waitForFinished();

    kDebug() << folder.value();

    int i = processXmlEntries(url, folder.value(), "listDirCallback");
    totalSize(i);
    listEntry(KIO::UDSEntry(), true);
    finished();
}

void KioFtp::copy(const KUrl &src, const KUrl &dest, int permissions, KIO::JobFlags flags)
{
    kDebug() << "copy: " << src.url() << " to " << dest.url();
    connect(m_session, SIGNAL(TransferProgress(qulonglong)), this, SLOT(TransferProgress(qulonglong)));
    connect(m_session, SIGNAL(TransferCompleted()), this, SLOT(TransferCompleted()));
    connect(m_session, SIGNAL(ErrorOccurred(QString,QString)), this, SLOT(ErrorOccurred(QString,QString)));

    if (src.scheme() == "obexftp") {
        ENSURE_SESSION_CREATED(src)
        changeDirectory(src.directory());
        kDebug() << "CopyingRemoteFile....";
        m_session->CopyRemoteFile(src.fileName(), dest.path());
        kDebug() << "Copyied";
    } else if (dest.scheme() == "obexftp") {
        ENSURE_SESSION_CREATED(dest)
        changeDirectory(dest.directory());
        kDebug() << "Sendingfile....";
        m_session->SendFile(src.path());
        kDebug() << "Copyied";
    }

    m_eventLoop.exec();

    finished();
}

void KioFtp::rename(const KUrl& src, const KUrl& dest, KIO::JobFlags flags)
{
    error(KIO::ERR_UNSUPPORTED_ACTION, src.prettyUrl());
    finished();
}


void KioFtp::setHost(const QString &host, quint16 port, const QString &user, const QString &pass)
{
    kDebug() << "setHost: " << host;
    m_statMap.clear();
}

void KioFtp::del(const KUrl& url, bool isfile)
{
    kDebug() << "Del: " << url.url();
    ENSURE_SESSION_CREATED(url)
    changeDirectory(url.directory());
    m_session->DeleteRemoteFile(url.fileName()).waitForFinished();
    finished();
}

void KioFtp::mkdir(const KUrl& url, int permissions)
{
    kDebug() << "MkDir: " << url.url();
    ENSURE_SESSION_CREATED(url)
    changeDirectory(url.directory());
    m_session->CreateFolder(url.fileName()).waitForFinished();
    finished();
}

void KioFtp::slave_status()
{
    kDebug() << "Slave status";
    KIO::SlaveBase::slave_status();
}

void KioFtp::stat(const KUrl &url)
{
    kDebug() << "Stat: " << url.path();
    ENSURE_SESSION_CREATED(url)
    if (url.directory() == "/") {
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, QString::fromLatin1("/"));
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );
        statEntry(entry);

    } else {
        if (m_statMap.contains(url.url())) {
            kDebug() << "statMap contains the url";
            statEntry(m_statMap[url.url()]);

        } else {
            kDebug() << "statMap NOT contains the url";
            changeDirectory(url.directory());

            kDebug() << "RetrieveFolderListing";
            QDBusPendingReply<QString> folder = m_session->RetrieveFolderListing();
            kDebug() << "retireve called";
            folder.waitForFinished();
            kDebug() << "RetrieveError: " << folder.error().message();
            kDebug() << folder.value();

            processXmlEntries(url, folder.value(), "statCallback");
        }
    }

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

        QString fullPath = url.url();
        fullPath.append("/" + attr.value("name").toString());

        KIO::UDSEntry entry;
        if (!m_statMap.contains(fullPath)) {
            kDebug() << "path not cached: " << fullPath;
            entry.insert(KIO::UDSEntry::UDS_NAME, attr.value("name").toString());

            if (m_xml->name() == "folder") {
                entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
                entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );
            } else {
                entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
                entry.insert(KIO::UDSEntry::UDS_SIZE, attr.value("size").toString().toUInt());
                entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, attr.value("modified").toString());
            }

            entry.insert(KIO::UDSEntry::UDS_CREATION_TIME, attr.value("created").toString());

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
    listEntry(entry, false);
}

void KioFtp::statCallback(const KIO::UDSEntry& entry, const KUrl &url)
{
    kDebug() << "FileName : " << url.fileName();
    if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == url.fileName()) {
        kDebug() << "setting statEntry : ";
        statEntry(entry);
    }
}

void KioFtp::sessionCreated(const QDBusObjectPath& path)
{
    kDebug() << "session Created!";
    m_eventLoop.exit();
}

void KioFtp::TransferProgress(qulonglong transfered)
{
    processedSize(transfered);
    kDebug() << "TransferProgress: ";
}

void KioFtp::TransferCompleted()
{
    kDebug() << "TransferCompleted: ";
    m_eventLoop.exit();
}

void KioFtp::ErrorOccurred(const QString &name, const QString &msg)
{
    kDebug() << "ERROR ERROR: " << name;
    kDebug() << "ERROR ERROR: " << msg;
}
