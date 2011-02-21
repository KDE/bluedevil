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

#include <QDebug>
#include <QFileSystemModel>
#include <QDesktopServices>

#include <KFileDialog>
#include <kstandarddirs.h>

SharedFilesDialog::SharedFilesDialog(QWidget* parent, Qt::WFlags flags): KDialog(parent, flags)
{
    QWidget *sharedFiles = new QWidget(this);
    m_ui = new Ui::sharedFiles();
    m_ui->setupUi(sharedFiles);
    setMainWidget(sharedFiles);
    m_ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QFileSystemModel *model = new QFileSystemModel();
    qDebug() << KStandardDirs().saveLocation("data", "bluedevil/shared_files/");
    QModelIndex in = model->setRootPath(KStandardDirs().saveLocation("data", "bluedevil/shared_files/"));

    m_ui->listView->setModel(model);
    m_ui->listView->setRootIndex(in);

    m_ui->addBtn->setIcon(KIcon("list-add"));
    m_ui->removeBtn->setIcon(KIcon("list-remove"));

    connect(m_ui->addBtn, SIGNAL(clicked(bool)), this, SLOT(addFiles()));
    connect(m_ui->removeBtn, SIGNAL(clicked(bool)), this, SLOT(removeFiles()));
}

void SharedFilesDialog::addFiles()
{
    KFileDialog *dialog = new KFileDialog(QDesktopServices::storageLocation(QDesktopServices::HomeLocation), "*", this);
    dialog->setMode(KFile::Directory | KFile::Files | KFile::LocalOnly);
    dialog->exec();

    QFile file;
    KUrl url;
    QString baseDir = KStandardDirs().saveLocation("data", "bluedevil/shared_files/");

    QStringList files = dialog->selectedFiles();
    Q_FOREACH(const QString &fileUrl, files) {
        file.setFileName(fileUrl);
        url.setPath(fileUrl);
        file.link(baseDir + url.fileName());
    }
}

void SharedFilesDialog::removeFiles()
{
    QItemSelectionModel *select = m_ui->listView->selectionModel();
    QModelIndexList list = select->selectedIndexes();
    QFile file;

    Q_FOREACH(const QModelIndex &index, list) {
        file.setFileName(index.data(QFileSystemModel::FilePathRole).toString());
        file.remove();
    }
}