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

#include "kio_obexd.h"
#include "obexdclient.h"
#include "obexdmanager.h"
#include "obexdsession.h"
#include "obexdfiletransfer.h"
#include "agent.h"

#include <KDebug>
#include <KComponentData>
#include <KCmdLineArgs>
#include <KAboutData>
#include <KLocale>
#include <KApplication>

#define ENSURE_SESSION_CREATED(url) if (d->m_sessionPath.isEmpty()) { \
                                        d->createSession(url);  \
                                    }

extern "C" int KDE_EXPORT kdemain(int argc, char **argv)
{
    KAboutData about("kio_obexd", 0, ki18n("kio_obexd"), 0);
    KCmdLineArgs::init(&about);

    KApplication app;
    if (argc != 4) {
        fprintf(stderr, "Usage: kio_obexd protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    KioObexd slave(argv[2], argv[3]);
    slave.dispatchLoop();
    return 0;
}

class KioObexd::Private
{
public:
    Private(KioObexd *q);
    virtual ~Private();

    void createSession(const QString &address);

    KioObexd *m_q;
    org::openobex::Manager      *m_manager;
    org::openobex::Client       *m_client;
    org::openobex::Session      *m_session;
    org::openobex::FileTransfer *m_fileTransfer;
    Agent                       *m_agent;

    QString m_sessionPath;
};

KioObexd::Private::Private(KioObexd *q)
    : m_q(q)
    , m_manager(new org::openobex::Manager("org.openobex", "/", QDBusConnection::sessionBus(), 0))
    , m_client(new org::openobex::Client("org.openobex.client", "/", QDBusConnection::sessionBus(), 0))
    , m_session(0)
    , m_fileTransfer(0)
    , m_agent(new Agent(0))
{
}

KioObexd::Private::~Private()
{
    delete m_manager;
    delete m_client;
    delete m_session;
    delete m_fileTransfer;
    delete m_agent;
}

void KioObexd::Private::createSession(const QString &address)
{
    QVariantMap device;
    device["Destination"] = address;
    device["Target"] = "ftp";

    m_sessionPath = m_client->CreateSession(device).value().path();
    m_session = new org::openobex::Session("org.openobex.client", m_sessionPath, QDBusConnection::sessionBus(), 0);
    m_session->AssignAgent(QDBusObjectPath("/"));
    m_fileTransfer = new org::openobex::FileTransfer("org.openobex.client", m_sessionPath, QDBusConnection::sessionBus(), 0);
}

KioObexd::KioObexd(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("obexd", pool, app)
    , d(new Private(this))
{
    d->createSession("A8:7E:33:5D:6F:4E");
}

KioObexd::~KioObexd()
{
    delete d;
}

void KioObexd::listDir(const KUrl &url)
{
    KIO::SlaveBase::listDir(url);
}

void KioObexd::copy(const KUrl &src, const KUrl &dest, int permissions, KIO::JobFlags flags)
{
    KIO::SlaveBase::copy(src, dest, permissions, flags);
}

void KioObexd::setHost(const QString &host, quint16 port, const QString &user, const QString &pass)
{
    KIO::SlaveBase::setHost(host, port, user, pass);
}

void KioObexd::del(const KUrl& url, bool isfile)
{
    KIO::SlaveBase::del(url, isfile);
}

void KioObexd::mkdir(const KUrl& url, int permissions)
{
    KIO::SlaveBase::mkdir(url, permissions);
}

void KioObexd::slave_status()
{
    KIO::SlaveBase::slave_status();
}

void KioObexd::stat(const KUrl &url)
{
    KIO::SlaveBase::stat(url);
}
