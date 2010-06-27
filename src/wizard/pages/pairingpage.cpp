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
#include "../bluewizard.h"
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
        m_device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
        WizardAgent *agent = m_wizard->agent();

        QString pin;
        if (m_wizard->manualPin()) {
            pin = m_wizard->pin();
        } else {
            pin = agent->getPin(m_device->UBI());
        }

        agent->setPin(pin);
        pinNumber->setText(pin);

        connect(agent, SIGNAL(pinRequested(const QString&)), pinNumber, SLOT(setText(QString)));
        connect(m_device, SIGNAL(connectedChanged(bool)), this, SLOT(nextPage()));
        connect(m_device, SIGNAL(pairedChanged(bool)), this, SLOT(nextPage()));

        m_device->pair("/wizardAgent", "DisplayYesNo");
    }
}

bool PairingPage::isComplete() const
{
    return false;
}

int PairingPage::nextId() const
{
    if (m_device->isPaired()) {
        return BlueWizard::Services;
    }
    return BlueWizard::Introduction;
}

void PairingPage::nextPage()
{
    disconnect(m_device, SIGNAL(connectedChanged(bool)), this, SLOT(nextPage()));
    disconnect(m_device, SIGNAL(pairedChanged(bool)), this, SLOT(nextPage()));
    m_wizard->next();
}
