/***************************************************************************
 *   This file is part of the KDE project                                  *
 *                                                                         *
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@ufocoders.com>    *
 *   Copyright (C) 2010 UFO Coders <info@ufocoders.com>                    *
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

#ifndef SELECTDEVICEPAGE_H
#define SELECTDEVICEPAGE_H

#include "ui_selectfilediscover.h"

#include <QWizard>

class KUrl;
class KFileDialog;
namespace BlueDevil {
    class Device;
}
using namespace BlueDevil;

class SelectDevicePage : public QWizardPage ,
        public Ui::SelectFileDiscover
{
Q_OBJECT
public:
    SelectDevicePage(QWidget* parent = 0);

    virtual bool isComplete() const;

private Q_SLOTS:
    void deviceSelected(Device*);
    void openFileDialog();
    void selectionChanged();

private:
    KFileDialog *m_dialog;
};

#endif // SELECTDEVICEPAGE_H
