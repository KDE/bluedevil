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

#include "obexftpdaemon.h"
#include "version.h"

#include <QHash>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>

#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>

#include <BluezQt/ObexManager>
#include <BluezQt/InitObexManagerJob>
#include <BluezQt/ObexFileTransfer>
#include <BluezQt/ObexSession>
#include <BluezQt/PendingCall>

K_PLUGIN_FACTORY_WITH_JSON(ObexFtpFactory,
                           "obexftpdaemon.json",
                           registerPlugin<ObexFtpDaemon>();)

struct ObexFtpDaemon::Private
{
    BluezQt::ObexManager *m_manager;
    QHash<QString, QString> m_sessionMap;
    QHash<QString, QList<QDBusMessage> > m_pendingSessions;
    QList<QDBusMessage> m_pendingIsOnlineCalls;
    bool m_initializing;
};

ObexFtpDaemon::ObexFtpDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , d(new Private)
{
    KAboutData aboutData(QStringLiteral("obexftpdaemon"),
                         i18n("ObexFtp Daemon"),
                         bluedevil_version,
                         i18n("ObexFtp Daemon"),
                         KAboutLicense::GPL,
                         i18n("(c) 2010, UFO Coders"));

    aboutData.addAuthor(QStringLiteral("David Rosca"), i18n("Maintainer"),
                        QStringLiteral("nowrep@gmail.com"), QStringLiteral("http://david.rosca.cz"));

    aboutData.addAuthor(QStringLiteral("Alejandro Fiestas Olivares"), i18n("Previous Maintainer"),
                        QStringLiteral("afiestas@kde.org"), QStringLiteral("http://www.afiestas.org"));

    // Initialize BluezQt
    d->m_initializing = true;
    d->m_manager = new BluezQt::ObexManager(this);
    BluezQt::InitObexManagerJob *job = d->m_manager->init();
    job->start();
    connect(job, &BluezQt::InitObexManagerJob::result, this, &ObexFtpDaemon::initJobResult);
}

ObexFtpDaemon::~ObexFtpDaemon()
{
    delete d;
}

bool ObexFtpDaemon::isOnline(const QDBusMessage &msg)
{
    if (d->m_initializing) {
        msg.setDelayedReply(true);
        d->m_pendingIsOnlineCalls.append(msg);
        return false;
    }

    return d->m_manager->isOperational();
}

QString ObexFtpDaemon::session(const QString &address, const QString &target, const QDBusMessage &msg)
{
    if (!d->m_manager->isOperational()) {
        return QString();
    }

    if (d->m_sessionMap.contains(address)) {
        return d->m_sessionMap[address];
    }

    qCDebug(OBEXDAEMON) << "Creating session for" << address;

    // At this point we always want delayed reply
    msg.setDelayedReply(true);

    if (d->m_pendingSessions.contains(address)) {
        d->m_pendingSessions[address].append(msg);
        return QString();
    }

    d->m_pendingSessions.insert(address, QList<QDBusMessage>() << msg);

    QVariantMap args;
    args[QStringLiteral("Target")] = target;

    BluezQt::PendingCall *call = d->m_manager->createSession(address, args);
    call->setUserData(address);
    connect(call, &BluezQt::PendingCall::finished, this, &ObexFtpDaemon::createSessionFinished);

    return QString();
}

bool ObexFtpDaemon::cancelTransfer(const QString &transfer, const QDBusMessage &msg)
{
    // We need this function because kio_obexftp is not owner of the transfer,
    // and thus cannot cancel it.

    msg.setDelayedReply(true);

    QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.bluez.obex"),
                            transfer,
                            QStringLiteral("org.bluez.obex.Transfer1"),
                            QStringLiteral("Cancel"));

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(call));
    watcher->setProperty("ObexFtpDaemon-msg", QVariant::fromValue(msg));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ObexFtpDaemon::cancelTransferFinished);

    return false;
}

void ObexFtpDaemon::initJobResult(BluezQt::InitObexManagerJob *job)
{
    d->m_initializing = false;

    Q_FOREACH (const QDBusMessage &msg, d->m_pendingIsOnlineCalls) {
        QDBusMessage reply = msg.createReply(d->m_manager->isOperational());
        QDBusConnection::sessionBus().send(reply);
    }
    d->m_pendingIsOnlineCalls.clear();

    if (job->error()) {
        qCDebug(OBEXDAEMON) << "Error initializing manager" << job->errorText();
        return;
    }

    connect(d->m_manager, &BluezQt::ObexManager::operationalChanged, this, &ObexFtpDaemon::operationalChanged);
    connect(d->m_manager, &BluezQt::ObexManager::sessionRemoved, this, &ObexFtpDaemon::sessionRemoved);
}

void ObexFtpDaemon::createSessionFinished(BluezQt::PendingCall *call)
{
    QString path;

    if (call->error() == BluezQt::PendingCall::AlreadyExists) {
        // It may happen when kded crashes, or the session was created by different app
        // What to do here? We are not owners of the session...
        qCWarning(OBEXDAEMON) << "Session already exists but it was created by different process!";
    } else if (call->error()) {
        qCWarning(OBEXDAEMON) << "Error creating session" << call->errorText();
    } else {
        path = call->value().value<QDBusObjectPath>().path();
        qCDebug(OBEXDAEMON) << "Created session" << path;
    }

    const QString &address = call->userData().toString();

    // Send reply (empty session path in case of error)
    Q_FOREACH (const QDBusMessage &msg, d->m_pendingSessions[address]) {
        QDBusMessage reply = msg.createReply(path);
        QDBusConnection::sessionBus().send(reply);
    }

    d->m_pendingSessions.remove(address);

    if (!call->error()) {
        d->m_sessionMap.insert(address, path);
    }
}

void ObexFtpDaemon::cancelTransferFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<> &reply = *watcher;
    QDBusMessage msg = watcher->property("ObexFtpDaemon-msg").value<QDBusMessage>();

    bool success = !reply.isError();
    QDBusConnection::sessionBus().send(msg.createReply(QVariant(success)));
}

void ObexFtpDaemon::operationalChanged(bool operational)
{
    qCDebug(OBEXDAEMON) << "Operational changed" << operational;

    if (!operational) {
        d->m_sessionMap.clear();
        d->m_pendingSessions.clear();
    }
}

void ObexFtpDaemon::sessionRemoved(BluezQt::ObexSessionPtr session)
{
    const QString &path = session->objectPath().path();
    const QString &key = d->m_sessionMap.key(path);

    if (!d->m_sessionMap.contains(key)) {
        qCDebug(OBEXDAEMON) << "Removed session not ours" << path;
        return;
    }

    d->m_sessionMap.remove(key);
}

Q_LOGGING_CATEGORY(OBEXDAEMON, "ObexFtpDaemon")

#include "obexftpdaemon.moc"
