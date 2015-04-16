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

#ifndef CONNECT_H
#define CONNECT_H

#include "ui_connect.h"

#include <QWizardPage>

namespace BluezQt
{
    class PendingCall;
}

class BlueWizard;

class ConnectPage : public QWizardPage, Ui::Connect
{
    Q_OBJECT

public:
    explicit ConnectPage(BlueWizard *parent = 0);

    int nextId() const Q_DECL_OVERRIDE;
    void initializePage() Q_DECL_OVERRIDE;

protected:
    QList<QWizard::WizardButton> wizardButtonsLayout() const;

private Q_SLOTS:
    void connectFinished(BluezQt::PendingCall *call);

private:
    BlueWizard *m_wizard;
    bool m_success;
};

#endif // CONNECT_H
