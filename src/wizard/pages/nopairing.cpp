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

#include <QBluez/Device>

NoPairingPage::NoPairingPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_validPage(false)
    , m_wizard(parent)
{
    setupUi(this);

    m_working = new KPixmapSequenceOverlayPainter(this);
    m_working->setSequence(KIconLoader::global()->loadPixmapSequence(QStringLiteral("process-working"), 22));
    m_working->setWidget(working);
    m_working->start();
}

void NoPairingPage::initializePage()
{
    qCDebug(WIZARD) << "Initialize No Pairing Page";

    m_wizard->setButtonLayout(wizardButtonsLayout());

    connecting->setText(connecting->text().append(m_wizard->device()->name()));

    // It can happen that the device is technically connected and trusted but we are not connected
    // to the profile. We have no way to know if the profile was activated or not so we have to relay
    // on a timeout (10s)
    QTimer::singleShot(10000, this, SLOT(timeout()));
    connect(m_wizard->device(), &QBluez::Device::connectedChanged, this , &NoPairingPage::connectedChanged);
    connect(m_wizard->device(), &QBluez::Device::trustedChanged, this, &NoPairingPage::connectedChanged);

    m_wizard->device()->connect();
    m_wizard->device()->setTrusted(true);
}

void NoPairingPage::timeout()
{
    connectedChanged(true);
}

void NoPairingPage::connectedChanged(bool connected)
{
    qCDebug(WIZARD) << "NoPairingPage::connectedChanged" << connected;

    m_validPage = connected;
    if (m_validPage) {
        qCDebug(WIZARD) << "Connected";
        m_wizard->next();
    }
}

bool NoPairingPage::validatePage()
{
    return m_validPage;
}

int NoPairingPage::nextId() const
{
    return BlueWizard::Success;
}

QList<QWizard::WizardButton> NoPairingPage::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;

    return list;
}
