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

#ifndef LEGACYPAIRINGDATABASE_H
#define LEGACYPAIRINGDATABASE_H

#include "ui_nopairing.h"
#include <QWizardPage>

class BlueWizard;
class KPixmapSequenceOverlayPainter;

namespace QBluez {
    class PendingCall;
}

class LegacyPairingPageDatabase : public QWizardPage, Ui::NoPairing
{
    Q_OBJECT

public:
    LegacyPairingPageDatabase(BlueWizard *parent = 0);

    int nextId() const Q_DECL_OVERRIDE;
    void initializePage() Q_DECL_OVERRIDE;

public Q_SLOTS:
    void setPairableFinished(QBluez::PendingCall *call);
    void pairingFinished(QBluez::PendingCall *call);

protected:
    QList <QWizard::WizardButton> wizardButtonsLayout() const;

private:
    BlueWizard *m_wizard;
    KPixmapSequenceOverlayPainter *m_working;
    bool m_success;
};

#endif // LEGACYPAIRINGDATABASE_H
