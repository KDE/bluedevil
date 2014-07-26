/*****************************************************************************
 * This file is part of the KDE project                                      *
 *                                                                           *
 * Copyright (C) 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>     *
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

#include "selectdevicepage.h"
#include "discoverwidget.h"
#include "../sendfilewizard.h"

#include <kiconloader.h>
#include <kpixmapsequence.h>
#include <kpixmapsequenceoverlaypainter.h>

#include <QBluez/Device>

SelectDevicePage::SelectDevicePage(SendFileWizard *parent)
    : QWizardPage(parent)
    , m_dialog(0)
    , m_wizard(parent)
{
    setupUi(this);

    DiscoverWidget *widget = new DiscoverWidget(m_wizard->manager(), this);
    widget->setContentsMargins(0, 0, 0, 0);
    discoverLayout->addWidget(widget);

    KPixmapSequenceOverlayPainter *workingPainter = new KPixmapSequenceOverlayPainter(this);
    workingPainter->setSequence(KIconLoader::global()->loadPixmapSequence(QStringLiteral("process-working"), 22));
    workingPainter->setWidget(working);
    workingPainter->start();

    selectBtn->setHidden(true);
    selectLbl->setHidden(true);
    connect(widget, &DiscoverWidget::deviceSelected, this, &SelectDevicePage::deviceSelected);
}

void SelectDevicePage::deviceSelected(QBluez::Device *device)
{
    if (!device->name().isEmpty()) {
        m_wizard->setDevice(device);
    } else {
        m_wizard->setDevice(0);
    }
    emit completeChanged();
}

bool SelectDevicePage::isComplete() const
{
    if (!m_wizard->device()) {
        return false;
    }

    return true;
}
