/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

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
    explicit ConnectPage(BlueWizard *parent = nullptr);

    int nextId() const override;
    void initializePage() override;

protected:
    QList<QWizard::WizardButton> wizardButtonsLayout() const;

private Q_SLOTS:
    void connectFinished(BluezQt::PendingCall *call);

private:
    BlueWizard *m_wizard;
    bool m_success;
};

#endif // CONNECT_H
