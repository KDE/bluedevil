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

#ifndef PAIRINGPAGE_H
#define PAIRINGPAGE_H

#include "ui_pairing.h"

#include <QWizardPage>

#include <BluezQt/Device>
#include <BluezQt/Request>

class BlueWizard;

class PairingPage : public QWizardPage, Ui::Pairing
{
    Q_OBJECT

public:
    explicit PairingPage(BlueWizard *parent = Q_NULLPTR);

    int nextId() const override;
    void initializePage() override;

public Q_SLOTS:
    void pairingFinished(BluezQt::PendingCall *call);
    void pinRequested(const QString &pin);
    void confirmationRequested(const QString &passkey, const BluezQt::Request<> &req);
    void matchesClicked();
    void notMatchClicked();
    void cancelClicked();

private:
    QList<QWizard::WizardButton> wizardButtonsLayout() const;

    BlueWizard *m_wizard;
    BluezQt::DevicePtr m_device;
    BluezQt::Request<> m_req;
    bool m_success;
};

#endif // PAIRINGPAGE_H
