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

NoPairingPage::NoPairingPage(BlueWizard* parent) : QWizardPage(parent)
, m_wizard(parent)
{
    setupUi(this);
    m_working = new KPixmapSequenceOverlayPainter(this);
    m_working->setWidget(working);
    m_working->start();
}

void NoPairingPage::initializePage()
{
    kDebug();
    m_wizard->setButtonLayout(wizardButtonsLayout());

    Device *device = deviceFromWizard();
    connecting->setText(connecting->text().arg(device->name()));

    connect(device, SIGNAL(registered(Device*)), this, SLOT(registerDeviceResult(Device*)));

    QMetaObject::invokeMethod(device, "registerDeviceAsync", Qt::QueuedConnection);
}

void NoPairingPage::registerDeviceResult(Device* device)
{
    Q_UNUSED(device);
    m_wizard->next();
}

bool NoPairingPage::validatePage()
{
    Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
    return device->isRegistered();
}

int NoPairingPage::nextId() const
{
    return BlueWizard::Services;
}

Device* NoPairingPage::deviceFromWizard()
{
    return Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
}

QList<QWizard::WizardButton> NoPairingPage::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;

    return list;
}
