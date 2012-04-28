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


#include "ssppairing.h"
#include "bluewizard.h"

#include <KDebug>
#include <KPushButton>
#include <kpixmapsequenceoverlaypainter.h>

#include <bluedevil/bluedevil.h>
#include <wizardagent.h>

using namespace BlueDevil;

SSPPairingPage::SSPPairingPage(BlueWizard* parent) : QWizardPage(parent)
, m_buttonClicked(QWizard::NoButton)
, m_wizard(parent)
{
    setupUi(this);
    m_working = new KPixmapSequenceOverlayPainter(this);
    m_working->setWidget(pinNumber);
    m_working->start();

    QFont font(pinNumber->font());
    font.setPointSize(42);
    font.setBold(true);
    pinNumber->setFont(font);
}

void SSPPairingPage::initializePage()
{
    kDebug();
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;
    m_wizard->setButtonLayout(list);

    Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
    confirmLbl->setText(confirmLbl->text().arg(device->name()));

    connect(device, SIGNAL(registered(Device*)), this, SLOT(registered(Device*)));

    QMetaObject::invokeMethod(device, "registerDeviceAsync", Qt::QueuedConnection);
}

void SSPPairingPage::registered(Device* device)
{
    connect(device, SIGNAL(pairedChanged(bool)), this, SLOT(pairedChanged(bool)));
    connect(m_wizard->agent(), SIGNAL(confirmationRequested(quint32,QDBusMessage)),
            this, SLOT(confirmationRequested(quint32,QDBusMessage)));

    device->pair("/wizardAgent", Adapter::DisplayYesNo);
}

void SSPPairingPage::confirmationRequested(quint32 passkey, const QDBusMessage& msg)
{
    m_msg = msg;

    KPushButton *matches = new KPushButton(KStandardGuiItem::apply());
    matches->setText(i18n("Matches"));
    KPushButton *notMatch = new KPushButton(KStandardGuiItem::cancel());
    notMatch->setText(i18n("Does not match"));

    connect(matches, SIGNAL(clicked(bool)), this, SLOT(matchesClicked()));
    connect(notMatch, SIGNAL(clicked(bool)), this, SLOT(notMatchClicked()));

    wizard()->setButton(QWizard::CustomButton1, matches);
    wizard()->setButton(QWizard::CustomButton2, notMatch);

    wizard()->setButtonLayout(wizardButtonsLayout());

    m_working->stop();
    pinNumber->setText(QString::number(passkey));

    Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
    confirmLbl->setText(i18n("Please, confirm that the PIN displayed on \"%1\" matches the wizard one.", device->name()));

}

void SSPPairingPage::pairedChanged(bool paired)
{
    kDebug() << paired;

    wizard()->next();
}

void SSPPairingPage::matchesClicked()
{
    wizard()->button(QWizard::CustomButton1)->setEnabled(false);
    wizard()->button(QWizard::CustomButton2)->setEnabled(false);

    m_buttonClicked = QWizard::CustomButton1;
    QDBusConnection::systemBus().send(m_msg.createReply());

    wizard()->next();
}

void SSPPairingPage::notMatchClicked()
{
    m_buttonClicked = QWizard::CustomButton2;

    wizard()->next();
}

bool SSPPairingPage::validatePage()
{
    if (m_buttonClicked == QWizard::CustomButton2){
        return true;
    }
    if (deviceFromWizard()->isPaired() &&  m_buttonClicked == QWizard::NoButton) {
        return true;
    }
    if (deviceFromWizard()->isPaired() &&  m_buttonClicked == QWizard::CustomButton1) {
        return true;
    }

    return false;
}

int SSPPairingPage::nextId() const
{
    if (m_buttonClicked == QWizard::CustomButton2) {
        return BlueWizard::Fail;
    }

    return BlueWizard::Services;
}

Device* SSPPairingPage::deviceFromWizard()
{
    return Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
}

QList<QWizard::WizardButton> SSPPairingPage::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CustomButton2;
    list << QWizard::CustomButton1;

    return list;
}
