/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "bluetooth.h"

#include <QAbstractItemModel>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QQuickItem>
#include <QQuickRenderControl>

#include <KAboutData>
#include <KConfigGroup>
#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>
#include <KWaylandExtras>
#include <KWindowSystem>

#include <BluezQt/Services>

#include "bluedevilglobalsettings.h"
#include "filereceiversettings.h"

K_PLUGIN_CLASS_WITH_JSON(Bluetooth, "kcm_bluetooth.json")

Bluetooth::Bluetooth(QObject *parent, const KPluginMetaData &data)
    : KQuickConfigModule(parent, data)
{
    setButtons(Help);

    qmlRegisterAnonymousType<QAbstractItemModel>("org.kde.bluedevil.kcm", 1);
    qmlRegisterSingletonInstance("org.kde.bluedevil.kcm", 1, 0, "FileReceiverSettings", FileReceiverSettings::self());
    qmlRegisterSingletonInstance("org.kde.bluedevil.kcm", 1, 0, "GlobalSettings", GlobalSettings::self());
}

void Bluetooth::runWizard(QQuickItem *context)
{
    auto runIt = [this](const QString &windowHandle) {
        auto *job = new KIO::CommandLauncherJob(QStringLiteral("bluedevil-wizard"), {QStringLiteral("--parentWindow"), windowHandle});

        connect(job, &KJob::finished, this, [this](KJob *job) {
            if (job->error()) {
                Q_EMIT errorOccured(job->errorString());
            }
        });
        job->start();
    };

    QWindow *actualWindow = QQuickRenderControl::renderWindowFor(context->window());

    if (KWindowSystem::isPlatformWayland()) {
        KWaylandExtras::exportWindow(actualWindow);
        connect(
            KWaylandExtras::self(),
            &KWaylandExtras::windowExported,
            this,
            [runIt](QWindow *, const QString &windowHandle) {
                runIt(windowHandle);
            },
            Qt::SingleShotConnection);
    } else {
        runIt(QStringLiteral("0x%1").arg((unsigned int)actualWindow->winId(), 0, 16));
    }
}

void Bluetooth::runSendFile(const QString &ubi)
{
    auto *job = new KIO::CommandLauncherJob(QStringLiteral("bluedevil-sendfile"), {QStringLiteral("-u"), ubi});
    job->setDesktopName(QStringLiteral("org.kde.bluedevilsendfile"));
    connect(job, &KJob::finished, this, [this](KJob *job) {
        if (job->error()) {
            Q_EMIT errorOccured(job->errorString());
        }
    });
    job->start();
}

void Bluetooth::checkNetworkConnection(const QStringList &uuids, const QString &address)
{
    if (uuids.contains(BluezQt::Services::Nap)) {
        checkNetworkInternal(QStringLiteral("nap"), address);
    }

    if (uuids.contains(BluezQt::Services::DialupNetworking)) {
        checkNetworkInternal(QStringLiteral("dun"), address);
    }
}

void Bluetooth::checkNetworkInternal(const QString &service, const QString &address)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmanetworkmanagement"),
                                                      QStringLiteral("/org/kde/plasmanetworkmanagement"),
                                                      QStringLiteral("org.kde.plasmanetworkmanagement"),
                                                      QStringLiteral("bluetoothConnectionExists"));

    msg << address;
    msg << service;

    QDBusPendingCallWatcher *call = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg));
    connect(call, &QDBusPendingCallWatcher::finished, this, [this, service, call]() {
        QDBusPendingReply<bool> reply = *call;
        if (reply.isError()) {
            return;
        }

        Q_EMIT networkAvailable(service, reply.value());
    });
}

void Bluetooth::setupNetworkConnection(const QString &service, const QString &address, const QString &deviceName)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmanetworkmanagement"),
                                                      QStringLiteral("/org/kde/plasmanetworkmanagement"),
                                                      QStringLiteral("org.kde.plasmanetworkmanagement"),
                                                      QStringLiteral("addBluetoothConnection"));

    msg << address;
    msg << service;
    msg << i18nc("DeviceName Network (Service)", "%1 Network (%2)", deviceName, service);

    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
}

QString Bluetooth::receiveFolderPath() const
{
    return FileReceiverSettings::saveUrl().toLocalFile();
}

void Bluetooth::setReceiveFolderPath(const QString &path)
{
    const QUrl url = QUrl::fromUserInput(path, QString(), QUrl::AssumeLocalFile);
    FileReceiverSettings::setSaveUrl(url);
    Q_EMIT receiveFolderPathChanged();
}

#include "bluetooth.moc"

#include "moc_bluetooth.cpp"
