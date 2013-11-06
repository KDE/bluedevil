/*************************************************************************************
 *  Copyright (C) 2013 by Alejandro Fiestas Fiestas <afiestas@kde.org>               *
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

#include "ObexFtpDaemon.h"
#include "createsessionjob.h"

#include "version.h"

#include <QHash>
#include <QDBusConnection>

#include <kdemacros.h>
#include <KDebug>
#include <KAboutData>
#include <KPluginFactory>

#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>

using namespace BlueDevil;
K_PLUGIN_FACTORY(ObexFtpFactory,
                 registerPlugin<ObexFtpDaemon>();)
K_EXPORT_PLUGIN(ObexFtpFactory("obexftpdaemon", "obexftpdaemon"))

struct ObexFtpDaemon::Private
{
    enum Status {
        Online = 0,
        Offline
    } m_status;

    QHash <QString, QString> m_sessionMap;
};

ObexFtpDaemon::ObexFtpDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , d(new Private)
{
    d->m_status = Private::Offline;

    KAboutData aboutData(
        "obexftpdaemon",
        "bluedevil",
        ki18n("ObexFtp Daemon"),
        bluedevil_version,
        ki18n("ObexFtp Daemon"),
        KAboutData::License_GPL,
        ki18n("(c) 2010, UFO Coders")
    );

    aboutData.addAuthor(ki18n("Alejandro Fiestas Olivares"), ki18n("Maintainer"), "afiestas@kde.org",
        "http://www.afiestas.org");

    connect(Manager::self(), SIGNAL(usableAdapterChanged(Adapter*)),
            SLOT(usableAdapterChanged(Adapter*)));

    //WARNING this blocks if org.bluez in system bus is dead
    if (Manager::self()->usableAdapter()) {
        onlineMode();
    }
}

ObexFtpDaemon::~ObexFtpDaemon()
{
    if (d->m_status == Private::Online) {
        offlineMode();
    }
    delete d;
}

void ObexFtpDaemon::onlineMode()
{
    kDebug(dobex());
    if (d->m_status == Private::Online) {
        kDebug(dobex()) << "Already in onlineMode";
        return;
    }

    d->m_status = Private::Online;
}

void ObexFtpDaemon::offlineMode()
{
    kDebug(dobex()) << "Offline mode";
    if (d->m_status == Private::Offline) {
        kDebug(dobex()) << "Already in offlineMode";
        return;
    }

    d->m_status = Private::Offline;
}

void ObexFtpDaemon::usableAdapterChanged(Adapter *adapter)
{
    if (!adapter) {
        offlineMode();
        return;
    }

    onlineMode();
}

QString ObexFtpDaemon::session(QString address, const QDBusMessage& msg)
{
    kDebug(dobex()) << address;
    address.replace("-", ":");

    if(d->m_sessionMap.contains(address)) {
        return d->m_sessionMap[address];
    }
    //TODO Implement the case where the session is being created

    msg.setDelayedReply(true);
    CreateSessionJob *job = new CreateSessionJob(address, msg);
    connect(job, SIGNAL(finished(KJob*)), SLOT(sessionCreated(KJob*)));
    job->start();

    return QString();
}

void ObexFtpDaemon::sessionCreated(KJob* job)
{
    CreateSessionJob* cJob = qobject_cast<CreateSessionJob*>(job);
    kDebug(dobex()) << cJob->path();

    d->m_sessionMap.insert(cJob->address(), cJob->path());
    QDBusMessage msg = cJob->msg().createReply(cJob->path());
    QDBusConnection::sessionBus().asyncCall(msg);
}

extern int dobex() { static int s_area = KDebug::registerArea("ObexDaemon", false); return s_area; }