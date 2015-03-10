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
#include "debug_p.h"

#include <QDebug>
#include <QTimer>

#include <kiconloader.h>
#include <kpixmapsequence.h>
#include <kpixmapsequenceoverlaypainter.h>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

NoPairingPage::NoPairingPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_success(false)
    , m_wizard(parent)
{
    setupUi(this);
    m_working = new KPixmapSequenceOverlayPainter(this);
    m_working->setSequence(KIconLoader::global()->loadPixmapSequence(QStringLiteral("process-working"), 22));
    m_working->setWidget(working);
    m_working->start();
}

int NoPairingPage::nextId() const
{
    if (m_success) {
        return BlueWizard::Success;
    }
    return BlueWizard::Fail;
}

void NoPairingPage::initializePage()
{
    qCDebug(WIZARD);
    m_wizard->setButtonLayout(wizardButtonsLayout());

    connecting->setText(connecting->text().append(m_wizard->device()->name()));

    connect(m_wizard->device(), SIGNAL(connectedChanged(bool)), SLOT(connectedChanged(bool)));
    connect(m_wizard->device(), SIGNAL(trustedChanged(bool)), SLOT(connectedChanged(bool)));

    m_wizard->device()->connectDevice();
    m_wizard->device()->setTrusted(true);
}

void NoPairingPage::connectedChanged(bool connected)
{
    qCDebug(WIZARD) << "Connect finished" << connected;

    // Connect may fail but that doesn't really mean the device was setup incorrectly
    // Device::connectDevice will fail eg. when A2DP profile could not be connected due to missing pulseaudio plugin
    m_success = true;
    QTimer::singleShot(500, m_wizard, SLOT(next()));
}

QList<QWizard::WizardButton> NoPairingPage::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;
    return list;
}
