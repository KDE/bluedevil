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


#include "serviceoption.h"
#include <QButtonGroup>
#include <kservice.h>

ServiceOption::ServiceOption(const KService* service, QButtonGroup& buttonGroup, QWidget* parent):
QWidget(parent)
{
    init(service->name(), service->comment(), buttonGroup);
    m_service = service;
}

ServiceOption::ServiceOption(const QString& radioText, const QString& descText, QButtonGroup& buttonGroup, QWidget* parent)
: QWidget(parent)
{
    init(radioText, descText, buttonGroup);
}

void ServiceOption::init(const QString& radioText, const QString& descText, QButtonGroup& buttonGroup)
{
    m_service = 0;
    setupUi(this);

    radioButton->setText(radioText);
    descLbl->setText(descText);
    buttonGroup.addButton(radioButton);
    connect(radioButton, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));    
}


void ServiceOption::setChecked(bool checked)
{
    radioButton->setChecked(checked);
}

void ServiceOption::toggled(bool checked)
{
    if (checked == true) {
        emit selected(m_service);
    }
}
