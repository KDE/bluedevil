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


#include "pairingpage.h"
#include "../wizard.h"
#include "../wizardagent.h"

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;
PairingPage::PairingPage(QWidget* parent): QWizardPage(parent), m_wizard(0)
{
    setTitle("PIN Validation");
    setupUi(this);
}

void PairingPage::initializePage()
{
    if (!m_wizard) {
        m_wizard = static_cast<BlueWizard*>(wizard());
        Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
        WizardAgent *agent = m_wizard->agent();

        if (device->isPaired()) {
            qDebug() << "DEvice alreayd paired men";
        }

        QString pin;
        if (m_wizard->manualPin()) {
            pin = m_wizard->pin();
        } else {
            pin = agent->getPin(device->UBI());
        }
        agent->setPin(pin);
        pinNumber->setText(pin);

        connect(agent, SIGNAL(pinRequested(const QString&)), this, SLOT(usedPin(const QString&)));
        connect(device, SIGNAL(pairedChanged(bool)), this, SLOT(devicePaired(bool)));
        device->pair("/wizardAgent", "DisplayYesNo");
    }
}

void PairingPage::usedPin(const QString& pin)
{
    pinNumber->setText(pin);
}

void PairingPage::devicePaired(bool paired)
{
    qDebug() << "Device Paired! " << paired;
}
