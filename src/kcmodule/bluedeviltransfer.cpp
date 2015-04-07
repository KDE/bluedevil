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

#include <QVBoxLayout>

#include <kaboutdata.h>
#include <klineedit.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <BluezQt/InitManagerJob>

K_PLUGIN_FACTORY_WITH_JSON(BlueDevilFactory,
                           "bluedeviltransfer.json",
                           registerPlugin<KCMBlueDevilTransfer>();)

////////////////////////////////////////////////////////////////////////////////////////////////////

KCMBlueDevilTransfer::KCMBlueDevilTransfer(QWidget *parent, const QVariantList&)
    : KCModule(parent)
    , m_systemCheck(0)
{
    KAboutData* ab = new KAboutData(QStringLiteral("kcmbluedeviltransfer"),
                                    i18n("Bluetooth Transfer"),
                                    QStringLiteral("1.0"),
                                    i18n("Bluetooth Transfer Control Panel Module"),
                                    KAboutLicense::GPL,
                                    i18n("(c) 2010 Rafael Fernández López"));

    ab->addAuthor(QStringLiteral("David Rosca"), i18n("Maintainer"),
                  QStringLiteral("nowrep@gmail.com"), QStringLiteral("http://david.rosca.cz"));

    ab->addAuthor(QStringLiteral("Rafael Fernández López"), i18n("Previous Developer and Maintainer"), QStringLiteral("ereslibre@kde.org"));

    setAboutData(ab);

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

    addConfig(FileReceiverSettings::self(), transfer);

    // Initialize BluezQt
    m_manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *job = m_manager->init();
    job->start();
    connect(job, &BluezQt::InitManagerJob::result, this, &KCMBlueDevilTransfer::initJobResult);
}

void KCMBlueDevilTransfer::initJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error()) {
        return;
    }

    QVBoxLayout *l = static_cast<QVBoxLayout*>(layout());

    m_systemCheck = new SystemCheck(m_manager, this);
    m_systemCheck->createWarnings(l);
}

#include "bluedeviltransfer.moc"
