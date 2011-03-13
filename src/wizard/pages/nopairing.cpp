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


#include "nopairing.h"
#include "bluewizard.h"

#include <KDebug>
#include <kpixmapsequenceoverlaypainter.h>

#include <bluedevil/bluedevil.h>
#include <QTimer>

using namespace BlueDevil;

NoPairing::NoPairing(BlueWizard* parent) : QWizardPage(parent)
, m_connected(false)
, m_wizard(parent)
{
    setupUi(this);
    m_working = new KPixmapSequenceOverlayPainter(this);
    m_working->setWidget(working);
    m_working->start();
}

void NoPairing::initializePage()
{
    kDebug();
    Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
//     connecting->setText(connecting->text().arg(device->name()));

    connect(device, SIGNAL(registerDeviceResult(Device*,bool)), this, SLOT(registerDeviceResult(Device*,bool)));

    qDebug() << thread();
    BlueDevil::asyncCall(device, SLOT(registerDevice()));
    kDebug();
}

void NoPairing::registerDeviceResult(Device* device, bool result)
{
    kDebug() << result;
    m_connected = result;
    //TODO: Handle errors

    if (!m_connected) {
        kDebug() << "Error";
    }
    emit completeChanged();
}

bool NoPairing::isComplete() const
{
    return m_connected;
}

int NoPairing::nextId() const
{
    if (!m_connected) {
        return BlueWizard::Discover;
    }

    Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());

    kDebug() << device->UUIDs();
    return BlueWizard::Services;

    return BlueWizard::Discover;
}
