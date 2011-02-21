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

#ifndef SHAREDFILESDIALOG_H
#define SHAREDFILESDIALOG_H

#include <kdialog.h>

class Ui_sharedFiles;
class SharedFilesDialog : public KDialog
{
    Q_OBJECT
public:
    SharedFilesDialog(QWidget* parent = 0, Qt::WFlags flags = 0);

private Q_SLOTS:
    void slotFinished(int result);
    void addFiles();
    void removeFiles();

private:
    Ui_sharedFiles *m_ui;
    QStringList m_removed;
    QStringList m_added;
};

#endif // SHAREDFILESDIALOG_H
