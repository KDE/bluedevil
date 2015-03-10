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
#include "pages/success.h"
#include "pages/fail.h"
#include "debug_p.h"

#include <QApplication>
#include <QDBusConnection>
#include <QPushButton>
#include <QString>

#include <KStandardGuiItem>
#include <KLocalizedString>
#include <KProcess>

#include <bluedevil/bluedevil.h>

BlueWizard::BlueWizard(const QUrl &url)
    : QWizard()
    , m_device(0)
    , m_manualPin(false)
{
    setWindowTitle(i18n("Bluetooth Device Wizard"));

    setOption(QWizard::IndependentPages, true);

    if (url.host().length() == 17) {
        setPreselectedAddress(url.host().replace(QLatin1Char('-'), QLatin1Char(':')).toLatin1());
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
    setPage(Success, new SuccessPage(this));
    setPage(Fail, new FailPage(this));

    QPushButton *backButton = new QPushButton(this);
    KGuiItem::assign(backButton, KStandardGuiItem::back(KStandardGuiItem::UseRTL));

    QPushButton *forwardButton = new QPushButton(this);
    KGuiItem::assign(forwardButton, KStandardGuiItem::forward(KStandardGuiItem::UseRTL));

    QPushButton *applyButton = new QPushButton(this);
    KGuiItem::assign(applyButton, KStandardGuiItem::apply());

    QPushButton *cancelButton = new QPushButton(this);
    KGuiItem::assign(cancelButton, KStandardGuiItem::cancel());

    setButton(QWizard::BackButton, backButton);
    setButton(QWizard::NextButton, forwardButton);
    setButton(QWizard::FinishButton, applyButton);
    setButton(QWizard::CancelButton, cancelButton);

    // We do not want "Forward" as text
    setButtonText(QWizard::NextButton, i18nc("Action to go to the next page on the wizard", "Next"));
    setButtonText(QWizard::FinishButton, i18nc("Action to finish the wizard", "Finish"));

    // First show, then do the rest
    show();

    if (!QDBusConnection::systemBus().registerObject(QStringLiteral("/wizardAgent"), qApp)) {
        qCDebug(WIZARD) << "The dbus object can't be registered";
    }

    m_agent = new WizardAgent(qApp);
}

BlueWizard::~BlueWizard()
{
}

void BlueWizard::done(int result)
{
    qCDebug(WIZARD) << "Wizard done: " << result;

    QWizard::done(result);
    qApp->exit(result);
}

Device* BlueWizard::device() const
{
    return m_device;
}

void BlueWizard::setDeviceAddress(const QByteArray& address)
{
    qCDebug(WIZARD) << "Device Address: " << address;
    if (!Manager::self()->usableAdapter()) {
        qCDebug(WIZARD) << "No usable adapter available";
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
    proc.setProgram(QStringLiteral("bluedevil-wizard"));
    proc.startDetached();

    qApp->quit();
}

void BlueWizard::setPin(const QByteArray& pinNum)
{
    qCDebug(WIZARD) << "Setting pin: :" << pinNum;
    m_pin = pinNum;
}

void BlueWizard::setPin(const QString& pin)
{
    setPin(pin.toUtf8());
}

QByteArray BlueWizard::pin() const
{
    return m_pin;
}

void BlueWizard::setPreselectedUuid(const QByteArray& uuid)
{
    qCDebug(WIZARD) << "Preselect UUID: " << uuid;
    m_preselectedUuid = uuid;
}

QByteArray BlueWizard::preselectedUuid() const
{
    return m_preselectedUuid;
}

void BlueWizard::setPreselectedAddress(const QByteArray& address)
{
    qCDebug(WIZARD) << "Preselected Address: " << address;
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
