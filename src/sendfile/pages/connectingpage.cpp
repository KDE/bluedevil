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

#include "connectingpage.h"
#include "../sendfilewizard.h"
#include "../debug_p.h"

#include <QDBusObjectPath>

#include <KLocalizedString>

#include <BluezQt/Device>
#include <BluezQt/PendingCall>
#include <BluezQt/InitObexManagerJob>

ConnectingPage::ConnectingPage(SendFileWizard *wizard)
    : QWizardPage(wizard)
    , m_wizard(wizard)
{
    setupUi(this);
}

void ConnectingPage::initializePage()
{
    m_device = m_wizard->device();
    connLabel->setText(i18nc("Connecting to a Bluetooth device", "Connecting to %1...", m_device->name()));

    m_wizard->setWindowTitle(QString());
    m_wizard->setButtonLayout(wizardButtonsLayout());

    // Init BluezQt
    BluezQt::ObexManager *manager = new BluezQt::ObexManager(this);
    BluezQt::InitObexManagerJob *job = manager->init();
    job->start();
    connect(job, &BluezQt::InitObexManagerJob::result, this, &ConnectingPage::initJobResult);
}

bool ConnectingPage::isComplete() const
{
    return false;
}

void ConnectingPage::initJobResult(BluezQt::InitObexManagerJob *job)
{
    if (job->error()) {
        qCWarning(SENDFILE) << "Error initializing obex manager" << job->errorText();
        m_wizard->next();
        return;
    }

    // Create ObjectPush session
    QVariantMap map;
    map[QStringLiteral("Target")] = QStringLiteral("opp");
    BluezQt::PendingCall *call = job->manager()->createSession(m_device->address(), map);
    connect(call, &BluezQt::PendingCall::finished, this, &ConnectingPage::createSessionFinished);
}

void ConnectingPage::createSessionFinished(BluezQt::PendingCall *call)
{
    if (call->error()) {
        qCWarning(SENDFILE) << "Error creating session" << call->errorText();
        m_wizard->next();
        return;
    }

    m_wizard->startTransfer(call->value().value<QDBusObjectPath>());
}

QList<QWizard::WizardButton> ConnectingPage::wizardButtonsLayout() const
{
    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;
    return list;
}
