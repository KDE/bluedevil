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

#include <QLabel>
#include <QDesktopServices>
#include <QtGui/QVBoxLayout>

#include <KUrl>
#include <kfiledialog.h>
#include <kpixmapsequenceoverlaypainter.h>
#include <KDebug>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;
SelectDevicePage::SelectDevicePage(QWidget* parent): QWizardPage(parent), m_dialog(0)
{
    setupUi(this);

    DiscoverWidget *widget = new DiscoverWidget(this);
    widget->setContentsMargins(0, 0, 0, 0);
    discoverLayout->addWidget(widget);

    KPixmapSequenceOverlayPainter *workingPainter = new KPixmapSequenceOverlayPainter(this);
    workingPainter->setWidget(working);
    workingPainter->start();

    selectBtn->setHidden(true);
    selectLbl->setHidden(true);
    connect(widget, SIGNAL(deviceSelected(Device*)), this, SLOT(deviceSelected(Device*)));
}

void SelectDevicePage::deviceSelected(Device* device)
{
    if (!device->name().isEmpty()) {
        static_cast<SendFileWizard* >(wizard())->setDevice(device);
    } else {
        static_cast<SendFileWizard* >(wizard())->setDevice(0);
    }
    emit completeChanged();
}

bool SelectDevicePage::isComplete() const
{
    if (!static_cast<SendFileWizard* >(wizard())->device()) {
        return false;
    }

    return true;
}
