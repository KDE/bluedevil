/***************************************************************************
 *   Copyright (C) 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org> *
 *   Copyright (C) 2010-2011 UFO Coders <info@ufocoders.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "sharedfilesdialog.h"
#include "ui_sharedfiles.h"
#include "linkproxymodel.h"
#include "filereceiversettings.h"

#include <QVBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFileSystemModel>
#include <QDialogButtonBox>

#include <KLocalizedString>

SharedFilesDialog::SharedFilesDialog(QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    m_ui = new Ui::sharedFiles();
    m_ui->setupUi(this);
    m_ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QFileSystemModel *model = new QFileSystemModel();
    QModelIndex in = model->setRootPath(FileReceiverSettings::self()->rootFolder().path());

    LinkProxyModel *proxy = new LinkProxyModel();
    proxy->setSourceModel(model);

    m_ui->listView->setModel(proxy);
    m_ui->listView->setRootIndex(proxy->mapFromSource(in));

    m_ui->addBtn->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    m_ui->removeBtn->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));

    connect(this, SIGNAL(finished(int)), this, SLOT(slotFinished(int)));
    connect(m_ui->addBtn, SIGNAL(clicked(bool)), this, SLOT(addFiles()));
    connect(m_ui->removeBtn, SIGNAL(clicked(bool)), this, SLOT(removeFiles()));
}

void SharedFilesDialog::slotFinished(int result)
{
    if (result == 1) {
        return;
    }

    QUrl url;
    QString baseDir = FileReceiverSettings::self()->rootFolder().path().append(QLatin1Char('/'));
    if (!m_added.isEmpty()) {
        Q_FOREACH(const QString &filePath, m_added) {
            url.setPath(filePath);
            QFile::remove(baseDir + url.fileName());
        }
    }
    if (!m_removed.isEmpty()) {
        Q_FOREACH(const QString &filePath, m_removed) {
            url.setPath(filePath);
            QFile::link(filePath, baseDir + url.fileName());
        }
    }
}

void SharedFilesDialog::addFiles()
{
    QFileDialog *dialog = new QFileDialog(this, i18n("Add files..."));
    dialog->setFileMode(QFileDialog::FileMode(QFileDialog::Directory| QFileDialog::ExistingFiles));
    dialog->setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    dialog->setNameFilter(QStringLiteral("*"));
    dialog->exec();

    QFile fileExist;
    QUrl url;
    QString linkPath;
    QString baseDir = FileReceiverSettings::self()->rootFolder().path().append(QLatin1Char('/'));

    QStringList files = dialog->selectedFiles();
    Q_FOREACH(const QString &filePath, files) {
        url.setPath(filePath);

        linkPath = baseDir + url.fileName();
        fileExist.setFileName(linkPath);
        if (fileExist.exists()) {
            continue;
        }

        QFile::link(filePath, linkPath);
        if (m_removed.contains(filePath)) {
            m_removed.removeOne(filePath);
            continue;
        }
        if (!m_added.contains(filePath)) {
            m_added.append(filePath);
        }
    }
}

void SharedFilesDialog::removeFiles()
{
    QItemSelectionModel *select = m_ui->listView->selectionModel();
    QModelIndexList list = select->selectedIndexes();

    QFile file;
    QString linkPath;
    Q_FOREACH(const QModelIndex &index, list) {
        linkPath = index.data(QFileSystemModel::FilePathRole).toString();
        file.setFileName(linkPath);
        if (m_added.contains(file.symLinkTarget())) {
            m_added.removeOne(file.symLinkTarget());
        } else if (!m_removed.contains(file.symLinkTarget())) {
            m_removed.append(file.symLinkTarget());
        }

        file.remove();
    }
}
