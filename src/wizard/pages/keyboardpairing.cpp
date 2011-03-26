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


#include "keyboardpairing.h"
#include "bluewizard.h"

#include <KDebug>
#include <kpixmapsequenceoverlaypainter.h>

#include <bluedevil/bluedevil.h>
#include <wizardagent.h>

using namespace BlueDevil;

KeyboardPairingPage::KeyboardPairingPage(BlueWizard* parent) : QWizardPage(parent)
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

void KeyboardPairingPage::initializePage()
{
    kDebug();
    m_wizard->setButtonLayout(wizardButtonsLayout());

    connect(m_wizard->agent(), SIGNAL(pinRequested(QString)), this, SLOT(pinRequested(QString)));

    Device *device = deviceFromWizard();
    connect(device, SIGNAL(registered(Device*)), this, SLOT(registered(Device*)));

    QMetaObject::invokeMethod(device, "registerDeviceAsync", Qt::QueuedConnection);
}

void KeyboardPairingPage::registered(Device *device)
{
    connect(device, SIGNAL(pairedChanged(bool)), this, SLOT(pairedChanged(bool)));
    device->pair("/wizardAgent", Adapter::DisplayYesNo);
}

void KeyboardPairingPage::pinRequested(const QString& pin)
{
    m_working->stop();
    pinNumber->setText(pin);
}

void KeyboardPairingPage::pairedChanged(bool paired)
{
    kDebug() << paired;
    m_wizard->next();
}

bool KeyboardPairingPage::validatePage()
{
    return deviceFromWizard()->isPaired();
}


int KeyboardPairingPage::nextId() const
{
    return BlueWizard::Services;
}

Device* KeyboardPairingPage::deviceFromWizard()
{
    return Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
}

QList<QWizard::WizardButton> KeyboardPairingPage::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;

    return list;
}