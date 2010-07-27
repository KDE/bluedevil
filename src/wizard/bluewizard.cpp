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
#include "../actionplugins/actionplugin.h"

#include <QApplication>
#include <QDBusConnection>
#include <QWizard>

#include <bluedevil/bluedevil.h>
#include <KServiceTypeTrader>
#include <KPushButton>
#include <kdebug.h>

BlueWizard::BlueWizard(const KUrl &url) : QWizard(), m_service(0), m_manualPin(false)
{
    setWindowTitle(i18n("BlueDevil Remote Device Wizard"));

    if (url.host().length() != 17) {
        setPage(Introduction, new IntroductionPage(this));
        setPage(Discover, new DiscoverPage(this));
    } else {
        setDeviceAddress(url.host().replace("-", ":").toLatin1());
    }

    if (url.fileName().length() == 36) {
        setPreselectedUuid(url.fileName().toLatin1());
    }
    setPage(Pin, new PinPage(this));
    setPage(Pairing, new PairingPage(this));
    setPage(Services, new ServicesPage(this));

    setButton(QWizard::BackButton, new KPushButton(KStandardGuiItem::back(KStandardGuiItem::UseRTL)));
    setButton(QWizard::NextButton, new KPushButton(KStandardGuiItem::forward(KStandardGuiItem::UseRTL)));
    setButton(QWizard::CancelButton, new KPushButton(KStandardGuiItem::cancel()));

    //We do not want "Forward" as text
    setButtonText(QWizard::NextButton, i18n("Next"));
    //First show, then do the rest
    show();

    if(!QDBusConnection::systemBus().registerObject("/wizardAgent", qApp)) {
        qDebug() << "The dbus object can't be registered";
    }

    m_agent = new WizardAgent(qApp);
    m_services = KServiceTypeTrader::self()->query("BlueDevil/ActionPlugin");
    kDebug() << "Num of found services: " << m_services.count();
}

void BlueWizard::done(int result)
{
    kDebug() << "Wizard done: " << result;

    QWizard::done(result);
    if (result == 1) {
        //If we have a service to connect with
        if (m_service) {
            kDebug() << "Connecting to: " << m_service->name();
            KPluginFactory *factory = KPluginLoader(m_service->library()).factory();

            Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_deviceAddress);
            ActionPlugin *plugin = factory->create<ActionPlugin>(this);
            connect(plugin, SIGNAL(finished()), qApp, SLOT(quit()));

            plugin->setDevice(device);
            plugin->startAction();
        } else {
            qApp->quit();
        }
    } else {
        qApp->quit();
    }
}

BlueWizard::~BlueWizard()
{

}

void BlueWizard::setDeviceAddress(const QByteArray& address)
{
    kDebug() << "Device AddresS: " << address;
    m_deviceAddress = address;
}

QByteArray BlueWizard::deviceAddress() const
{
    return m_deviceAddress;
}

void BlueWizard::setPin(const QByteArray& pinNum)
{
    kDebug() << "Setting pin: :" << pinNum;
    m_pin = pinNum;
}

QByteArray BlueWizard::pin() const
{
    return m_pin;
}

void BlueWizard::setManualPin(bool pinManual)
{
    kDebug() << "Manual pin: " << pinManual;
    m_manualPin = pinManual;
}

bool BlueWizard::manualPin() const
{
    return m_manualPin;
}

void BlueWizard::setPreselectedUuid(const QByteArray& uuid)
{
    kDebug() << "Preselect UUID: " << uuid;
    m_preselectedUuid = uuid;
}

QByteArray BlueWizard::preselectedUuid() const
{
    return m_preselectedUuid;
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
    kDebug() << "Setting service: " << (service ? service->name() : "none");
    m_service = service;
}
