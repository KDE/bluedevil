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
#include "../debug_p.h"

#include <QDebug>

#include <KLocalizedString>
#include <kiconloader.h>
#include <kpixmapsequence.h>
#include <kpixmapsequenceoverlaypainter.h>

#include <QBluez/Adapter>
#include <QBluez/Device>
#include <QBluez/PendingCall>

LegacyPairingPageDatabase::LegacyPairingPageDatabase(BlueWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
    , m_success(false)
{
    setupUi(this);

    m_working = new KPixmapSequenceOverlayPainter(this);
    m_working->setSequence(KIconLoader::global()->loadPixmapSequence(QStringLiteral("process-working"), 22));
    m_working->setWidget(working);
    m_working->start();
}

void LegacyPairingPageDatabase::initializePage()
{
    qCDebug(WIZARD) << "Initialize Legacy Database Pairing Page";

    m_wizard->setButtonLayout(wizardButtonsLayout());

    QBluez::Device *device = m_wizard->device();
    connecting->setText(i18n("Connecting to %1...", device->name()));

    // Adapter must be pairable, otherwise pairing would fail
    QBluez::PendingCall *call = device->adapter()->setPairable(true);
    connect(call, &QBluez::PendingCall::finished, [ this, device ]() {
        QBluez::PendingCall *call = device->pair();
        connect(call, &QBluez::PendingCall::finished, this, &LegacyPairingPageDatabase::pairingFinished);
    });
}

int LegacyPairingPageDatabase::nextId() const
{
    return BlueWizard::Connect;
}

void LegacyPairingPageDatabase::pairingFinished(QBluez::PendingCall *call)
{
    qCDebug(WIZARD) << "Legacy Database Pairing finished:";
    qCDebug(WIZARD) << "\t error     : " << (bool) call->error();
    qCDebug(WIZARD) << "\t errorText : " << call->errorText();

    m_success = !call->error();
    wizard()->next();
}

QList< QWizard::WizardButton > LegacyPairingPageDatabase::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;

    return list;
}
