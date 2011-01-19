/***************************************************************************
 *   This file is part of the KDE project                                  *
 *                                                                         *
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@ufocoders.com>    *
 *   Copyright (C) 2010 UFO Coders <info@ufocoders.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "selectdevicepage.h"
#include "discoverwidget.h"
#include "../sendfilewizard.h"

#include <QDesktopServices>
#include <QtGui/QVBoxLayout>

#include <KUrl>
#include <kfiledialog.h>
#include <kfilewidget.h>
#include <kdiroperator.h>
#include <kurlrequester.h>
#include <kurlcombobox.h>
#include <kpixmapsequenceoverlaypainter.h>
#include <KDebug>

#include <bluedevil/bluedevil.h>
#include <QLabel>

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

    int buttonSize = selectBtn->sizeHint().height();
    selectBtn->setFixedSize(buttonSize, buttonSize);
    selectBtn->setIcon(KIcon("document-open"));

    connect(widget, SIGNAL(deviceSelected(Device*)), this, SLOT(deviceSelected(Device*)));
    connect(selectBtn, SIGNAL(clicked(bool)), this, SLOT(openFileDialog()));
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

void SelectDevicePage::openFileDialog()
{
    //Don't worry MLaurent, I'm not going to check the pointer before delete it :)
    delete m_dialog;

    m_dialog = new KFileDialog(KUrl(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)), "*", this);
    m_dialog->setMode(KFile::Files);

    static_cast<SendFileWizard* >(wizard())->setFileDialog(m_dialog);
    connect(m_dialog, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

    m_dialog->exec();
}

void SelectDevicePage::selectionChanged()
{
    if (m_dialog->selectedUrls().isEmpty()) {
        selectLbl->setText(i18n("Select one or more files:"));
    } else {
        selectLbl->setText(i18n("Selected files: <b>%1</b>").arg(m_dialog->selectedUrls().count()));
    }
    emit completeChanged();
}

bool SelectDevicePage::isComplete() const
{
    if (!static_cast<SendFileWizard* >(wizard())->device()) {
        return false;
    }

    if (!m_dialog || m_dialog->selectedUrls().isEmpty()) {
        return false;
    }

    return true;
}
