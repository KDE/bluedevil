/*
 * Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
 * Copyright (C) 2010 UFO Coders <info@ufocoders.com>
 * Copyright (C) 2015 David Rosca <nowrep@gmail.com>
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

#include "bluedevilglobal.h"
#include "ui_bluedevilglobal.h"
#include "systemcheck.h"
#include "filereceiversettings.h"
#include "globalsettings.h"

#include <QVBoxLayout>

#include <kaboutdata.h>
#include <klineedit.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <BluezQt/InitManagerJob>

K_PLUGIN_FACTORY_WITH_JSON(BlueDevilFactory,
                           "bluedevilglobal.json",
                           registerPlugin<KCMBlueDevilGlobal>();)

////////////////////////////////////////////////////////////////////////////////////////////////////

KCMBlueDevilGlobal::KCMBlueDevilGlobal(QWidget *parent, const QVariantList&)
    : KCModule(parent)
    , m_systemCheck(Q_NULLPTR)
{
    KAboutData* ab = new KAboutData(QStringLiteral("kcmbluedevilglobal"),
                                    i18n("Bluetooth Advanced Settings"),
                                    QStringLiteral("1.0"),
                                    i18n("Bluetooth Advanced Settings Control Panel Module"),
                                    KAboutLicense::GPL,
                                    i18n("(c) 2010 Rafael Fernández López"));

    ab->addAuthor(QStringLiteral("David Rosca"), i18n("Maintainer"),
                  QStringLiteral("nowrep@gmail.com"), QStringLiteral("http://david.rosca.cz"));

    ab->addAuthor(QStringLiteral("Rafael Fernández López"), i18n("Previous Developer and Maintainer"), QStringLiteral("ereslibre@kde.org"));

    setAboutData(ab);

    m_ui = new Ui::Global();
    m_ui->setupUi(this);

    m_ui->kcfg_autoAccept->addItem(i18nc("'Auto accept' option value", "Never"), QVariant(0));
    m_ui->kcfg_autoAccept->addItem(i18nc("'Auto accept' option value", "Trusted devices"), QVariant(1));
    m_ui->kcfg_autoAccept->addItem(i18nc("'Auto accept' option value", "All devices"), QVariant(2));

    addConfig(FileReceiverSettings::self(), this);
    addConfig(GlobalSettings::self(), this);

    m_isEnabled = m_ui->kcfg_enableGlobalBluetooth->isChecked();

    receiveFilesChanged(m_ui->kcfg_enabled->isChecked());
    enableBluetoothChanged(m_ui->kcfg_enableGlobalBluetooth->isChecked());
    connect(m_ui->kcfg_enabled, &QCheckBox::toggled, this, &KCMBlueDevilGlobal::receiveFilesChanged);
    connect(m_ui->kcfg_enableGlobalBluetooth, &QCheckBox::toggled, this, &KCMBlueDevilGlobal::enableBluetoothChanged);

    // Initialize BluezQt
    m_manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *job = m_manager->init();
    job->start();
    connect(job, &BluezQt::InitManagerJob::result, this, &KCMBlueDevilGlobal::initJobResult);
}

void KCMBlueDevilGlobal::save()
{
    KCModule::save();

    if (!m_isEnabled && m_ui->kcfg_enableGlobalBluetooth->isChecked()) {
        m_systemCheck->kded()->setModuleAutoloading(QStringLiteral("bluedevil"), true);
        m_systemCheck->kded()->loadModule(QStringLiteral("bluedevil"));
    } else if (m_isEnabled && !m_ui->kcfg_enableGlobalBluetooth->isChecked()) {
        m_systemCheck->kded()->setModuleAutoloading(QStringLiteral("bluedevil"), false);
        m_systemCheck->kded()->unloadModule(QStringLiteral("bluedevil"));
    }

    QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded5"),
                                                       QStringLiteral("/modules/bluedevil"),
                                                       QStringLiteral("org.kde.BlueDevil"),
                                                       QStringLiteral("reloadConfig"));
    QDBusConnection::sessionBus().asyncCall(call);

    m_isEnabled = m_ui->kcfg_enableGlobalBluetooth->isChecked();
}

void KCMBlueDevilGlobal::initJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error()) {
        return;
    }

    QVBoxLayout *l = static_cast<QVBoxLayout*>(layout());

    m_systemCheck = new SystemCheck(m_manager, this);
    m_systemCheck->createWarnings(l);
}

void KCMBlueDevilGlobal::receiveFilesChanged(bool enable)
{
    enable = m_ui->kcfg_enableGlobalBluetooth->isChecked() && enable;

    m_ui->lbl_saveFiles->setEnabled(enable);
    m_ui->lbl_autoAccept->setEnabled(enable);
    m_ui->kcfg_saveUrl->setEnabled(enable);
    m_ui->kcfg_autoAccept->setEnabled(enable);
}

void KCMBlueDevilGlobal::enableBluetoothChanged(bool enable)
{
    m_ui->lbl_receivingFiles->setEnabled(enable);
    m_ui->kcfg_enabled->setEnabled(enable);
    receiveFilesChanged(m_ui->kcfg_enabled->isChecked());
}

#include "bluedevilglobal.moc"
