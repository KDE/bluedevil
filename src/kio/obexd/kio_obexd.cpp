/*  This file is part of the KDE libraries

    Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>
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

#include <KDebug>
#include <KComponentData>
#include <KCmdLineArgs>
#include <KAboutData>

#include <KApplication>
#include <KLocale>
#include <QUrlInfo>

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

    KioObexd *m_q;
};

KioObexd::Private::Private(KioObexd *q)
    : m_q(q)
{
}

KioObexd::Private::~Private()
{
}

KioObexd::KioObexd(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("obexd", pool, app)
    , d(new Private(this))
{
}

KioObexd::~KioObexd()
{
    delete d;
}


void KioObexd::listDir(const KUrl &url)
{
}

void KioObexd::get(const KUrl &url)
{
}

void KioObexd::setHost(const QString &constHostname, quint16 port, const QString &user,
                       const QString &pass)
{
}

void KioObexd::del(const KUrl& url, bool isfile)
{
}

void KioObexd::mkdir(const KUrl& url, int permissions)
{
}

void KioObexd::slave_status()
{
}

void KioObexd::stat(const KUrl &url)
{
}

#include "kio_obexd.moc"