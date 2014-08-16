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

#include "connect.h"
#include "bluewizard.h"
#include "debug_p.h"

#include <QDebug>
#include <QTimer>

#include <kiconloader.h>
#include <kpixmapsequence.h>
#include <kpixmapsequenceoverlaypainter.h>

#include <QBluez/Device>
#include <QBluez/PendingCall>

ConnectPage::ConnectPage(BlueWizard *parent)
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

int ConnectPage::nextId() const
{
    if (m_success) {
        return BlueWizard::Success;
    }
    return BlueWizard::Fail;
}

void ConnectPage::initializePage()
{
    qCDebug(WIZARD) << "Initialize Connect Page";

    m_wizard->setButtonLayout(wizardButtonsLayout());
    connecting->setText(connecting->text().append(m_wizard->device()->name()));

    m_wizard->device()->connectDevice();

    QBluez::PendingCall *call = m_wizard->device()->setTrusted(true);
    connect(call, &QBluez::PendingCall::finished, this, &ConnectPage::setTrustedFinished);
}

void ConnectPage::setTrustedFinished(QBluez::PendingCall *call)
{
    qCDebug(WIZARD) << "SetTrusted finished:";
    qCDebug(WIZARD) << "\t error     : " << (bool) call->error();
    qCDebug(WIZARD) << "\t errorText : " << call->errorText();

    m_success = !call->error();
    QTimer::singleShot(1000, m_wizard, SLOT(next()));
}

QList<QWizard::WizardButton> ConnectPage::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;
    return list;
}
