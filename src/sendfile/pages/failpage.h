/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef FAILPAGE_H
#define FAILPAGE_H

#include "ui_failpage.h"

#include <QWizardPage>

class SendFileWizard;

class FailPage : public QWizardPage, Ui::FailPage
{
    Q_OBJECT

public:
    explicit FailPage(SendFileWizard *parent = nullptr);

    void initializePage() override;

private:
    SendFileWizard *m_wizard;
};

#endif // FAILPAGE_H
