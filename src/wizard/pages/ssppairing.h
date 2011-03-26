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
#include <QDBusMessage>

class BlueWizard;
class KPixmapSequenceOverlayPainter;

namespace BlueDevil {
    class Device;
    class Adapter;
}

using namespace BlueDevil;

class SSPPairingPage : public QWizardPage
, Ui::SSPPairing
{
Q_OBJECT

public:
    SSPPairingPage(BlueWizard* parent = 0);

    virtual void initializePage();
    virtual int nextId() const;
    virtual bool validatePage();

public Q_SLOTS:
    void registered(Device *device);
    void confirmationRequested(quint32 passkey, const QDBusMessage &msg);
    void pairedChanged(bool paired);
    void matchesClicked();
    void notMatchClicked();

protected:
    Device* deviceFromWizard();
    QList <QWizard::WizardButton> wizardButtonsLayout() const;

private:
    QDBusMessage                   m_msg;
    QWizard::WizardButton          m_buttonClicked;
    BlueWizard                    *m_wizard;
    KPixmapSequenceOverlayPainter *m_working;
};

#endif // SSPPAIRINGPAGE_H