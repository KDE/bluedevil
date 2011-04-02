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
    QModelIndex in = model->setRootPath(FileReceiverSettings::self()->rootFolder().path());

    LinkProxyModel *proxy = new LinkProxyModel();
    proxy->setSourceModel(model);

    m_ui->listView->setModel(proxy);
    m_ui->listView->setRootIndex(proxy->mapFromSource(in));

    m_ui->addBtn->setIcon(KIcon("list-add"));
    m_ui->removeBtn->setIcon(KIcon("list-remove"));

    connect(this, SIGNAL(finished(int)), this, SLOT(slotFinished(int)));
    connect(m_ui->addBtn, SIGNAL(clicked(bool)), this, SLOT(addFiles()));
    connect(m_ui->removeBtn, SIGNAL(clicked(bool)), this, SLOT(removeFiles()));
}

void SharedFilesDialog::slotFinished(int result)
{
    if (result == 1) {
        return;
    }

    KUrl url;
    QString baseDir = FileReceiverSettings::self()->rootFolder().path().append("/");
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
    KFileDialog *dialog = new KFileDialog(QDesktopServices::storageLocation(QDesktopServices::HomeLocation), "*", this);
    dialog->setMode(KFile::Directory | KFile::Files | KFile::LocalOnly);
    dialog->exec();

    QFile fileExist;
    KUrl url;
    QString linkPath;
    QString baseDir = FileReceiverSettings::self()->rootFolder().path().append("/");

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
            continue;
        }
        if (!m_removed.contains(file.symLinkTarget())) {
            m_removed.append(file.symLinkTarget());
        }

        file.remove();
    }
}