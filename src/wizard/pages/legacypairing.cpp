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


#include "legacypairing.h"
#include "bluewizard.h"

#include <KDebug>
#include <kpixmapsequenceoverlaypainter.h>

#include <bluedevil/bluedevil.h>
#include <QTimer>
#include <wizardagent.h>

using namespace BlueDevil;

LegacyPairingPage::LegacyPairingPage(BlueWizard* parent) : QWizardPage(parent)
, m_triedToPair(false)
, m_paired(false)
, m_wizard(parent)
{
    setupUi(this);
    m_working = new KPixmapSequenceOverlayPainter(this);
    m_working->setWidget(pinNumber);
    m_working->start();

    QFont font(pinNumber->font());
    font.setPointSize(42);
    font.setBold(true);
    pinNumber->setFont(font);
}

void LegacyPairingPage::initializePage()
{
    kDebug();
    Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());

    kDebug() << device;
    connect(device, SIGNAL(pairedChanged(bool)), this, SLOT(pairedChanged(bool)));
    connect(m_wizard->agent(), SIGNAL(pinRequested(QString)), this, SLOT(pinRequested(QString)));

    device->UUIDs();//Needed to get properties updates
    device->pair("/wizardAgent", Adapter::DisplayYesNo);
}

void LegacyPairingPage::pinRequested(const QString& pin)
{
    m_working->stop();
    pinNumber->setText(pin);
}

void LegacyPairingPage::pairedChanged(bool paired)
{
    kDebug() << paired;
    m_triedToPair = true;
    m_paired = paired;

    emit completeChanged();
    m_wizard->next();
}

bool LegacyPairingPage::isComplete() const
{
    return m_paired;
}

int LegacyPairingPage::nextId() const
{
    if (!m_triedToPair) {
        return BlueWizard::Discover;
    }

    if (!m_paired) {
        return BlueWizard::Discover;
    }

    return BlueWizard::Services;
}
