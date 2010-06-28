/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef PAIRINGPAGE_H
#define PAIRINGPAGE_H

#include "ui_pairing.h"
#include <QWizardPage>

class BlueWizard;
class PairingPage : public QWizardPage
, Ui::Pairing
{
Q_OBJECT

public:
    PairingPage(QWidget* parent = 0);

    virtual void initializePage();

public Q_SLOTS:
    void usedPin(const QString&);
    void devicePaired(bool paired);

private:
    BlueWizard *m_wizard;
};

#endif // PAIRINGPAGE_H
