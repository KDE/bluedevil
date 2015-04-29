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
#include "pages/discover.h"
#include "pages/connect.h"
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
#include <QProcess>
#include <QString>

#include <KStandardGuiItem>
#include <KLocalizedString>

#include <BluezQt/InitManagerJob>

BlueWizard::BlueWizard()
    : QWizard()
    , m_agent(new WizardAgent(this))
{
    setWindowTitle(i18n("Bluetooth Device Wizard"));

    setOption(QWizard::IndependentPages, true);

    setPage(Discover, new DiscoverPage(this));
    setPage(Connect, new ConnectPage(this));
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

    // Initialize BluezQt
    m_manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *initJob = m_manager->init();
    initJob->start();
    connect(initJob, &BluezQt::InitManagerJob::result, this, &BlueWizard::initJobResult);
}

BluezQt::DevicePtr BlueWizard::device() const
{
    return m_device;
}

void BlueWizard::setDevice(BluezQt::DevicePtr device)
{
    m_device = device;
}

void BlueWizard::setPin(const QString &pin)
{
    qCDebug(WIZARD) << "Setting pin:" << pin;

    m_pin = pin;
}

QString BlueWizard::pin() const
{
    return m_pin;
}

WizardAgent *BlueWizard::agent() const
{
    return m_agent;
}

BluezQt::Manager *BlueWizard::manager() const
{
    return m_manager;
}

void BlueWizard::initJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error()) {
        qCWarning(WIZARD) << "Error initializing manager:" << job->errorText();
        qApp->exit(1);
        return;
    }

    qCDebug(WIZARD) << "Manager initialized";

    // Make sure to register agent when bluetoothd starts
    operationalChanged(m_manager->isOperational());
    connect(m_manager, &BluezQt::Manager::operationalChanged, this, &BlueWizard::operationalChanged);

    // Only show wizard after init is completed
    show();
}

void BlueWizard::operationalChanged(bool operational)
{
    if (operational) {
        m_manager->registerAgent(m_agent);
    } else {
        // Attempt to start bluetoothd
        BluezQt::Manager::startService();
    }
}

void BlueWizard::done(int result)
{
    qCDebug(WIZARD) << "Wizard done: " << result;

    QWizard::done(result);
    qApp->exit(result);
}
