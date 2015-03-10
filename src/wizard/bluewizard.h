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

#ifndef BLUEWIZARD_H
#define BLUEWIZARD_H

#include <QObject>
#include <QWizard>

namespace BlueDevil
{
    class Device;
}

class WizardAgent;

class BlueWizard : public QWizard
{
    Q_OBJECT

public:
    explicit BlueWizard(const QUrl &url);

    QString deviceAddress() const;
    void setDeviceAddress(const QString &address);

    BlueDevil::Device *device() const;

    QString pin() const;
    void setPin(const QString &pin);

    QString preselectedUuid() const;
    void setPreselectedUuid(const QString &uuid);

    QString preselectedAddress() const;
    void setPreselectedAddress(const QString &uuid);

    WizardAgent *agent() const;

    enum {
        Discover,
        NoPairing,
        LegacyPairing,
        LegacyPairingDatabase,
        KeyboardPairing,
        SSPPairing,
        Success,
        Fail,
        Connect
    };

public Q_SLOTS:
    void restartWizard();
    void done(int result) Q_DECL_OVERRIDE;

private:
    BlueDevil::Device *m_device;
    WizardAgent *m_agent;

    QString m_pin;
    QString m_deviceAddress;
    QString m_preselectedUuid;
    QString m_preselectedAddress;
    bool m_manualPin;
};

#endif // BLUEWIZARD_H
