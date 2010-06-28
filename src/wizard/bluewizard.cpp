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


#include "bluewizard.h"
#include "wizardagent.h"
#include "pages/introductionpage.h"
#include "pages/discoverpage.h"
#include "pages/pinpage.h"
#include "pages/pairingpage.h"
#include "pages/servicespage.h"
#include "plugins/serviceplugin.h"

#include <QDBusConnection>
#include <QApplication>

#include <bluedevil/bluedevil.h>
#include <KServiceTypeTrader>

BlueWizard::BlueWizard() : QWizard(), m_service(0), m_manualPin(false)
{

    setPage(Introduction, new IntroductionPage(this));
    setPage(Discover, new DiscoverPage(this));
    setPage(Pin, new PinPage(this));
    setPage(Pairing, new PairingPage(this));
    setPage(Services, new ServicesPage(this));

    //First show, then do the rest
    show();

    if(!QDBusConnection::systemBus().registerObject("/wizardAgent", qApp)) {
        qDebug() << "The dbus object can't be registered";
    }

    m_agent = new WizardAgent(qApp);
    m_services = KServiceTypeTrader::self()->query("BlueDevil/ServicePlugin");
}

void BlueWizard::done(int result)
{
    QWizard::done(result);

    //If we have a service to connect with
    if (m_service) {
        KPluginFactory *factory = KPluginLoader(m_service->library()).factory();

        Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_deviceAddress);
        ServicePlugin *plugin = factory->create<ServicePlugin>(this);
        connect(plugin, SIGNAL(finished()), qApp, SLOT(quit()));

        plugin->setDevice(device);
        plugin->connectService();
    } else {
        qApp->quit();
    }
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

void BlueWizard::setManualPin(bool pinManual)
{
    m_manualPin = pinManual;
}

bool BlueWizard::manualPin() const
{
    return m_manualPin;
}

WizardAgent* BlueWizard::agent() const
{
    return m_agent;
}

KService::List BlueWizard::services() const
{
    return m_services;
}

void BlueWizard::setService(const KService* service)
{
    m_service = service;
}
