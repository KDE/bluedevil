/*****************************************************************************
 * This file is part of the KDE project                                      *
 *                                                                           *
 * Copyright (C) 2015 David Rosca <nowrep@gmail.com>                         *
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

#include "failpage.h"
#include "../sendfilewizard.h"
#include "debug_p.h"

#include <QPushButton>

#include <KStandardGuiItem>
#include <KLocalizedString>
#include <KPixmapSequenceOverlayPainter>

#include <BluezQt/Device>

FailPage::FailPage(SendFileWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
{
    setupUi(this);

    failIcon->setPixmap(QIcon::fromTheme(QStringLiteral("task-reject")).pixmap(48));
}

void FailPage::initializePage()
{
    qCDebug(SENDFILE) << "Initialize Fail Page";

    QList<QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;

    m_wizard->setButtonLayout(list);

    BluezQt::DevicePtr device = m_wizard->device();

    if (device->name().isEmpty()) {
        failLbl->setText(i18nc("This string is shown when the wizard fail", "The connection to the device has failed"));
    } else {
        failLbl->setText(i18n("The connection to %1 has failed", device->name()));
    }
}
