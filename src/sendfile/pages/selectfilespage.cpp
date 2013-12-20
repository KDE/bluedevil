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

#include "selectfilespage.h"
#include "../sendfilewizard.h"

#include <kurl.h>
#include <kfilewidget.h>
#include <kdiroperator.h>
#include <kfileitem.h>

#include <QDesktopServices>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

SelectFilesPage::SelectFilesPage(QWidget* parent): QWizardPage(parent)
{
    m_files = new KFileWidget(KUrl(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)), this);
    m_files->setMode(KFile::Files);
    m_files->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);

    connect(m_files, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_files);
}

void SelectFilesPage::selectionChanged()
{
    QStringList fileList;
    KFileItemList itemList = m_files->dirOperator()->selectedItems();
    Q_FOREACH(const KFileItem &file, itemList) {
        fileList << file.localPath();
    }
    static_cast<SendFileWizard* >(wizard())->setFiles(fileList);
    emit completeChanged();
}

bool SelectFilesPage::isComplete() const
{
    return !m_files->dirOperator()->selectedItems().isEmpty();
}