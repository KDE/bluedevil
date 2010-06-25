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


#include "pinpage.h"
#include "ui_pin.h"
#include "../wizard.h"

#include <QDebug>

PinPage::PinPage(QWidget* parent): QWizardPage(parent), m_wizard(0)
{
    setTitle(i18n("Choose your PIN mode"));
    setupUi(this);

    connect(manualBtn, SIGNAL(toggled(bool)), this, SLOT(manualToggle(bool)));
    connect(pinEdit, SIGNAL(textChanged(QString)), this, SLOT(pinChange(QString)));
}

void PinPage::initializePage()
{
    if (!m_wizard) {
        m_wizard = static_cast<BlueWizard* >(wizard());
    }
}

void PinPage::manualToggle(bool checked)
{
    if (checked == true) {
        pinEdit->setFocus();
        pinEdit->setCursorPosition(0);
    }

    pinEdit->setEnabled(checked);
    m_wizard->setAutoPin(checked);
    emit completeChanged();
}

void PinPage::pinChange(const QString& pin)
{
    m_wizard->setPin(pin.toAscii());
    emit completeChanged();
}

bool PinPage::isComplete() const
{
    if (manualBtn->isChecked()) {
        if (!pinEdit->text().isEmpty()) {
            if (pinEdit->text().length() > 3) {
                return true;
            }
        }
        return false;
    }
    return true;
}
