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


#include "wizard.h"
#include "wizardagent.h"
#include "pages/introductionpage.h"
#include "pages/discoverpage.h"
#include "pages/pinpage.h"

#include <QDBusConnection>
#include <QApplication>
BlueWizard::BlueWizard() : QWizard()
{
    addPage(new IntroductionPage(this));
    addPage(new DiscoverPage(this));
    addPage(new PinPage(this));

    if(!QDBusConnection::systemBus().registerObject("/wizardAgent", qApp)) {
        qDebug() << "The dbus object can't be registered";
    }

    m_agent = new WizardAgent(qApp);
    show();
}

BlueWizard::~BlueWizard()
{

}

void BlueWizard::setDeviceAddress(const QByteArray& address)
{
    m_deviceAddress = address;
}

QByteArray BlueWizard::deviceAddress() const
{
    return m_deviceAddress;
}

void BlueWizard::setPin(const QByteArray& pinNum)
{
    m_pin = pinNum;
}

QByteArray BlueWizard::pin() const
{
    return m_pin;
}

void BlueWizard::setAutoPin(bool pinAuto)
{
    m_autoPin = pinAuto;
}

bool BlueWizard::autoPin() const
{
    return m_autoPin;
}
