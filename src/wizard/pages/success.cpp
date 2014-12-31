/*****************************************************************************
 * This file is part of the KDE project                                      *
 *                                                                           *
 * Copyright (C) 2010 Alejandro Fiestas Olivares <afiestas@kde.org>          *
 * Copyright (C) 2010-2011 UFO Coders <info@ufocoders.com>                   *
 * Copyright (C) 2014 David Rosca <nowrep@gmail.com>                         *
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

#include "success.h"
#include "bluewizard.h"

#include <KDebug>
#include <KLocalizedString>

#include <bluedevil/bluedevildevice.h>

SuccessPage::SuccessPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
{
    setupUi(this);

    successIcon->setPixmap(KIcon("task-complete").pixmap(48));
}

void SuccessPage::initializePage()
{
    kDebug() << "Initialize Success Page";

    QList<QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::FinishButton;

    m_wizard->setButtonLayout(list);

    setFinalPage(true);

    QString deviceName = m_wizard->device()->name();
    if (deviceName.isEmpty()) {
        successLbl->setText(i18nc("This string is shown when the wizard succeeds", "The setup of the device has succeeded"));
    } else {
        successLbl->setText(i18n("The setup of %1 has succeeded", deviceName));
    }
}

int SuccessPage::nextId() const
{
    return -1;
}
