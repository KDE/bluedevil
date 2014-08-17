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
#include "columnresizer.h"
#include "ui_transfer.h"
#include "filereceiversettings.h"
#include "bluedevil_service.h"
#include "sharedfilesdialog/sharedfilesdialog.h"
#include "debug_p.h"

#include <QTimer>
#include <QBoxLayout>

#include <kaboutdata.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kpluginfactory.h>
#include <kconfigdialogmanager.h>
#include <klocalizedstring.h>

#include <QBluez/Manager>
#include <QBluez/InitManagerJob>
#include <QBluez/Adapter>

K_PLUGIN_FACTORY_WITH_JSON(BlueDevilFactory,
                           "bluedeviltransfer.json",
                           registerPlugin<KCMBlueDevilTransfer>();)

////////////////////////////////////////////////////////////////////////////////////////////////////

KCMBlueDevilTransfer::KCMBlueDevilTransfer(QWidget *parent, const QVariantList&)
    : KCModule(parent)
    , m_systemCheck(0)
    , m_restartNeeded(false)
{
    KAboutData *ab = new KAboutData(QStringLiteral("kcmbluedeviltransfer"),
                                    i18n("Bluetooth Transfer"),
                                    QStringLiteral("1.0"),
                                    i18n("Bluetooth Transfer Control Panel Module"),
                                    KAboutLicense::GPL,
                                    i18n("(c) 2010 Rafael Fernández López"));

    ab->addAuthor(i18n("Rafael Fernández López"), i18n("Developer and Maintainer"), QStringLiteral("ereslibre@kde.org"));
    setAboutData(ab);

    connect(this, SIGNAL(changed(bool)), this, SLOT(slotChanged(bool)));

    QVBoxLayout *layout = new QVBoxLayout;
    QWidget *transfer = new QWidget(this);
    m_uiTransfer = new Ui::Transfer();
    m_uiTransfer->setupUi(transfer);
    layout->addWidget(transfer);
    setLayout(layout);

    m_uiTransfer->kcfg_saveUrl->lineEdit()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_uiTransfer->kcfg_autoAccept->addItem(i18nc("'Auto accept' option value", "Never"), QVariant(0));
    m_uiTransfer->kcfg_autoAccept->addItem(i18nc("'Auto accept' option value", "Trusted devices"), QVariant(1));
    m_uiTransfer->kcfg_autoAccept->addItem(i18nc("'Auto accept' option value", "All devices"), QVariant(2));

    m_uiTransfer->kcfg_requirePin->addItem(i18nc("'Require PIN' option value", "Never"), QVariant(false));
    m_uiTransfer->kcfg_requirePin->addItem(i18nc("'Require PIN' option value", "Always"), QVariant(true));

    m_uiTransfer->kcfg_allowWrite->addItem(i18nc("'Permissions' option value", "Read Only"), QVariant(false));
    m_uiTransfer->kcfg_allowWrite->addItem(i18nc("'Permissions' option value", "Modify and Read"), QVariant(true));

    addConfig(FileReceiverSettings::self(), transfer);

    connect(m_uiTransfer->sharedFiles, &QPushButton::clicked, this, &KCMBlueDevilTransfer::showSharedFilesDialog);

    // Initialize QBluez
    m_manager = new QBluez::Manager(this);
    QBluez::InitManagerJob *job = m_manager->init();
    job->start();
    connect(job, &QBluez::InitManagerJob::result, this, &KCMBlueDevilTransfer::initJobResult);
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

void KCMBlueDevilTransfer::initJobResult(QBluez::InitManagerJob *job)
{
    if (job->error()) {
        qCWarning(KCMBLUETOOTH) << "Error initializing manager" << job->errorText();
        return;
    }

    QVBoxLayout *l = static_cast<QVBoxLayout*>(layout());

    m_systemCheck = new SystemCheck(m_manager, this);
    m_systemCheck->createWarnings(l);

    connect(m_systemCheck, &SystemCheck::updateInformationStateRequest,
            this, &KCMBlueDevilTransfer::updateInformationState);

    connect(m_manager, &QBluez::Manager::usableAdapterChanged,
            this, &KCMBlueDevilTransfer::usableAdapterChanged);

    QBluez::Adapter *usableAdapter = m_manager->usableAdapter();
    if (usableAdapter) {
        connect(usableAdapter, &QBluez::Adapter::discoverableChanged,
                this, &KCMBlueDevilTransfer::adapterDiscoverableChanged);
    }

    updateInformationState();

    ColumnResizer *resizer = new ColumnResizer(this);
    resizer->addWidgetsFromFormLayout(m_uiTransfer->formLayout, QFormLayout::LabelRole);
    resizer->addWidgetsFromFormLayout(m_uiTransfer->formLayout_2, QFormLayout::LabelRole);
}

void KCMBlueDevilTransfer::usableAdapterChanged(QBluez::Adapter *adapter)
{
    if (adapter) {
        connect(adapter, &QBluez::Adapter::discoverableChanged,
                this, &KCMBlueDevilTransfer::adapterDiscoverableChanged);
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

void KCMBlueDevilTransfer::showSharedFilesDialog()
{
    SharedFilesDialog *d = new SharedFilesDialog();
    d->exec();
}

void KCMBlueDevilTransfer::slotChanged(bool changed)
{
    m_restartNeeded = changed;
}

#include "bluedeviltransfer.moc"
