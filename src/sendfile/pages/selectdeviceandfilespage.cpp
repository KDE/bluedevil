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

#include "selectdeviceandfilespage.h"
#include "../discoverwidget.h"
#include "../sendfilewizard.h"

#include <QLabel>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QUrl>
#include <QIcon>
#include <QFileDialog>

#include <KLocalizedString>

#include <BluezQt/Device>

SelectDeviceAndFilesPage::SelectDeviceAndFilesPage(SendFileWizard *wizard)
    : QWizardPage(wizard)
    , m_wizard(wizard)
{
    setupUi(this);

    DiscoverWidget *widget = new DiscoverWidget(m_wizard->manager(), this);
    widget->setContentsMargins(0, 0, 0, 0);
    discoverLayout->addWidget(widget);

    int buttonSize = selectBtn->sizeHint().height();
    selectBtn->setFixedSize(buttonSize, buttonSize);
    selectBtn->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));

    connect(widget, &DiscoverWidget::deviceSelected, this, &SelectDeviceAndFilesPage::deviceSelected);
    connect(selectBtn, &QPushButton::clicked, this, &SelectDeviceAndFilesPage::openFileDialog);
}

void SelectDeviceAndFilesPage::deviceSelected(BluezQt::DevicePtr device)
{
    static_cast<SendFileWizard*>(wizard())->setDevice(device);

    Q_EMIT completeChanged();
}

void SelectDeviceAndFilesPage::openFileDialog()
{
    const QStringList &files = QFileDialog::getOpenFileNames(this, i18n("Open file..."),
                                                             QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                                                             QStringLiteral("*"));

    if (files.isEmpty()) {
        selectLbl->setText(i18n("Select one or more files:"));
    } else {
        QStringList fileNames;
        Q_FOREACH (const QString &file, files) {
            QFileInfo info(file);
            fileNames.append(info.fileName());
        }
        selectLbl->setText(i18n("Selected files: <b>%1</b>", fileNames.join(QStringLiteral(", "))));
    }

    m_wizard->setFiles(files);

    Q_EMIT completeChanged();
}

bool SelectDeviceAndFilesPage::isComplete() const
{
    if (!m_wizard->device()) {
        return false;
    }

    if (m_wizard->files().isEmpty()) {
        return false;
    }

    return true;
}
