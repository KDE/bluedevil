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
#include "kded.h"
#include "globalsettings.h"

#include <kconfiggroup.h>
#include <kcolorscheme.h>
#include <klocalizedstring.h>
#include <kmessagewidget.h>

#include <QLabel>
#include <QAction>
#include <QWidget>
#include <QPainter>
#include <QVBoxLayout>
#include <QPaintEvent>

#include <QBluez/Manager>
#include <QBluez/Adapter>

SystemCheck::SystemCheck(QBluez::Manager *manager, QWidget *parent)
    : QObject(parent)
    , m_parent(parent)
    , m_manager(manager)
    , m_noAdaptersError(0)
    , m_noUsableAdapterError(0)
    , m_notDiscoverableAdapterError(0)
    , m_disabledNotificationsError(0)
{
    m_kded = new KDED(QStringLiteral("org.kde.kded5"),
                      QStringLiteral("/kded"),
                      QDBusConnection::sessionBus());
}

SystemCheck::~SystemCheck()
{
    delete m_kded;
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

    m_noUsableAdapterError = new KMessageWidget(m_parent);
    m_noUsableAdapterError->setMessageType(KMessageWidget::Warning);
    m_noUsableAdapterError->setCloseButtonVisible(false);
    m_noUsableAdapterError->setText(i18n("Your Bluetooth adapter is powered off."));
    QAction *fixNoUsableAdapter = new QAction(m_noUsableAdapterError);
    fixNoUsableAdapter->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));
    fixNoUsableAdapter->setText(i18nc("Action to fix a problem", "Fix it"));
    connect(fixNoUsableAdapter, &QAction::triggered, this, &SystemCheck::fixNoUsableAdapterError);
    m_noUsableAdapterError->addAction(fixNoUsableAdapter);

    layout->insertWidget(1, m_noUsableAdapterError);

    m_notDiscoverableAdapterError = new KMessageWidget(m_parent);
    m_notDiscoverableAdapterError->setMessageType(KMessageWidget::Warning);
    m_notDiscoverableAdapterError->setCloseButtonVisible(false);
    m_notDiscoverableAdapterError->setText(i18n("Your Bluetooth adapter is not visible for remote devices."));
    QAction *fixNotDiscoverableAdapter = new QAction(m_notDiscoverableAdapterError);
    fixNotDiscoverableAdapter->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));
    fixNotDiscoverableAdapter->setText(i18nc("Action to fix a problem", "Fix it"));
    connect(fixNotDiscoverableAdapter, &QAction::triggered, this, &SystemCheck::fixNotDiscoverableAdapterError);
    m_notDiscoverableAdapterError->addAction(fixNotDiscoverableAdapter);

    layout->insertWidget(2, m_notDiscoverableAdapterError);

    m_disabledNotificationsError = new KMessageWidget(m_parent);
    m_disabledNotificationsError->setMessageType(KMessageWidget::Warning);
    m_disabledNotificationsError->setCloseButtonVisible(false);
    m_disabledNotificationsError->setText(i18n("Interaction with Bluetooth system is not optimal."));
    QAction *fixDisabledNotifications = new QAction(m_disabledNotificationsError);
    fixDisabledNotifications->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));
    fixDisabledNotifications->setText(i18nc("Action to fix a problem", "Fix it"));
    connect(fixDisabledNotifications, &QAction::triggered, this, &SystemCheck::fixDisabledNotificationsError);
    m_disabledNotificationsError->addAction(fixDisabledNotifications);

    layout->insertWidget(3, m_disabledNotificationsError);

    m_noKDEDRunning = new KMessageWidget(m_parent);
    m_noKDEDRunning ->setMessageType(KMessageWidget::Warning);
    m_noKDEDRunning->setCloseButtonVisible(false);
    m_noKDEDRunning->setText(i18n("Bluetooth is not completely enabled."));
    QAction *fixNoKDEDRunning = new QAction(m_noKDEDRunning);
    fixNoKDEDRunning->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));
    fixNoKDEDRunning->setText(i18nc("Action to fix a problem", "Fix it"));
    connect(fixNoKDEDRunning, &QAction::triggered, this, &SystemCheck::fixNoKDEDRunning);
    m_noKDEDRunning->addAction(fixNoKDEDRunning);

    layout->insertWidget(4, m_noKDEDRunning);
}

bool SystemCheck::checkKDEDModuleLoaded()
{
    const QStringList res = m_kded->loadedModules();
    bool moduleLoaded = false;
    foreach (const QString &module, res) {
        if (module == QLatin1String("bluedevil")) {
            moduleLoaded = true;
            break;
        }
    }
    return moduleLoaded;
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

KDED *SystemCheck::kded()
{
    return m_kded;
}

void SystemCheck::updateInformationState()
{
    m_noAdaptersError->setEnabled(true);
    m_noAdaptersError->setVisible(false);
    m_noUsableAdapterError->setVisible(false);
    m_notDiscoverableAdapterError->setVisible(false);
    m_disabledNotificationsError->setVisible(false);
    m_noKDEDRunning->setVisible(false);

    if (!GlobalSettings::self()->enableGlobalBluetooth()) {
        m_noAdaptersError->setEnabled(false);
        return;
    }

    if (m_manager->adapters().isEmpty()) {
        m_noAdaptersError->setVisible(true);
        return;
    }

    QBluez::Adapter *usableAdapter = m_manager->usableAdapter();
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
    if (!checkKDEDModuleLoaded()) {
        m_noKDEDRunning->setVisible(true);
        return;
    }
}

void SystemCheck::fixNoKDEDRunning()
{
    m_noKDEDRunning->setVisible(false);
    m_kded->loadModule(QStringLiteral("bluedevil"));
}

void SystemCheck::fixNoUsableAdapterError()
{
    m_noUsableAdapterError->setVisible(false);
    m_manager->adapters().first()->setPowered(true);
}

void SystemCheck::fixNotDiscoverableAdapterError()
{
    m_notDiscoverableAdapterError->setVisible(false);
    m_manager->usableAdapter()->setDiscoverable(true);
    m_manager->usableAdapter()->setDiscoverableTimeout(0);
    // No need to call to updateInformationState, since we are changing this property, it will be
    // triggered automatically.
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

    emit updateInformationStateRequest();
}
