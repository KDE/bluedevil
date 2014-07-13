/***************************************************************************
 *   Copyright (C) 2010  Alex Fiestas <alex@eyeos.org>                     *
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

#include <QApplication>

#include <QBluez/Agent>

namespace QBluez {
    class Device;
}

class WizardAgent : public QBluez::Agent
{
    Q_OBJECT

public:
    WizardAgent(QObject* parent);
    ~WizardAgent();

    void setPin(const QString& pin);
    QString getPin(QBluez::Device* device);
    QString pin();
    bool isFromDatabase();

    QDBusObjectPath objectPath() const Q_DECL_OVERRIDE;

    void requestPinCode(QBluez::Device *device, const QBluez::Request<QString> &req) Q_DECL_OVERRIDE;
    void displayPinCode(QBluez::Device *device, const QString &pinCode) Q_DECL_OVERRIDE;
    void requestPasskey(QBluez::Device *device, const QBluez::Request<quint32> &req) Q_DECL_OVERRIDE;
    void displayPasskey(QBluez::Device *device, const QString &passkey, const QString &entered) Q_DECL_OVERRIDE;
    void requestConfirmation(QBluez::Device *device, const QString &passkey, const QBluez::Request<void> &req) Q_DECL_OVERRIDE;
    void requestAuthorization(QBluez::Device *device, const QBluez::Request<void> &req) Q_DECL_OVERRIDE;
    void authorizeService(QBluez::Device *device, const QString &uuid, const QBluez::Request<void> &req) Q_DECL_OVERRIDE;
    void cancel() Q_DECL_OVERRIDE;
    void release() Q_DECL_OVERRIDE;

private:
    bool m_fromDatabase;
    QString m_pin;

Q_SIGNALS:
    void pinRequested(const QString &pin);
    void confirmationRequested(const QString &passkey, const QBluez::Request<void> &req);
    void agentReleased();
};

#endif
