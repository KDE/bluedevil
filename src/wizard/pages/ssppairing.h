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

#ifndef SSPPAIRINGPAGE_H
#define SSPPAIRINGPAGE_H

#include "ui_ssppairing.h"
#include <QWizardPage>

#include <QBluez/Agent>

class BlueWizard;
class KPixmapSequenceOverlayPainter;

namespace QBluez {
    class PendingCall;
}

class SSPPairingPage : public QWizardPage, Ui::SSPPairing
{
    Q_OBJECT

public:
    SSPPairingPage(BlueWizard *parent = 0);

    int nextId() const Q_DECL_OVERRIDE;
    void initializePage() Q_DECL_OVERRIDE;

public Q_SLOTS:
    void setPairableFinished(QBluez::PendingCall *call);
    void pairingFinished(QBluez::PendingCall *call);
    void confirmationRequested(const QString &passkey, const QBluez::Request<void> &req);
    void pinRequested(const QString &pin);
    void matchesClicked();
    void notMatchClicked();
    void cancelClicked();

protected:
    QList<QWizard::WizardButton> wizardButtonsLayout() const;

private:
    QBluez::Request<void> m_req;
    BlueWizard *m_wizard;
    KPixmapSequenceOverlayPainter *m_working;
    bool m_success;
};

#endif // SSPPAIRINGPAGE_H
