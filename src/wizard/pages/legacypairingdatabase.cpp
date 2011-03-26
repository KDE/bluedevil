/*****************************************************************************
 * This file is part of the KDE project                                      *
 *                                                                           *
 * Copyright (C) 2010 Alejandro Fiestas Olivares <afiestas@kde.org>          *
 * Copyright (C) 2010-2011 UFO Coders <info@ufocoders.com>                   *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "legacypairingdatabase.h"
#include "bluewizard.h"
#include "../wizardagent.h"

#include <KDebug>
#include <kpixmapsequenceoverlaypainter.h>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

LegacyPairingPageDatabase::LegacyPairingPageDatabase(BlueWizard* parent) : QWizardPage(parent)
, m_wizard(parent)
{
    setupUi(this);
    m_working = new KPixmapSequenceOverlayPainter(this);
    m_working->setWidget(working);
    m_working->start();
}

void LegacyPairingPageDatabase::initializePage()
{
    m_wizard->setButtonLayout(wizardButtonsLayout());

    Device *device = deviceFromWizard();
    connecting->setText(connecting->text().arg(device->name()));

    connect(device, SIGNAL(registered(Device*)), this, SLOT(registered(Device*)));

    QMetaObject::invokeMethod(device, "registerDeviceAsync", Qt::QueuedConnection);
}

void LegacyPairingPageDatabase::registered(Device *device)
{
    connect(device, SIGNAL(pairedChanged(bool)), this, SLOT(pairedChanged(bool)));
    device->pair("/wizardAgent", Adapter::DisplayYesNo);
}

void LegacyPairingPageDatabase::pairedChanged(bool paired)
{
    kDebug() << paired;
    m_wizard->next();
}

bool LegacyPairingPageDatabase::validatePage()
{
    return deviceFromWizard()->isPaired();
}

int LegacyPairingPageDatabase::nextId() const
{
    return BlueWizard::Services;
}

Device* LegacyPairingPageDatabase::deviceFromWizard()
{
    return Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
}

QList< QWizard::WizardButton > LegacyPairingPageDatabase::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;

    return list;
}