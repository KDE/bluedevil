/***************************************************************************
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@eyeos.org>        *
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
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

#ifndef BLUEZAGENT_H
#define BLUEZAGENT_H

#include <BluezQt/Agent>

class QProcess;

class BluezAgent : public BluezQt::Agent
{
    Q_OBJECT

public:
    explicit BluezAgent(QObject *parent);

    QDBusObjectPath objectPath() const Q_DECL_OVERRIDE;

    void authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request) Q_DECL_OVERRIDE;
    void requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request) Q_DECL_OVERRIDE;
    void requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request) Q_DECL_OVERRIDE;
    void requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request) Q_DECL_OVERRIDE;
    void requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request) Q_DECL_OVERRIDE;
    void release() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void processClosedBool(int exitCode);
    void processClosedAuthorize(int exitCode);
    void processClosedPin(int exitCode);
    void processClosedPasskey(int exitCode);

Q_SIGNALS:
    void agentReleased();

private:
    QProcess *m_process;
    BluezQt::DevicePtr m_device;
    BluezQt::Request<> m_boolRequest;
    BluezQt::Request<quint32> m_passkeyRequest;
    BluezQt::Request<QString> m_pinRequest;
};

#endif // BLUEZAGENT_H
