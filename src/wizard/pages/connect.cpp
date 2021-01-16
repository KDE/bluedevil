/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "connect.h"
#include "../bluewizard.h"
#include "debug_p.h"

#include <QTimer>

#include <BluezQt/Device>
#include <BluezQt/PendingCall>

ConnectPage::ConnectPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
    , m_success(false)
{
    setupUi(this);
}

int ConnectPage::nextId() const
{
    if (m_success) {
        return BlueWizard::Success;
    }
    return BlueWizard::Fail;
}

void ConnectPage::initializePage()
{
    qCDebug(WIZARD) << "Initialize Connect Page";

    m_wizard->setButtonLayout(wizardButtonsLayout());
    connecting->setText(i18nc("Connecting to a Bluetooth device", "Connecting to %1...", m_wizard->device()->name()));

    m_wizard->device()->setTrusted(true);

    BluezQt::PendingCall *call = m_wizard->device()->connectToDevice();
    connect(call, &BluezQt::PendingCall::finished, this, &ConnectPage::connectFinished);
}

void ConnectPage::connectFinished(BluezQt::PendingCall *call)
{
    qCDebug(WIZARD) << "Connect finished:";
    qCDebug(WIZARD) << "\t error     : " << (bool)call->error();
    qCDebug(WIZARD) << "\t errorText : " << call->errorText();

    m_success = !call->error();
    QTimer::singleShot(500, m_wizard, &BlueWizard::next);
}

QList<QWizard::WizardButton> ConnectPage::wizardButtonsLayout() const
{
    QList<QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;
    return list;
}
