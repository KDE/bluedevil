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


#include "pairingpage.h"
#include "../bluewizard.h"
#include "../wizardagent.h"

#include <QDBusMessage>
#include <QWizard>
#include <QAbstractButton>

#include <KDebug>
#include <kpixmapsequence.h>
#include <kpixmapsequenceoverlaypainter.h>
#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

PairingPage::PairingPage(QWidget* parent): QWizardPage(parent), m_wizard(0)
{
    setTitle(i18nc("PIN Validation page title of the wizard", "PIN Validation"));
    setupUi(this);

    QFont font(pinNumber->font());
    font.setPointSize(42);
    font.setBold(true);
    pinNumber->setFont(font);

}

void PairingPage::initializePage()
{
    kDebug() << "Initialize Page";
    if (!m_wizard) {
        kDebug() << "No wizard, getting everything again";
        m_wizard = static_cast<BlueWizard*>(wizard());
        m_device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());

        m_agent = m_wizard->agent();

        QString pin;
        if (m_wizard->manualPin()) {
            pin = m_wizard->pin();
        } else {
            pin = m_agent->getPin(m_device);
        }

        m_agent->setPin(pin);
        m_working = new KPixmapSequenceOverlayPainter(this);
        m_working->setWidget(pinNumber);
        m_working->start();

        kDebug() << "Legacy: " << m_device->hasLegacyPairing();
        if (!m_device->hasLegacyPairing()) {
            label_2->setHidden(true);
            m_wizard->setButtonText(QWizard::NextButton, i18n("PIN correct"));
            m_wizard->setButtonText(QWizard::BackButton, i18n("PIN incorrect"));
        }

        if (pin == "NULL") {
            label_2->setHidden(true);
        }

        connect(m_agent, SIGNAL(pinRequested(QString)), this, SLOT(pinRequested(QString)));
        connect(m_agent, SIGNAL(confirmationRequested(quint32,QDBusMessage)), this, SLOT(confirmationRequested(quint32,QDBusMessage)));
        connect(m_device, SIGNAL(connectedChanged(bool)), this, SLOT(nextPage()));
        connect(m_device, SIGNAL(pairedChanged(bool)), this, SLOT(nextPage()));

        doPair();
    }
}

void PairingPage::doPair()
{
    if (m_agent->pin() == "NULL") {
        m_device->UBI();//This will call createDevice, is the only way of doing it right now
    } else {
        m_device->pair("/wizardAgent", Adapter::DisplayYesNo);
    }
    kDebug() << "pair has been executed, waiting...";
}

bool PairingPage::isComplete() const
{
    if (!m_device->hasLegacyPairing() && !pinNumber->text().isEmpty()) {
        kDebug() << "True";
        return true;
    }
    if (m_agent->pin() == "NULL") {
        return true;
    }
    kDebug() << "False";
    return false;
}

int PairingPage::nextId() const
{
    if (m_device->isPaired() || m_agent->pin() == "NULL") {
        kDebug() << "Device paired";
        return BlueWizard::Services;
    }
    kDebug() << "Error, going back to introduction";
    return BlueWizard::Introduction;
}

bool PairingPage::validatePage()
{
    kDebug() << "Legacy: " << m_device->hasLegacyPairing();
     if (!m_device->isPaired() && m_agent->pin() != "NULL") {
        kDebug() << "Device is not paired";
        if (!m_device->hasLegacyPairing() && !pinNumber->text().isEmpty()) {
            kDebug() << "But device is SSP: ";
            QDBusConnection::systemBus().send(m_msg.createReply());
            pinNumber->setText("");
            m_working->start();
            emit completeChanged();
            return false;
        }
    }
    return true;
}


void PairingPage::cleanupPage()
{
    if (!m_device->hasLegacyPairing() && !pinNumber->text().isEmpty()) {
        QDBusConnection::systemBus().send(m_msg.createErrorReply("org.bluez.Error.Rejected", "The request was rejected"));
        QDBusConnection::systemBus().unregisterObject("/wizardAgent", QDBusConnection::UnregisterTree);

        if(!QDBusConnection::systemBus().registerObject("/wizardAgent", qApp)) {
            kDebug() << "The dbus object can't be registered";
        }
        kDebug();
    }

    pinNumber->setText("");
    connect(m_agent, SIGNAL(pinRequested(QString)), this, SLOT(pinRequested(QString)));
    connect(m_agent, SIGNAL(confirmationRequested(quint32,QDBusMessage)), this, SLOT(confirmationRequested(quint32,QDBusMessage)));
    disconnect(m_device, SIGNAL(connectedChanged(bool)), this, SLOT(nextPage()));
    disconnect(m_device, SIGNAL(pairedChanged(bool)), this, SLOT(nextPage()));
    m_wizard->setButtonText(QWizard::NextButton, i18n("Next"));
    m_wizard->setButtonText(QWizard::BackButton, i18n("Back"));
    m_wizard  = 0;
}

void PairingPage::nextPage()
{
    kDebug();
    disconnect(m_device, SIGNAL(connectedChanged(bool)), this, SLOT(nextPage()));
    disconnect(m_device, SIGNAL(pairedChanged(bool)), this, SLOT(nextPage()));
    m_wizard->next();
}

void PairingPage::pinRequested(const QString& pin)
{
    kDebug() << classToType(m_device->deviceClass());
    kDebug() << m_device->deviceClass();
    m_working->stop();
    pinNumber->setText(pin);
}

void PairingPage::confirmationRequested(quint32 passkey, const QDBusMessage& msg)
{
    kDebug();
    m_msg = msg;
    m_working->stop();
    pinNumber->setText(QString::number(passkey));
    emit completeChanged();
}
