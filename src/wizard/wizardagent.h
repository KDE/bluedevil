/***************************************************************************
 *   Copyright (C) 2010 Alex Fiestas <alex@eyeos.org>                      *
 *   Copyright (C) 2010 UFO Coders <info@ufocoders.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef WIZARDAGENT_H
#define WIZARDAGENT_H

#include <BluezQt/Agent>

class WizardAgent : public BluezQt::Agent
{
    Q_OBJECT

public:
    explicit WizardAgent(QObject *parent = 0);

    QString pin();
    void setPin(const QString &pin);

    bool isFromDatabase();
    QString getPin(BluezQt::DevicePtr device);

    QDBusObjectPath objectPath() const override;

    void requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &req) override;
    void displayPinCode(BluezQt::DevicePtr device, const QString &pinCode) override;
    void requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &req) override;
    void displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered) override;
    void requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &req) override;

Q_SIGNALS:
    void pinRequested(const QString &pin);
    void confirmationRequested(const QString &passkey, const BluezQt::Request<> &req);

private:
    bool m_fromDatabase;
    QString m_pin;
};

#endif // WIZARDAGENT_H
