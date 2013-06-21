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
, m_validPage(false)
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

    connecting->setText(connecting->text().append(m_wizard->device()->name()));

    connect(m_wizard->device(), SIGNAL(connectedChanged(bool)), SLOT(connectedChanged(bool)));

    m_wizard->device()->connectDevice();
}

void NoPairingPage::connectedChanged(bool connected)
{
    m_validPage = connected;
    m_wizard->next();
}

bool NoPairingPage::validatePage()
{
    return m_validPage;
}

int NoPairingPage::nextId() const
{
    return BlueWizard::Services;
}

QList<QWizard::WizardButton> NoPairingPage::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;

    return list;
}
