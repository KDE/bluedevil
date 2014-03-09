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
#include "bluewizard.h"

#include <KDebug>
#include <KPushButton>
#include <kpixmapsequenceoverlaypainter.h>

#include <bluedevil/bluedevil.h>
#include <QTimer>

using namespace BlueDevil;

FailPage::FailPage(BlueWizard* parent) : QWizardPage(parent)
, m_wizard(parent)
{
    setupUi(this);
}

void FailPage::initializePage()
{
    kDebug();
    KPushButton *reset = new KPushButton(KStandardGuiItem::reset());
    reset->setText(i18nc("Button offered when the wizard fail. This button will restart the wizard","Restart the wizard"));
    connect(reset, SIGNAL(clicked(bool)), m_wizard, SLOT(restartWizard()));

    m_wizard->setButton(QWizard::CustomButton3, reset);
    m_wizard->setButtonText(QWizard::CancelButton, i18nc("Button that closes the wizard","Close"));

    QList<QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CustomButton3;
    list << QWizard::CancelButton;

    m_wizard->setButtonLayout(list);

    QString deviceName = m_wizard->device()->name();
    if (deviceName.isEmpty()) {
        failLbl->setText(i18nc("This string is shown when the wizard fail","The setup of the device has failed"));
    } else {
        failLbl->setText(i18n("The setup of %1 has failed", deviceName));
    }

}
