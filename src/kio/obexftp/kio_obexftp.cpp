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

#define ENSURE_SESSION_CREATED(url) if (!d->m_session) {       \
                                        d->createSession(url); \
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

class KioFtp::Private
{
public:
    Private(KioFtp *q);
    virtual ~Private();

    void createSession(const KUrl &address);
    void changeDirectory(const KUrl &url);

    KioFtp                      *m_q;
    org::openobex::Manager      *m_manager;
    org::openobex::Session      *m_session;
    QMap<QString, KIO::UDSEntry> m_statMap;
};

KioFtp::Private::Private(KioFtp *q)
    : m_q(q)
    , m_session(0)
{
}

KioFtp::Private::~Private()
{
    delete m_manager;
    delete m_session;
}

void KioFtp::Private::createSession(const KUrl &address)
{
    m_q->infoMessage(i18n("Connecting to the remote device..."));

    m_manager = new org::openobex::Manager("org.openobex", "/org/openobex", QDBusConnection::sessionBus(), 0);
    QDBusPendingReply <QDBusObjectPath > rep = m_manager->CreateBluetoothSession("A8:7E:33:5D:6F:4E", "00:00:00:00:00:00", "ftp");
    rep.waitForFinished();

    kDebug(200000) << "SessionError: " << rep.error().message();
    kDebug(200000) << "SessionPath: " << rep.value().path();

    const QString sessioPath = rep.value().path();
    m_session = new org::openobex::Session("org.openobex", sessioPath, QDBusConnection::sessionBus(), 0);


    QDBusPendingReply <QString > a =  m_session->RetrieveFolderListing();
    a.waitForFinished();
    while(a.error().message() == "Not connected") {
        sleep(1);
        kDebug(200000) << "LOOP";
        a =  m_session->RetrieveFolderListing();
        a.waitForFinished();
    }
    kDebug(200000) << "Private Ctor Ends";
}

void KioFtp::Private::changeDirectory(const KUrl& url)
{
    kDebug(200000) << "ChangeUrl: " << url.path();
    QStringList list = url.path().split("/");

    kDebug(200000) << "List of itens: " << list;

    //Lame?
    if (list.first() == "") {
        list.removeFirst();
    }

    m_session->ChangeCurrentFolderToRoot().waitForFinished();
    kDebug(200000) << "We're in root now";

    Q_FOREACH(const QString &dir, list) {
        if (dir != "A8-7E-33-5D-6F-4E") {
            kDebug(200000) << "Changing to: " << dir;
           QDBusPendingReply <void > a = m_session->ChangeCurrentFolder(dir);
           a.waitForFinished();
           kDebug(200000)  << "Change Error: " << a.error().message();
        } else {
            kDebug(200000) << "Skyping" << dir;
//            QDBusPendingReply <void > a = m_session->ChangeCurrentFolder("/");
//            a.waitForFinished();
//            kDebug(200000)  << "Change Error: " << a.error().message();
        }
    }
}

KioFtp::KioFtp(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("obexftp", pool, app)
    , d(new Private(this))
{
    kDebug(200000) << "INSTANCED";
}

KioFtp::~KioFtp()
{
    delete d;
}

void KioFtp::listDir(const KUrl &url)
{
    kDebug(200000) << "listdir: " << url;
    ENSURE_SESSION_CREATED(url)

    infoMessage(i18n("Retrieving information from remote device..."));

    if (url.directory() != "/") {
        d->changeDirectory(url);
    }

    QDBusPendingReply<QString> folder = d->m_session->RetrieveFolderListing();
    folder.waitForFinished();

    kDebug(200000) << folder.value();

    int i = processXmlEntries(url, folder.value(), "listDirCallback");
    totalSize(i);
    listEntry(KIO::UDSEntry(), true);
    finished();
}

void KioFtp::copy(const KUrl &src, const KUrl &dest, int permissions, KIO::JobFlags flags)
{
    kDebug(200000) << "copy: " << src.url() << " to " << dest.url();
    if (src.scheme() == "obexftp") {
        ENSURE_SESSION_CREATED(src)
        d->changeDirectory(src.directory());
        d->m_session->CopyRemoteFile(src.fileName(), dest.path());
    } else if (dest.scheme() == "obexftp") {
        ENSURE_SESSION_CREATED(dest)
        d->changeDirectory(dest.directory());
        d->m_session->SendFile(src.path());
    }
//     KIO::SlaveBase::copy(src, dest, permissions, flags);
}

void KioFtp::setHost(const QString &host, quint16 port, const QString &user, const QString &pass)
{
    kDebug(200000) << "setHost: " << host;
    d->m_statMap.clear();
//     d->m_path = "/";
    KIO::SlaveBase::setHost("A8-7E-33-5D-6F-4E", port, user, pass);
}

void KioFtp::del(const KUrl& url, bool isfile)
{
    kDebug(200000) << "Del: " << url.url();
    ENSURE_SESSION_CREATED(url)
//     d->m_fileTransfer->Delete(url.path());
}

void KioFtp::mkdir(const KUrl& url, int permissions)
{
    kDebug(200000) << "MkDir: " << url.url();
    ENSURE_SESSION_CREATED(url)
//     d->m_fileTransfer->CreateFolder(url.fileName());
}

void KioFtp::slave_status()
{
    kDebug(200000) << "Slave status";
    KIO::SlaveBase::slave_status();
}

void KioFtp::stat(const KUrl &url)
{
    kDebug(200000) << "Stat: " << url.path();
    ENSURE_SESSION_CREATED(url)
    if (url.directory() == "/") {
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, QString::fromLatin1("/"));
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );
        statEntry(entry);
    } else {
        if (d->m_statMap.contains(url.url())) {
            kDebug(200000) << "statMap contains the url";
            statEntry(d->m_statMap[url.url()]);
        } else {
            kDebug(200000) << "statMap NOT contains the url";
            d->changeDirectory(url.directory());

            kDebug(200000) << "RetrieveFolderListing";
            QDBusPendingReply<QString> folder = d->m_session->RetrieveFolderListing();
            kDebug(200000) << "retireve called";
            folder.waitForFinished();
            kDebug(200000) << "RetrieveError: " << folder.error().message();
            kDebug(200000) << "Wait endds";
            kDebug(200000) << folder.value();

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
            kDebug(200000) << "Skiping dir: " << m_xml->name();
            continue;
        }
        QXmlStreamAttributes attr = m_xml->attributes();
        if (!attr.hasAttribute("name")) {
            continue;
        }

        KIO::UDSEntry entry;
        kDebug(200000) << "Name Yeah baby: " << attr.value("name");
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
        //Access?
        KUrl _url(url);
        QString pathToAdd = _url.url();
        pathToAdd.append(attr.value("name").toString());

        kDebug(200000) << "Adding surl to map: " << _url.url();
        d->m_statMap[pathToAdd] = entry;

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
    kDebug(200000) << "FileName : " << url.fileName();
    if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == url.fileName()) {
        kDebug(200000) << "setting statEntry : ";
        statEntry(entry);
    }
}
