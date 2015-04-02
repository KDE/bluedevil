/*
 * Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
 * Copyright (C) 2010 UFO Coders <info@ufocoders.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "systemcheck.h"
#include "globalsettings.h"

#include <kconfiggroup.h>
#include <kcolorscheme.h>
#include <klocalizedstring.h>
#include <kmessagewidget.h>

#include <QLabel>
#include <QAction>
#include <QWidget>
#include <QPainter>
#include <QBoxLayout>
#include <QPaintEvent>

#include <BluezQt/Adapter>

SystemCheck::SystemCheck(BluezQt::Manager *manager, QWidget *parent)
    : QObject(parent)
    , m_parent(parent)
    , m_manager(manager)
    , m_blockedError(0)
    , m_noAdaptersError(0)
    , m_noKdedRunningError(0)
    , m_noUsableAdapterError(0)
    , m_disabledNotificationsError(0)
    , m_notDiscoverableAdapterError(0)
{
    m_kded = new org::kde::kded5(QStringLiteral("org.kde.kded5"), QStringLiteral("/kded"), QDBusConnection::sessionBus(), this);

    connect(manager, &BluezQt::Manager::usableAdapterChanged, this, &SystemCheck::usableAdapterChanged);
    connect(manager, &BluezQt::Manager::bluetoothBlockedChanged, this, &SystemCheck::updateInformationState);
}

org::kde::kded5 *SystemCheck::kded()
{
    return m_kded;
}

void SystemCheck::createWarnings(QVBoxLayout *layout)
{
    if (m_noAdaptersError) {
        return;
    }

    m_noAdaptersError = new KMessageWidget(m_parent);
    m_noAdaptersError->setMessageType(KMessageWidget::Error);
    m_noAdaptersError->setCloseButtonVisible(false);
    m_noAdaptersError->setText(i18n("No Bluetooth adapters have been found."));

    layout->insertWidget(0, m_noAdaptersError);

    m_blockedError = new KMessageWidget(m_parent);
    m_blockedError->setMessageType(KMessageWidget::Error);
    m_blockedError->setCloseButtonVisible(false);
    m_blockedError->setText(i18n("Bluetooth is disabled."));

    QAction *fixBlocked = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")), i18nc("Action to fix a problem", "Fix it"), m_blockedError);
    connect(fixBlocked, SIGNAL(triggered(bool)), this, SLOT(fixBlockedError()));
    m_blockedError->addAction(fixBlocked);

    layout->insertWidget(0, m_blockedError);

    m_noUsableAdapterError = new KMessageWidget(m_parent);
    m_noUsableAdapterError->setMessageType(KMessageWidget::Warning);
    m_noUsableAdapterError->setCloseButtonVisible(false);
    m_noUsableAdapterError->setText(i18n("Your Bluetooth adapter is powered off."));

    QAction *fixNoUsableAdapter = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")), i18nc("Action to fix a problem", "Fix it"), m_noUsableAdapterError);
    connect(fixNoUsableAdapter, SIGNAL(triggered(bool)), this, SLOT(fixNoUsableAdapterError()));
    m_noUsableAdapterError->addAction(fixNoUsableAdapter);

    layout->insertWidget(0, m_noUsableAdapterError);

    m_notDiscoverableAdapterError = new KMessageWidget(m_parent);
    m_notDiscoverableAdapterError->setMessageType(KMessageWidget::Warning);
    m_notDiscoverableAdapterError->setCloseButtonVisible(false);

    QAction *fixNotDiscoverableAdapter = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")), i18nc("Action to fix a problem", "Fix it"), m_notDiscoverableAdapterError);
    connect(fixNotDiscoverableAdapter, SIGNAL(triggered(bool)), this, SLOT(fixNotDiscoverableAdapterError()));
    m_notDiscoverableAdapterError->addAction(fixNotDiscoverableAdapter);
    m_notDiscoverableAdapterError->setText(i18n("Your default Bluetooth adapter is not visible for remote devices."));

    layout->insertWidget(0, m_notDiscoverableAdapterError);

    m_disabledNotificationsError = new KMessageWidget(m_parent);
    m_disabledNotificationsError->setMessageType(KMessageWidget::Warning);
    m_disabledNotificationsError->setCloseButtonVisible(false);

    QAction *fixDisabledNotifications = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")), i18nc("Action to fix a problem", "Fix it"), m_disabledNotificationsError);
    connect(fixDisabledNotifications, SIGNAL(triggered(bool)), this, SLOT(fixDisabledNotificationsError()));
    m_disabledNotificationsError->addAction(fixDisabledNotifications);
    m_disabledNotificationsError->setText(i18n("Interaction with Bluetooth system is not optimal."));

    layout->insertWidget(0, m_disabledNotificationsError);

    m_noKdedRunningError = new KMessageWidget(m_parent);
    m_noKdedRunningError ->setMessageType(KMessageWidget::Warning);
    m_noKdedRunningError->setCloseButtonVisible(false);

    QAction *fixNoKDEDRunning = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")), i18nc("Action to fix a problem", "Fix it"), m_noKdedRunningError);
    connect(fixNoKDEDRunning, SIGNAL(triggered(bool)), this, SLOT(fixNoKDEDRunning()));
    m_noKdedRunningError->addAction(fixNoKDEDRunning);
    m_noKdedRunningError->setText(i18n("Bluetooth is not completely enabled."));

    layout->insertWidget(0, m_noKdedRunningError);

    usableAdapterChanged(m_manager->usableAdapter());
}

void SystemCheck::updateInformationState()
{
    m_blockedError->setVisible(false);
    m_noAdaptersError->setVisible(false);
    m_noUsableAdapterError->setVisible(false);
    m_notDiscoverableAdapterError->setVisible(false);
    m_disabledNotificationsError->setVisible(false);
    m_noKdedRunningError->setVisible(false);

    if (!GlobalSettings::self()->enableGlobalBluetooth()) {
        return;
    }

    if (m_manager->isBluetoothBlocked()) {
        m_blockedError->setVisible(true);
        return;
    }

    if (m_manager->adapters().isEmpty()) {
        m_noAdaptersError->setVisible(true);
        return;
    }

    BluezQt::AdapterPtr usableAdapter = m_manager->usableAdapter();
    if (!usableAdapter) {
        m_noUsableAdapterError->setVisible(true);
        return;
    }

    if (!usableAdapter->isDiscoverable()) {
        m_notDiscoverableAdapterError->setVisible(true);
        return;
    }

    if (!checkNotificationsOK()) {
        m_disabledNotificationsError->setVisible(true);
        return;
    }

    if (!m_kded->loadedModules().value().contains(QStringLiteral("bluedevil"))) {
        m_noKdedRunningError->setVisible(true);
        return;
    }
}

void SystemCheck::usableAdapterChanged(BluezQt::AdapterPtr adapter)
{
    if (adapter) {
        connect(adapter.data(), &BluezQt::Adapter::discoverableChanged, this, &SystemCheck::adapterDiscoverableChanged);
    }

    updateInformationState();
}

void SystemCheck::adapterDiscoverableChanged(bool discoverable)
{
    Q_UNUSED(discoverable)

    updateInformationState();
}

void SystemCheck::fixBlockedError()
{
    m_manager->setBluetoothBlocked(false);
}

void SystemCheck::fixNoKDEDRunning()
{
    m_noKdedRunningError->setVisible(false);
    m_kded->loadModule(QStringLiteral("bluedevil"));

    updateInformationState();
}

void SystemCheck::fixNoUsableAdapterError()
{
    if (m_manager->adapters().isEmpty()) {
        return;
    }

    m_noUsableAdapterError->setVisible(false);
    m_manager->adapters().first()->setPowered(true);
}

void SystemCheck::fixNotDiscoverableAdapterError()
{
    if (!m_manager->usableAdapter()) {
        return;
    }

    m_notDiscoverableAdapterError->setVisible(false);
    m_manager->usableAdapter()->setDiscoverable(true);
    m_manager->usableAdapter()->setDiscoverableTimeout(0);
}

void SystemCheck::fixDisabledNotificationsError()
{
    m_disabledNotificationsError->setVisible(false);

    KConfig config(QStringLiteral("bluedevil.notifyrc"), KConfig::NoGlobals);
    config.addConfigSources(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("bluedevil/bluedevil.notifyrc")));

    QStringList confList = config.groupList();
    QRegExp rx(QStringLiteral("^Event/([^/]*)$"));
    confList = confList.filter(rx);

    Q_FOREACH (const QString &group , confList) {
        KConfigGroup cg(&config, group);
        cg.writeEntry("Action", "Popup");
    }

    config.sync();

    updateInformationState();
}

bool SystemCheck::checkNotificationsOK()
{
    KConfig config(QStringLiteral("bluedevil.notifyrc"), KConfig::NoGlobals);
    config.addConfigSources(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("bluedevil/bluedevil.notifyrc")));

    QStringList confList = config.groupList();
    QRegExp rx(QStringLiteral("^Event/([^/]*)$"));
    confList = confList.filter(rx);

    Q_FOREACH (const QString &group , confList) {
        KConfigGroup cg(&config, group);
        const QString action = cg.readEntry("Action");
        if (!action.contains(QLatin1String("Popup"))) {
            return false;
        }
    }

    return true;
}
