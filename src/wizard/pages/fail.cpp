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

#include "fail.h"
#include "../bluewizard.h"
#include "debug_p.h"

#include <QPushButton>

#include <KStandardGuiItem>
#include <KLocalizedString>
#include <KPixmapSequenceOverlayPainter>

#include <BluezQt/Device>

FailPage::FailPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
{
    setupUi(this);

    failIcon->setPixmap(QIcon::fromTheme(QStringLiteral("emblem-error")).pixmap(48));
}

void FailPage::initializePage()
{
    qCDebug(WIZARD) << "Initialize Fail Page";

    QPushButton *reset = new QPushButton(this);
    KGuiItem::assign(reset, KStandardGuiItem::reset());
    reset->setText(i18nc("Button offered when the wizard fail. This button will restart the wizard", "Restart the wizard"));
    connect(reset, &QPushButton::clicked, m_wizard, &QWizard::restart);

    m_wizard->setButton(QWizard::CustomButton3, reset);
    m_wizard->setButtonText(QWizard::CancelButton, i18nc("Button that closes the wizard", "Close"));

    QList<QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CustomButton3;
    list << QWizard::CancelButton;

    m_wizard->setButtonLayout(list);

    BluezQt::DevicePtr device = m_wizard->device();

    if (device->name().isEmpty()) {
        failLbl->setText(i18nc("This string is shown when the wizard fail", "The setup of the device has failed"));
    } else {
        failLbl->setText(i18n("The setup of %1 has failed", device->name()));
    }
}
