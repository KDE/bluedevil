/*
    Copyright (C) 2010 UFO Coders <info@ufocoders.com>

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


#ifndef PINPAGE_H
#define PINPAGE_H

#include "ui_pin.h"
#include <QWizardPage>

class BlueWizard;
namespace BlueDevil {
    class Device;
}

using namespace BlueDevil;
class PinPage : public QWizardPage
, public Ui::Pin
{
Q_OBJECT

public:
    PinPage(QWidget* parent = 0);
    virtual bool isComplete() const;
    virtual void initializePage();

private Q_SLOTS:
    void manualToggle(bool checked);
    void pinChange(const QString& pin);

private:
    Device     *m_device;
    BlueWizard *m_wizard;
};

#endif // PINPAGE_H
