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

#include <kurl.h>

namespace BlueDevil {
    class Device;
}

class WizardAgent;
class BlueWizard : public QWizard
{
Q_OBJECT

public:
    BlueWizard(const KUrl& url);
    virtual ~BlueWizard();

    QByteArray deviceAddress() const;
    void setDeviceAddress(const QByteArray& address);

    BlueDevil::Device *device() const;

    QByteArray pin() const;
    void setPin(const QByteArray& pin);

    QByteArray preselectedUuid() const;
    void setPreselectedUuid(const QByteArray &uuid);

    QByteArray preselectedAddress() const;
    void setPreselectedAddress(const QByteArray &uuid);

    WizardAgent* agent() const;

    enum {Discover, NoPairing, LegacyPairing, LegacyPairingDatabase, KeyboardPairing, SSPPairing, Fail, Connect};

public Q_SLOTS:
    void restartWizard();
    void setPin(const QString& pin);
    virtual void done(int result);

private:
    QByteArray m_deviceAddress;
    BlueDevil::Device * m_device;
    QByteArray m_pin;
    QByteArray m_preselectedUuid;
    QByteArray m_preselectedAddress;
    WizardAgent *m_agent;

    bool m_manualPin;
};

#endif // BLUEWIZARD_H
