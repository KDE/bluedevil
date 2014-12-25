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

#include <bluedevil/bluedevil.h>

#include <kglobal.h>
#include <kaction.h>
#include <kdeversion.h>
#include <kpushbutton.h>
#include <kconfiggroup.h>
#include <kcolorscheme.h>
#include <kstandarddirs.h>
#include <klocalizedstring.h>
#include <kmessagewidget.h>

#include <QtGui/QLabel>
#include <QtGui/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QBoxLayout>
#include <QtGui/QPaintEvent>

SystemCheck::SystemCheck(QWidget *parent)
    : QObject(parent)
    , m_kded(new KDED("org.kde.kded", "/kded", QDBusConnection::sessionBus()))
    , m_parent(parent)
    , m_noAdaptersError(0)
    , m_noUsableAdapterError(0)
    , m_notDiscoverableAdapterError(0)
    , m_disabledNotificationsError(0)
{
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
    layout->addWidget(m_noAdaptersError);

    m_noUsableAdapterError = new KMessageWidget(m_parent);
    m_noUsableAdapterError->setMessageType(KMessageWidget::Warning);
    m_noUsableAdapterError->setCloseButtonVisible(false);
    m_noUsableAdapterError->setText(i18n("Your Bluetooth adapter is powered off."));

    KAction *fixNoUsableAdapter = new KAction(KIcon("dialog-ok-apply"), i18nc("Action to fix a problem", "Fix it"), m_noUsableAdapterError);
    connect(fixNoUsableAdapter, SIGNAL(triggered(bool)), this, SLOT(fixNoUsableAdapterError()));
    m_noUsableAdapterError->addAction(fixNoUsableAdapter);
    layout->addWidget(m_noUsableAdapterError);

    m_notDiscoverableAdapterError = new KMessageWidget(m_parent);
    m_notDiscoverableAdapterError->setMessageType(KMessageWidget::Warning);
    m_notDiscoverableAdapterError->setCloseButtonVisible(false);

    KAction *fixNotDiscoverableAdapter = new KAction(KIcon("dialog-ok-apply"), i18nc("Action to fix a problem", "Fix it"), m_notDiscoverableAdapterError);
    connect(fixNotDiscoverableAdapter, SIGNAL(triggered(bool)), this, SLOT(fixNotDiscoverableAdapterError()));
    m_notDiscoverableAdapterError->addAction(fixNotDiscoverableAdapter);
    m_notDiscoverableAdapterError->setText(i18n("Your default Bluetooth adapter is not visible for remote devices."));

    layout->addWidget(m_notDiscoverableAdapterError);

    m_disabledNotificationsError = new KMessageWidget(m_parent);
    m_disabledNotificationsError->setMessageType(KMessageWidget::Warning);
    m_disabledNotificationsError->setCloseButtonVisible(false);

    KAction *fixDisabledNotifications = new KAction(KIcon("dialog-ok-apply"), i18nc("Action to fix a problem", "Fix it"), m_disabledNotificationsError);
    connect(fixDisabledNotifications, SIGNAL(triggered(bool)), this, SLOT(fixDisabledNotificationsError()));
    m_disabledNotificationsError->addAction(fixDisabledNotifications);
    m_disabledNotificationsError->setText(i18n("Interaction with Bluetooth system is not optimal."));

    layout->addWidget(m_disabledNotificationsError);

    m_noKDEDRunning = new KMessageWidget(m_parent);
    m_noKDEDRunning ->setMessageType(KMessageWidget::Warning);
    m_noKDEDRunning->setCloseButtonVisible(false);

    KAction *fixNoKDEDRunning = new KAction(KIcon("dialog-ok-apply"), i18nc("Action to fix a problem", "Fix it"), m_noKDEDRunning);
    connect(fixNoKDEDRunning, SIGNAL(triggered(bool)), this, SLOT(fixNoKDEDRunning()));
    m_noKDEDRunning->addAction(fixNoKDEDRunning);
    m_noKDEDRunning->setText(i18n("Bluetooth is not completely enabled."));

    layout->addWidget(m_noKDEDRunning);
}

bool SystemCheck::checkKDEDModuleLoaded()
{
    const QStringList res = m_kded->loadedModules();
    bool moduleLoaded = false;
    foreach (const QString &module, res) {
        if (module == "bluedevil") {
            moduleLoaded = true;
            break;
        }
    }
    return moduleLoaded;
}

bool SystemCheck::checkNotificationsOK()
{
    KConfig config("bluedevil.notifyrc", KConfig::NoGlobals);
    config.addConfigSources(KGlobal::dirs()->findAllResources("data", "bluedevil/bluedevil.notifyrc"));

    QStringList confList = config.groupList();
    QRegExp rx("^Event/([^/]*)$");
    confList = confList.filter(rx);

    Q_FOREACH (const QString &group , confList) {
        KConfigGroup cg(&config, group);
        const QString action = cg.readEntry("Action");
        if (!action.contains("Popup")) {
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

    if (BlueDevil::Manager::self()->adapters().isEmpty()) {
        m_noAdaptersError->setVisible(true);
        return;
    }

    BlueDevil::Adapter *const usableAdapter = BlueDevil::Manager::self()->usableAdapter();
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
    m_kded->loadModule("bluedevil");
}

void SystemCheck::fixNoUsableAdapterError()
{
    m_noUsableAdapterError->setVisible(false);
    BlueDevil::Manager::self()->adapters().first()->setPowered(true);
}

void SystemCheck::fixNotDiscoverableAdapterError()
{
    m_notDiscoverableAdapterError->setVisible(false);
    BlueDevil::Manager::self()->usableAdapter()->setDiscoverable(true);
    BlueDevil::Manager::self()->usableAdapter()->setDiscoverableTimeout(0);
    // No need to call to updateInformationState, since we are changing this property, it will be
    // triggered automatically.
}

void SystemCheck::fixDisabledNotificationsError()
{
    m_disabledNotificationsError->setVisible(false);

    KConfig config("bluedevil.notifyrc", KConfig::NoGlobals);
    config.addConfigSources(KGlobal::dirs()->findAllResources("data", "bluedevil/bluedevil.notifyrc"));

    QStringList confList = config.groupList();
    QRegExp rx("^Event/([^/]*)$");
    confList = confList.filter(rx);

    Q_FOREACH (const QString &group , confList) {
        KConfigGroup cg(&config, group);
        cg.writeEntry("Action", "Popup");
    }

    config.sync();

    emit updateInformationStateRequest();
}
