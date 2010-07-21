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

#define ENSURE_SESSION_CREATED(url) if (d->m_sessionPath.isEmpty()) { \
                                        d->createSession(url);        \
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

    KioFtp *m_q;
    org::openobex::Manager      *m_manager;
    org::openobex::Client       *m_client;
    org::openobex::Session      *m_session;
    QString                      m_sessionPath;
    org::openobex::FileTransfer *m_fileTransfer;
    Agent                       *m_agent;
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
    QVariantMap device;
    device["Destination"] = address.path().replace('-', ':').mid(1, 17);
    device["Target"] = "ftp";

    m_sessionPath = m_client->CreateSession(device).value().path();
    m_session = new org::openobex::Session("org.openobex.client", m_sessionPath, QDBusConnection::sessionBus(), 0);
    m_session->AssignAgent(QDBusObjectPath("/"));
    m_fileTransfer = new org::openobex::FileTransfer("org.openobex.client", m_sessionPath, QDBusConnection::sessionBus(), 0);
}

QString KioFtp::Private::realPath(const KUrl &path) const
{
    return path.path().right(18);
}

KioFtp::KioFtp(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("obexftp", pool, app)
    , d(new Private(this))
{
    qRegisterMetaType<QVariantMapList>("QVariantMapList");
    qDBusRegisterMetaType<QVariantMapList>();
}

KioFtp::~KioFtp()
{
    delete d;
}

void KioFtp::listDir(const KUrl &url)
{
    ENSURE_SESSION_CREATED(url)

    d->m_fileTransfer->ChangeFolder(d->realPath(url));

    int i = 0;
    Q_FOREACH (const QVariantMap &map, d->m_fileTransfer->ListFolder().value()) {
        KIO::UDSEntry entry;
        entry.insert(KIO::UDSEntry::UDS_NAME, map["Name"].toString());
        if (map["Type"].toString() == "folder") {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        } else {
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
        }
        entry.insert(KIO::UDSEntry::UDS_SIZE, map["Size"].toInt());
        entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, map["Modified"].toUInt());
        entry.insert(KIO::UDSEntry::UDS_ACCESS_TIME, map["Accessed"].toUInt());
        entry.insert(KIO::UDSEntry::UDS_CREATION_TIME, map["Created"].toUInt());
        listEntry(entry, false);
        ++i;
    }

    listEntry(KIO::UDSEntry(), true);
    totalSize(i);
    finished();
}

void KioFtp::copy(const KUrl &src, const KUrl &dest, int permissions, KIO::JobFlags flags)
{
    if (src.scheme() == "obexftp") {
        ENSURE_SESSION_CREATED(src)
    } else if (dest.scheme() == "obexftp") {
        ENSURE_SESSION_CREATED(dest)
    }

    KIO::SlaveBase::copy(src, dest, permissions, flags);
}

void KioFtp::setHost(const QString &host, quint16 port, const QString &user, const QString &pass)
{
    KIO::SlaveBase::setHost(host, port, user, pass);
}

void KioFtp::del(const KUrl& url, bool isfile)
{
    ENSURE_SESSION_CREATED(url)

    d->m_fileTransfer->Delete(url.path());

    KIO::SlaveBase::del(url, isfile);
}

void KioFtp::mkdir(const KUrl& url, int permissions)
{
    ENSURE_SESSION_CREATED(url)

    d->m_fileTransfer->CreateFolder(url.path());

    KIO::SlaveBase::mkdir(url, permissions);
}

void KioFtp::slave_status()
{
    KIO::SlaveBase::slave_status();
}

void KioFtp::stat(const KUrl &url)
{
    ENSURE_SESSION_CREATED(url)

    KIO::SlaveBase::stat(url);
}
