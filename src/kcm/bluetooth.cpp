/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "bluetooth.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include <KAboutData>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>

#include <BluezQt/Services>

#include "filereceiversettings.h"

K_PLUGIN_CLASS_WITH_JSON(Bluetooth, "metadata.json")

Bluetooth::Bluetooth(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
{
    KAboutData *about = new KAboutData("kcm_bluetooth", i18n("Bluetooth"), "1.0", QString(), KAboutLicense::GPL);
    about->addAuthor(i18n("Nicolas Fella"), QString(), "nicolas.fella@gmx.de");
    setAboutData(about);
    setButtons(KQuickAddons::ConfigModule::NoAdditionalButton);

    qmlRegisterAnonymousType<QAbstractItemModel>("org.kde.bluedevil.kcm", 1);
    qmlRegisterSingletonInstance("org.kde.bluedevil.kcm", 1, 0, "FileReceiverSettings", FileReceiverSettings::self());
}

void Bluetooth::runWizard()
{
    QProcess::startDetached(QStringLiteral("bluedevil-wizard"), QStringList());
}

void Bluetooth::runSendFile(const QString &ubi)
{
    QProcess::startDetached(QStringLiteral("bluedevil-sendfile"), {QStringLiteral("-u"), ubi});
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

QString Bluetooth::bluetoothStatusAtLogin() const
{
    const auto config = KSharedConfig::openConfig(QStringLiteral("bluedevilglobalrc"));
    const KConfigGroup globalGroup = config->group("Global");
    return globalGroup.readEntry("launchState", "remember");
}

void Bluetooth::setBluetoothStatusAtLogin(const QString newStatus)
{
    auto config = KSharedConfig::openConfig(QStringLiteral("bluedevilglobalrc"));
    KConfigGroup globalGroup = config->group("Global");
    const QString currentValue = (globalGroup.readEntry("launchState", "remember"));

    if (newStatus == currentValue) {
        return;
    }

    if (newStatus == "remember") {
        // Default value
        globalGroup.deleteEntry("launchState");
    } else {
        globalGroup.writeEntry("launchState", newStatus);
    }

    Q_EMIT bluetoothStatusAtLoginChanged(newStatus);
}

#include "bluetooth.moc"
