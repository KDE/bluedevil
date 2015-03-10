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

#include "bluedeviltransfer.h"
#include "systemcheck.h"
#include "ui_transfer.h"
#include "filereceiversettings.h"
#include "bluedevil_service.h"

#include <QTimer>
#include <QBoxLayout>

#include <bluedevil/bluedevil.h>

#include <kaboutdata.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kpluginfactory.h>
#include <kconfigdialogmanager.h>
#include <klocalizedstring.h>

K_PLUGIN_FACTORY_WITH_JSON(BlueDevilFactory,
                           "bluedeviltransfer.json",
                           registerPlugin<KCMBlueDevilTransfer>();)

using namespace BlueDevil;

////////////////////////////////////////////////////////////////////////////////////////////////////

KCMBlueDevilTransfer::KCMBlueDevilTransfer(QWidget *parent, const QVariantList&)
    : KCModule(parent)
    , m_systemCheck(new SystemCheck(this))
    , m_restartNeeded(false)
{
    KAboutData* ab = new KAboutData(QStringLiteral("kcmbluedeviltransfer"),
                                    i18n("Bluetooth Transfer"),
                                    QStringLiteral("1.0"),
                                    i18n("Bluetooth Transfer Control Panel Module"),
                                    KAboutLicense::GPL,
                                    i18n("(c) 2010 Rafael Fernández López"));

    ab->addAuthor(i18n("Rafael Fernández López"), i18n("Developer and Maintainer"), QStringLiteral("ereslibre@kde.org"));
    setAboutData(ab);

    connect(m_systemCheck, SIGNAL(updateInformationStateRequest()),
            this, SLOT(updateInformationState()));
    connect(this, SIGNAL(changed(bool)), this, SLOT(changed(bool)));

    QVBoxLayout *layout = new QVBoxLayout;
    m_systemCheck->createWarnings(layout);

    QWidget *transfer = new QWidget(this);
    m_uiTransfer = new Ui::Transfer();
    m_uiTransfer->setupUi(transfer);
    layout->addWidget(transfer);
    setLayout(layout);

    m_uiTransfer->kcfg_saveUrl->lineEdit()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_uiTransfer->kcfg_autoAccept->addItem(i18nc("'Auto accept' option value", "Never"), QVariant(0));
    m_uiTransfer->kcfg_autoAccept->addItem(i18nc("'Auto accept' option value", "Trusted devices"), QVariant(1));
    m_uiTransfer->kcfg_autoAccept->addItem(i18nc("'Auto accept' option value", "All devices"), QVariant(2));

    addConfig(FileReceiverSettings::self(), transfer);

    connect(BlueDevil::Manager::self(), &Manager::usableAdapterChanged, this, &KCMBlueDevilTransfer::usableAdapterChanged);

    BlueDevil::Adapter *const usableAdapter = BlueDevil::Manager::self()->usableAdapter();
    if (usableAdapter) {
        connect(usableAdapter, SIGNAL(discoverableChanged(bool)),
                this, SLOT(adapterDiscoverableChanged()));
    }

    updateInformationState();
}

void KCMBlueDevilTransfer::save()
{
    if (!m_restartNeeded) {
        return;
    }

    KCModule::save();

    org::kde::BlueDevil::Service *service = new org::kde::BlueDevil::Service(
                                                    QStringLiteral("org.kde.BlueDevil.Service"),
                                                    QStringLiteral("/Service"),
                                                    QDBusConnection::sessionBus(), this);
    if (service->isRunning()) {
        service->stopServer();
    }

    service->launchServer();
}

void KCMBlueDevilTransfer::usableAdapterChanged(Adapter *adapter)
{
    if (adapter) {
        connect(adapter, SIGNAL(discoverableChanged(bool)),
                this, SLOT(adapterDiscoverableChanged()));
    }
    QTimer::singleShot(300, this, SLOT(updateInformationState()));
}

void KCMBlueDevilTransfer::adapterDiscoverableChanged()
{
    QTimer::singleShot(300, this, SLOT(updateInformationState()));
}

void KCMBlueDevilTransfer::updateInformationState()
{
    m_systemCheck->updateInformationState();
}

void KCMBlueDevilTransfer::changed(bool changed)
{
    m_restartNeeded = changed;
}

#include "bluedeviltransfer.moc"
