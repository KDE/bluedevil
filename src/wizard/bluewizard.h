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

#include <BluezQt/Manager>

class WizardAgent;

class BlueWizard : public QWizard
{
    Q_OBJECT

public:
    explicit BlueWizard();

    BluezQt::DevicePtr device() const;
    void setDevice(BluezQt::DevicePtr device);

    QString pin() const;
    void setPin(const QString &pin);

    WizardAgent *agent() const;
    BluezQt::Manager *manager() const;

    enum {
        Discover,
        LegacyPairing,
        LegacyPairingDatabase,
        KeyboardPairing,
        SSPPairing,
        Success,
        Fail,
        Connect
    };

private Q_SLOTS:
    void initJobResult(BluezQt::InitManagerJob *job);
    void operationalChanged(bool operational);

private:
    void done(int result) Q_DECL_OVERRIDE;

    BluezQt::Manager *m_manager;
    WizardAgent *m_agent;

    BluezQt::DevicePtr m_device;
    QString m_pin;
};

#endif // BLUEWIZARD_H
