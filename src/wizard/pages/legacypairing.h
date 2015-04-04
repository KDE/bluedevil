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

#ifndef LEGACYPAIRING_H
#define LEGACYPAIRING_H

#include "ui_legacypairing.h"

#include <QWizardPage>

namespace BluezQt
{
    class PendingCall;
}

class BlueWizard;
class KPixmapSequenceOverlayPainter;

class LegacyPairingPage : public QWizardPage, Ui::LegacyPairing
{
    Q_OBJECT

public:
    explicit LegacyPairingPage(BlueWizard *parent = 0);

    int nextId() const Q_DECL_OVERRIDE;
    void initializePage() Q_DECL_OVERRIDE;

public Q_SLOTS:
    void setPairableFinished(BluezQt::PendingCall *call);
    void pairingFinished(BluezQt::PendingCall *call);
    void pinRequested(const QString &pin);

private:
    QList <QWizard::WizardButton> wizardButtonsLayout() const;

    BlueWizard *m_wizard;
    KPixmapSequenceOverlayPainter *m_working;
    bool m_success;
};

#endif // LEGACYPAIRING_H
