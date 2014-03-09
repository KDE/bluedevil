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
#include "pages/discoverpage.h"
#include "pages/nopairing.h"
#include "pages/legacypairing.h"
#include "pages/legacypairingdatabase.h"
#include "pages/keyboardpairing.h"
#include "pages/ssppairing.h"
#include "pages/fail.h"

#include <QApplication>
#include <QDBusConnection>
#include <QWizard>
#include <QString>

#include <bluedevil/bluedevil.h>
#include <KPushButton>
#include <kdebug.h>
#include <KProcess>

BlueWizard::BlueWizard(const KUrl &url) : QWizard(), m_device(0), m_manualPin(false)
{
    setWindowTitle(i18n("Bluetooth Device Wizard"));

    setOption(QWizard::IndependentPages, true);

    if (url.host().length() == 17) {
        setPreselectedAddress(url.host().replace("-", ":").toLatin1());
    }

    if (url.fileName().length() == 36) {
        setPreselectedUuid(url.fileName().toLatin1());
    }

    setPage(Discover, new DiscoverPage(this));
    setPage(NoPairing, new NoPairingPage(this));
    setPage(Connect, new NoPairingPage(this));
    setPage(LegacyPairing, new LegacyPairingPage(this));
    setPage(LegacyPairingDatabase, new LegacyPairingPageDatabase(this));
    setPage(KeyboardPairing, new KeyboardPairingPage(this));
    setPage(SSPPairing, new SSPPairingPage(this));
    setPage(Fail, new FailPage(this));

    setButton(QWizard::BackButton, new KPushButton(KStandardGuiItem::back(KStandardGuiItem::UseRTL)));
    setButton(QWizard::NextButton, new KPushButton(KStandardGuiItem::forward(KStandardGuiItem::UseRTL)));
    setButton(QWizard::FinishButton, new KPushButton(KStandardGuiItem::apply()));
    setButton(QWizard::CancelButton, new KPushButton(KStandardGuiItem::cancel()));

    //We do not want "Forward" as text
    setButtonText(QWizard::NextButton, i18nc("Action to go to the next page on the wizard", "Next"));
    setButtonText(QWizard::FinishButton, i18nc("Action to finish the wizard", "Finish"));
    //First show, then do the rest
    show();

    if(!QDBusConnection::systemBus().registerObject("/wizardAgent", qApp)) {
        qDebug() << "The dbus object can't be registered";
    }

    m_agent = new WizardAgent(qApp);
}

void BlueWizard::done(int result)
{
    kDebug() << "Wizard done: " << result;

    QWizard::done(result);
    qApp->exit(result);
}

Device* BlueWizard::device() const
{
    return m_device;
}

BlueWizard::~BlueWizard()
{

}

void BlueWizard::setDeviceAddress(const QByteArray& address)
{
    kDebug() << "Device Address: " << address;
    if (!Manager::self()->usableAdapter()) {
        kDebug() << "No usable adapter available";
        return;
    }

    m_deviceAddress = address;
    m_device = Manager::self()->usableAdapter()->deviceForAddress(m_deviceAddress);
}

QByteArray BlueWizard::deviceAddress() const
{
    return m_deviceAddress;
}

void BlueWizard::restartWizard()
{
    KProcess proc;
    proc.setProgram("bluedevil-wizard");
    proc.startDetached();

    qApp->quit();
}

void BlueWizard::setPin(const QByteArray& pinNum)
{
    kDebug() << "Setting pin: :" << pinNum;
    m_pin = pinNum;
}

void BlueWizard::setPin(const QString& pin)
{
    setPin(pin.toAscii());
}

QByteArray BlueWizard::pin() const
{
    return m_pin;
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

void BlueWizard::setPreselectedAddress(const QByteArray& address)
{
    kDebug() << "Preselected Address: " << address;
    m_preselectedAddress = address;
}

QByteArray BlueWizard::preselectedAddress() const
{
    return m_preselectedAddress;
}


WizardAgent* BlueWizard::agent() const
{
    return m_agent;
}
