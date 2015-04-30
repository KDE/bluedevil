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

#ifndef CONNECTINGPAGE_H
#define CONNECTINGPAGE_H

#include "ui_connecting.h"

#include <QWizardPage>

#include <BluezQt/ObexManager>

class SendFileWizard;

class ConnectingPage : public QWizardPage, public Ui::Connecting
{
    Q_OBJECT

public:
    explicit ConnectingPage(SendFileWizard *wizard = Q_NULLPTR);

    void initializePage() Q_DECL_OVERRIDE;
    bool isComplete() const Q_DECL_OVERRIDE;

private Q_SLOTS:
    void initJobResult(BluezQt::InitObexManagerJob *job);
    void createSessionFinished(BluezQt::PendingCall *call);

private:
    QList<QWizard::WizardButton> wizardButtonsLayout() const;

    SendFileWizard *m_wizard;
    BluezQt::DevicePtr m_device;
};

#endif // CONNECTINGPAGE_H
