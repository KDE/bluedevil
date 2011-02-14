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
    Ui_sharedFiles *ui = new Ui::sharedFiles();
    ui->setupUi(sharedFiles);
    setMainWidget(sharedFiles);

    QFileSystemModel *model = new QFileSystemModel();
    qDebug() << KStandardDirs().saveLocation("data", "bluedevil/shared_files/");
    QModelIndex in = model->setRootPath(KStandardDirs().saveLocation("data", "bluedevil/shared_files/"));
    
    ui->listView->setModel(model);
    ui->listView->setRootIndex(in);

    ui->addBtn->setIcon(KIcon("list-add"));
    ui->removeBtn->setIcon(KIcon("list-remove"));

    connect(ui->addBtn, SIGNAL(clicked(bool)), this, SLOT(addFles()));
}

void SharedFilesDialog::addFles()
{
    KFileDialog *dialog = new KFileDialog(QDesktopServices::storageLocation(QDesktopServices::HomeLocation), "*", this);
    dialog->setMode(KFile::Directory | KFile::Files | KFile::LocalOnly);
    dialog->exec();

    qDebug() << dialog->selectedFiles();
}

