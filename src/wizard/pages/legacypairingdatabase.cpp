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
, m_triedToPair(false)
, m_paired(false)
, m_wizard(parent)
{
    setupUi(this);
    m_working = new KPixmapSequenceOverlayPainter(this);
    m_working->setWidget(working);
    m_working->start();
}

void LegacyPairingPageDatabase::initializePage()
{
    kDebug();
    Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
    connecting->setText(connecting->text().arg(device->name()));

    connect(device, SIGNAL(pairedChanged(bool)), this, SLOT(pairedChanged(bool)));

    device->UUIDs();//Needed to get properties updates
    device->pair("/wizardAgent", Adapter::DisplayYesNo);
}

void LegacyPairingPageDatabase::pairedChanged(bool paired)
{
    kDebug() << paired;

    m_triedToPair = true;
    m_paired = paired;

    emit completeChanged();
    m_wizard->next();
}

bool LegacyPairingPageDatabase::isComplete() const
{
    return m_paired;
}

int LegacyPairingPageDatabase::nextId() const
{
    if (!m_triedToPair) {
        return BlueWizard::Discover;
    }

    if (!m_paired) {
        return BlueWizard::Discover;
    }

    return BlueWizard::Services;
}