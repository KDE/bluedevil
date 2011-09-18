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
#include <kservice.h>

class WizardAgent;
class BlueWizard : public QWizard
{
Q_OBJECT

public:
    BlueWizard(const KUrl& url);
    virtual ~BlueWizard();

    QByteArray deviceAddress() const;
    void setDeviceAddress(const QByteArray& address);

    QByteArray pin() const;
    void setPin(const QByteArray& pin);

    QByteArray preselectedUuid() const;
    void setPreselectedUuid(const QByteArray &uuid);

    QByteArray preselectedAddress() const;
    void setPreselectedAddress(const QByteArray &uuid);

    WizardAgent* agent() const;

    KService::List services() const;

    void setService(const KService *);
    enum {Discover, Services, NoPairing, LegacyPairing, LegacyPairingDatabase, KeyboardPairing, SSPPairing, Fail};

public Q_SLOTS:
    void restartWizard();
    void setPin(const QString& pin);
    virtual void done(int result);

private:
    QByteArray m_deviceAddress;
    QByteArray m_pin;
    QByteArray m_preselectedUuid;
    QByteArray m_preselectedAddress;
    WizardAgent *m_agent;
    KService::List m_services;
    const KService *m_service;

    bool m_manualPin;
};

#endif // BLUEWIZARD_H
