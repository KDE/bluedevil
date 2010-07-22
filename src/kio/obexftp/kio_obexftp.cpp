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
#include "obexftpclient.h"
#include "obexftpmanager.h"
#include "obexftpsession.h"
#include "obexftpfiletransfer.h"
#include "agent.h"

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
    QString realPath(const KUrl &path) const;

    KioFtp                      *m_q;
    org::openobex::Manager      *m_manager;
    org::openobex::Client       *m_client;
    org::openobex::Session      *m_session;
    org::openobex::FileTransfer *m_fileTransfer;
    Agent                       *m_agent;
    QString                      m_path;
    QString                      m_sessionPath;
    QMap<QString, KIO::UDSEntry> m_statMap;
};

KioFtp::Private::Private(KioFtp *q)
    : m_q(q)
    , m_manager(new org::openobex::Manager("org.openobex", "/", QDBusConnection::sessionBus(), 0))
    , m_client(new org::openobex::Client("org.openobex.client", "/", QDBusConnection::sessionBus(), 0))
    , m_session(0)
    , m_fileTransfer(0)
    , m_agent(new Agent(0))
{
}

KioFtp::Private::~Private()
{
    delete m_manager;
    delete m_client;
    delete m_session;
    delete m_fileTransfer;
    delete m_agent;
}

void KioFtp::Private::createSession(const KUrl &address)
{
    m_q->infoMessage(i18n("Connecting to the remote device..."));

    QVariantMap device;
    kDebug(200000) << "Raw address: " << address.path();
    device["Destination"] = address.path().replace('-', ':').mid(1, 17);
    device["Target"] = "ftp";

    kDebug(200000) << "Connect to: " << device["Destination"].toString();

    m_sessionPath = m_client->CreateSession(device).value().path();
    kDebug(200000) << "Session path: " << m_sessionPath;
    m_session = new org::openobex::Session("org.openobex.client", m_sessionPath, QDBusConnection::sessionBus(), 0);
    m_session->AssignAgent(QDBusObjectPath("/"));
    m_fileTransfer = new org::openobex::FileTransfer("org.openobex.client", m_sessionPath, QDBusConnection::sessionBus(), 0);
    m_path = "/";
    kDebug(200000) << "Private Ctor Ends";
}

QString KioFtp::Private::realPath(const KUrl &path) const
{
    QString res = path.path().mid(19);
    if (res.isEmpty()) {
        res = "/";
    }
    return res;
}

KioFtp::KioFtp(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("obexftp", pool, app)
    , d(new Private(this))
{
    kDebug(200000) << "INSTANCED";
    qRegisterMetaType<QVariantMapList>("QVariantMapList");
    qDBusRegisterMetaType<QVariantMapList>();
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

    const QString path = d->realPath(url);
    kDebug(200000) << "realPath : " << path;
    if (path != "/") {
        QStringList parts = path.split("/");
        Q_FOREACH(const QString &part, parts) {
            qDebug() << "Changing to: " << part;
            QDBusPendingReply <void> rep = d->m_fileTransfer->ChangeFolder(part);
            rep.waitForFinished();
        }
        kDebug(200000) << "waited";
    }

    QDBusPendingReply<QVariantMapList> folder = d->m_fileTransfer->ListFolder();
    folder.waitForFinished();

    int i = 0;
    if (folder.error().type() == QDBusError::NoError) {
        Q_FOREACH (const QVariantMap &map, folder.value()) {
            KIO::UDSEntry entry;
            kDebug(200000) << "Name Yeah baby: " << map["Name"].toString();
            entry.insert(KIO::UDSEntry::UDS_NAME, map["Name"].toString());
            entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, map["Name"].toString());

            if (map["Type"].toString() == "folder") {
                entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
            } else {
                entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
            }

            entry.insert(KIO::UDSEntry::UDS_SIZE, map["Size"].toInt());
            entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, map["Modified"].toUInt());
            entry.insert(KIO::UDSEntry::UDS_ACCESS_TIME, map["Accessed"].toUInt());
            entry.insert(KIO::UDSEntry::UDS_CREATION_TIME, map["Created"].toUInt());
            entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );
            KUrl _url(url);
            _url.addPath(map["Name"].toString());
            d->m_statMap[_url.url()] = entry;
            listEntry(entry, false);
            ++i;
        }
    }

    delete d->m_fileTransfer;
    
    d->m_fileTransfer = m_fileTransfer = new org::openobex::FileTransfer("org.openobex.client", m_sessionPath, QDBusConnection::sessionBus(), 0);
    listEntry(KIO::UDSEntry(), true);
    totalSize(i);
    finished();
}

void KioFtp::copy(const KUrl &src, const KUrl &dest, int permissions, KIO::JobFlags flags)
{
    kDebug(200000) << "copy: " << src.url() << " to " << dest.url();
    if (src.scheme() == "obexftp") {
        ENSURE_SESSION_CREATED(src)
    } else if (dest.scheme() == "obexftp") {
        ENSURE_SESSION_CREATED(dest)
    }
    KIO::SlaveBase::copy(src, dest, permissions, flags);
}

void KioFtp::setHost(const QString &host, quint16 port, const QString &user, const QString &pass)
{
    kDebug(200000) << "setHost: " << host;
    d->m_statMap.clear();
    d->m_path = "/";
    KIO::SlaveBase::setHost("A8-7E-33-5D-6F-4E", port, user, pass);
}

void KioFtp::del(const KUrl& url, bool isfile)
{
    kDebug(200000) << "Del: " << url.url();
    ENSURE_SESSION_CREATED(url)
    d->m_fileTransfer->Delete(url.path());
}

void KioFtp::mkdir(const KUrl& url, int permissions)
{
    kDebug(200000) << "MkDir: " << url.url();
    ENSURE_SESSION_CREATED(url)
    d->m_fileTransfer->CreateFolder(url.fileName());
}

void KioFtp::slave_status()
{
    kDebug(200000) << "Slave status";
    KIO::SlaveBase::slave_status();
}

void KioFtp::stat(const KUrl &url)
{
    kDebug(200000) << "Stat: " << url.directory();
    ENSURE_SESSION_CREATED(url)
    if (url.directory() == "/") {
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, QString::fromLatin1("/"));
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1( "inode/directory" ) );
        statEntry(entry);
    } else {
        statEntry(d->m_statMap[url.url()]);
    }

    finished();
}
