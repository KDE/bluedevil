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
    m_wizard->setButtonLayout(wizardButtonsLayout());

    Device *device = m_wizard->device();
    connect(m_wizard->agent(), SIGNAL(pinRequested(QString)), this, SLOT(pinRequested(QString)));
    connect(device, SIGNAL(pairedChanged(bool)), this, SLOT(pairedChanged(bool)));

    device->pair();
}

void LegacyPairingPage::pinRequested(const QString& pin)
{
    m_working->stop();
    pinNumber->setText(pin);
}

void LegacyPairingPage::pairedChanged(bool paired)
{
    kDebug() << paired;
    m_wizard->next();
}

bool LegacyPairingPage::validatePage()
{
    return m_wizard->device()->isPaired();
}

int LegacyPairingPage::nextId() const
{
    return BlueWizard::Connect;
}

QList<QWizard::WizardButton> LegacyPairingPage::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;

    return list;
}