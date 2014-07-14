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
#include "wizardagent.h"
#include "debug_p.h"

#include <QDebug>
#include <QPushButton>
#include <QDBusConnection>

#include <KStandardGuiItem>
#include <KLocalizedString>
#include <kiconloader.h>
#include <kpixmapsequence.h>
#include <kpixmapsequenceoverlaypainter.h>

#include <QBluez/Adapter>
#include <QBluez/Device>
#include <QBluez/PendingCall>

SSPPairingPage::SSPPairingPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
    , m_success(false)
{
    setupUi(this);
    m_working = new KPixmapSequenceOverlayPainter(this);
    m_working->setSequence(KIconLoader::global()->loadPixmapSequence(QStringLiteral("process-working"), 22));
    m_working->setWidget(pinNumber);
    m_working->start();

    QFont font(pinNumber->font());
    font.setPointSize(42);
    font.setBold(true);
    pinNumber->setFont(font);
}

void SSPPairingPage::initializePage()
{
    qCDebug(WIZARD) << "Initialize Secure Simple Pairing Page";

    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CustomButton1;
    m_wizard->setButtonLayout(list);

    QPushButton *cancel = new QPushButton(this);
    KGuiItem::assign(cancel, KStandardGuiItem::cancel());

    connect(cancel, &QPushButton::clicked, this, &SSPPairingPage::cancelClicked);
    wizard()->setButton(QWizard::CustomButton1, cancel);

    QBluez::Device *device = m_wizard->device();
    confirmLbl->setText(i18n("Connecting to %1...", device->name()));

    connect(m_wizard->agent(), &WizardAgent::confirmationRequested, this, &SSPPairingPage::confirmationRequested);
    connect(m_wizard->agent(), &WizardAgent::pinRequested, this, &SSPPairingPage::pinRequested);

    // Adapter must be pairable, otherwise pairing would fail
    QBluez::PendingCall *call = device->adapter()->setPairable(true);
    connect(call, &QBluez::PendingCall::finished, [ this, device ]() {
        QBluez::PendingCall *call = device->pair();
        connect(call, &QBluez::PendingCall::finished, this, &SSPPairingPage::pairingFinished);
    });
}

void SSPPairingPage::confirmationRequested(const QString &passkey, const QBluez::Request<void> &req)
{
    m_req = req;

    QPushButton *matches = new QPushButton(this);
    KGuiItem::assign(matches, KStandardGuiItem::apply());
    matches->setText(i18n("Matches"));

    QPushButton *notMatch = new QPushButton(this);
    KGuiItem::assign(notMatch, KStandardGuiItem::cancel());
    notMatch->setText(i18n("Does not match"));

    connect(matches, &QPushButton::clicked, this, &SSPPairingPage::matchesClicked);
    connect(notMatch, &QPushButton::clicked, this, &SSPPairingPage::notMatchClicked);

    wizard()->setButton(QWizard::CustomButton1, matches);
    wizard()->setButton(QWizard::CustomButton2, notMatch);

    wizard()->setButtonLayout(wizardButtonsLayout());

    m_working->stop();
    pinNumber->setText(passkey);

    confirmLbl->setText(i18n("Please, confirm that the PIN displayed on \"%1\" matches the wizard one.", m_wizard->device()->name()));
}

void SSPPairingPage::pairingFinished(QBluez::PendingCall *call)
{
    qCDebug(WIZARD) << "Secure Pairing finished:";
    qCDebug(WIZARD) << "\t error     : " << (bool) call->error();
    qCDebug(WIZARD) << "\t errorText : " << call->errorText();

    // TODO: We can show the error message to user here
    m_success = !call->error();
    wizard()->next();
}

void SSPPairingPage::cancelClicked()
{
    m_wizard->device()->cancelPairing();
}

void SSPPairingPage::pinRequested(const QString& pin)
{
    m_working->stop();
    pinNumber->setText(pin);
    confirmLbl->setText(i18n("Please introduce the PIN in your device when it appears"));
}

void SSPPairingPage::matchesClicked()
{
    wizard()->button(QWizard::CustomButton1)->setEnabled(false);
    wizard()->button(QWizard::CustomButton2)->setEnabled(false);

    m_req.accept();
}

void SSPPairingPage::notMatchClicked()
{
    m_req.reject();
}

int SSPPairingPage::nextId() const
{
    if (m_success) {
        return BlueWizard::Connect;
    }
    return BlueWizard::Fail;
}

QList<QWizard::WizardButton> SSPPairingPage::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CustomButton2;
    list << QWizard::CustomButton1;

    return list;
}
