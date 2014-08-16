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
#include "version.h"

#include <QHash>
#include <QDBusMessage>
#include <QDBusConnection>

#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>

#include <QBluez/ObexManager>
#include <QBluez/InitObexManagerJob>
#include <QBluez/ObexFileTransfer>
#include <QBluez/PendingCall>

K_PLUGIN_FACTORY_WITH_JSON(ObexFtpFactory,
                           "obexftpdaemon.json",
                           registerPlugin<ObexFtpDaemon>();)

struct ObexFtpDaemon::Private
{
    QBluez::ObexManager *m_manager;
    QHash<QString, QString> m_sessionMap;
    QHash<QString, QString> m_reverseSessionMap;
    QHash<QString, QList<QDBusMessage> > m_wipSessions;
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

    aboutData.addAuthor(i18n("Alejandro Fiestas Olivares"), i18n("Maintainer"),
                        QStringLiteral("afiestas@kde.org"), QStringLiteral("http://www.afiestas.org"));

    // Initialize QBluez
    d->m_initializing = true;
    d->m_manager = new QBluez::ObexManager(this);
    QBluez::InitObexManagerJob *job = d->m_manager->init();
    job->start();
    connect(job, &QBluez::InitObexManagerJob::result, this, &ObexFtpDaemon::initJobResult);
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

QString ObexFtpDaemon::session(QString address, const QDBusMessage &msg)
{
    if (!d->m_manager->isOperational()) {
        return QString();
    }

    address.replace(QLatin1Char('-'), QLatin1Char(':'));
    address = address.toUpper();

    if (d->m_sessionMap.contains(address)) {
        return d->m_sessionMap[address];
    }

    qCDebug(OBEXDAEMON) << "Creating session for" << address;

    // At this point we always want delayed reply
    msg.setDelayedReply(true);

    if (d->m_wipSessions.contains(address)) {
        d->m_wipSessions[address].append(msg);
        return QString();
    }

    d->m_wipSessions.insert(address, QList<QDBusMessage>() << msg);

    QVariantMap args;
    args[QStringLiteral("Target")] = QStringLiteral("ftp");

    QBluez::PendingCall *call = d->m_manager->createSession(address, args);
    call->setUserData(address);
    connect(call, &QBluez::PendingCall::finished, this, &ObexFtpDaemon::createSessionFinished);

    return QString();
}

bool ObexFtpDaemon::cancelTransfer(const QDBusObjectPath &transfer, const QDBusMessage &msg)
{
    // We need this function because kio_obexftp is not owner of the transfer,
    // and thus cannot cancel it.

    msg.setDelayedReply(true);

    QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.bluez.obex"),
                            transfer.path(),
                            QStringLiteral("org.bluez.obex.Transfer1"),
                            QStringLiteral("Cancel"));

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(call));
    watcher->setProperty("ObexFtpDaemon-msg", QVariant::fromValue(msg));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ObexFtpDaemon::cancelTransferFinished);

    return false;
}

void ObexFtpDaemon::initJobResult(QBluez::InitObexManagerJob *job)
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

    connect(d->m_manager, &QBluez::ObexManager::operationalChanged, this, &ObexFtpDaemon::operationalChanged);
    connect(d->m_manager, &QBluez::ObexManager::sessionRemoved, this, &ObexFtpDaemon::sessionRemoved);
}

void ObexFtpDaemon::createSessionFinished(QBluez::PendingCall *call)
{
    QString path;

    if (call->error() == QBluez::PendingCall::AlreadyExists) {
        // It may happen when kded crashes, or the session was created by different app
        // What to do here? We are not owners of the session...
        qCDebug(OBEXDAEMON) << "Session already exists but it was created by different process!";
    } else if (call->error()) {
        qCDebug(OBEXDAEMON) << "Error creating session" << call->errorText();
    } else {
        path = call->value().value<QDBusObjectPath>().path();
        qCDebug(OBEXDAEMON) << "Created session" << path;
    }

    const QString &address = call->userData().toString();

    // Send reply (empty session path in case of error)
    Q_FOREACH (const QDBusMessage &msg, d->m_wipSessions[address]) {
        QDBusMessage reply = msg.createReply(path);
        QDBusConnection::sessionBus().send(reply);
    }

    d->m_wipSessions.remove(address);

    if (!call->error()) {
        d->m_sessionMap.insert(address, path);
        d->m_reverseSessionMap.insert(path, address);
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
        d->m_reverseSessionMap.clear();
        d->m_wipSessions.clear();
    }
}

void ObexFtpDaemon::sessionRemoved(const QDBusObjectPath &session)
{
    const QString &path = session.path();
    if (!d->m_reverseSessionMap.contains(path)) {
        qCDebug(OBEXDAEMON) << "Removed session not ours" << d->m_reverseSessionMap;
        return;
    }

    const QString &address = d->m_reverseSessionMap.take(path);
    qCDebug(OBEXDAEMON) << address;
    qCDebug(OBEXDAEMON) << d->m_sessionMap.remove(address);
}

Q_LOGGING_CATEGORY(OBEXDAEMON, "ObexDaemon")

#include "ObexFtpDaemon.moc"
