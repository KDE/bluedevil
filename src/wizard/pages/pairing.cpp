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

#include "pairing.h"
#include "../bluewizard.h"
#include "../wizardagent.h"
#include "debug_p.h"

#include <QPushButton>

#include <KStandardGuiItem>
#include <KLocalizedString>

#include <BluezQt/Device>
#include <BluezQt/Adapter>
#include <BluezQt/PendingCall>

PairingPage::PairingPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
    , m_success(false)
{
    setupUi(this);

    QFont font(pinNumber->font());
    font.setPointSize(42);
    font.setBold(true);
    pinNumber->setFont(font);
}

int PairingPage::nextId() const
{
    if (m_success) {
        return BlueWizard::Connect;
    }
    return BlueWizard::Fail;
}

void PairingPage::initializePage()
{
    qCDebug(WIZARD) << "Initialize Pairing Page";

    m_device = m_wizard->device();
    m_wizard->setButtonLayout(wizardButtonsLayout());

    QPushButton *cancel = new QPushButton(this);
    KGuiItem::assign(cancel, KStandardGuiItem::cancel());
    connect(cancel, &QPushButton::clicked, this, &PairingPage::cancelClicked);
    wizard()->setButton(QWizard::CustomButton1, cancel);

    frame->hide();
    progressBar->show();
    connectingLbl->show();
    confirmLbl->clear();

    connectingLbl->setText(i18n("Connecting to %1...", m_device->name()));

    connect(m_wizard->agent(), &WizardAgent::pinRequested, this, &PairingPage::pinRequested);
    connect(m_wizard->agent(), &WizardAgent::confirmationRequested, this, &PairingPage::confirmationRequested);

    BluezQt::PendingCall *pairCall = m_device->pair();
    connect(pairCall, &BluezQt::PendingCall::finished, this, &PairingPage::pairingFinished);
}

void PairingPage::pairingFinished(BluezQt::PendingCall *call)
{
    qCDebug(WIZARD) << "Pairing finished:";
    qCDebug(WIZARD) << "\t error     : " << (bool) call->error();
    qCDebug(WIZARD) << "\t errorText : " << call->errorText();

    m_success = !call->error();
    m_wizard->next();
}

void PairingPage::pinRequested(const QString &pin)
{
    // Don't ask user to introduce the PIN if it was used from database
    if (m_wizard->agent()->isFromDatabase()) {
        return;
    }

    frame->show();
    connectingLbl->hide();
    progressBar->hide();

    if (m_device->type() == BluezQt::Device::Keyboard) {
        confirmLbl->setText(i18n("Please introduce the PIN in your keyboard when it appears and press Enter"));
    } else {
        confirmLbl->setText(i18n("Please introduce the PIN in your device when it appears"));
    }

    pinNumber->setText(pin);

    m_wizard->setMinimumSize(m_wizard->sizeHint());
}

void PairingPage::confirmationRequested(const QString &passkey, const BluezQt::Request<> &req)
{
    m_req = req;

    frame->show();
    connectingLbl->hide();
    progressBar->hide();

    QPushButton *matches = new QPushButton(this);
    KGuiItem::assign(matches, KStandardGuiItem::apply());
    matches->setText(i18n("Matches"));

    QPushButton *notMatch = new QPushButton(this);
    KGuiItem::assign(notMatch, KStandardGuiItem::cancel());
    notMatch->setText(i18n("Does not match"));

    connect(matches, &QPushButton::clicked, this, &PairingPage::matchesClicked);
    connect(notMatch, &QPushButton::clicked, this, &PairingPage::notMatchClicked);

    m_wizard->setButton(QWizard::CustomButton1, matches);
    m_wizard->setButton(QWizard::CustomButton2, notMatch);

    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CustomButton1;
    list << QWizard::CustomButton2;
    m_wizard->setButtonLayout(list);

    pinNumber->setText(passkey);
    confirmLbl->setText(i18n("Please, confirm that the PIN displayed on %1 matches the wizard one.", m_wizard->device()->name()));

    m_wizard->setMinimumSize(m_wizard->sizeHint());
}

void PairingPage::matchesClicked()
{
    wizard()->button(QWizard::CustomButton1)->setEnabled(false);
    wizard()->button(QWizard::CustomButton2)->setEnabled(false);

    m_req.accept();
}

void PairingPage::notMatchClicked()
{
    m_req.reject();
}

void PairingPage::cancelClicked()
{
    m_device->cancelPairing();
}

QList<QWizard::WizardButton> PairingPage::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CustomButton1;
    return list;
}
