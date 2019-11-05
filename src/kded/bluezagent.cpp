/***************************************************************************
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@eyeos.org>        *
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
 *   Copyright (C) 2010 UFO Coders <info@ufocoders.com>                    *
 *   Copyright (C) 2014-2015 David Rosca <nowrep@gmail.com>                *
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

#include "bluezagent.h"
#include "debug_p.h"
#include "helpers/requestauthorization.h"
#include "helpers/requestconfirmation.h"
#include "helpers/requestpin.h"

#include <QStandardPaths>
#include <QDBusObjectPath>

#include <BluezQt/Device>

BluezAgent::BluezAgent(QObject *parent)
    : BluezQt::Agent(parent)
{
}

QDBusObjectPath BluezAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/modules/bluedevil/Agent"));
}

static void processAuthorizationRequest(BluezQt::DevicePtr device, const BluezQt::Request<> &request, RequestAuthorization::Result result)
{
    switch (result) {
    case RequestAuthorization::Accept:
        qCDebug(BLUEDAEMON) << "Accepting request";
        request.accept();
        break;

    case RequestAuthorization::AcceptAndTrust:
        qCDebug(BLUEDAEMON) << "Accepting request and trusting device";
        request.accept();
        device->setTrusted(true);
        break;

    default:
        qCDebug(BLUEDAEMON) << "Rejecting request";
        request.reject();
        break;
    }
}

void BluezAgent::authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request)
{
    // TODO: Show user the Service UUID
    qCDebug(BLUEDAEMON) << "AGENT-AuthorizeService" << device->name() << "Service:" << uuid;

    RequestAuthorization *helper = new RequestAuthorization(device, this);
    connect(helper, &RequestAuthorization::done, this, [ device, request](RequestAuthorization::Result result) {
        processAuthorizationRequest(device, request, result);
    });
}

void BluezAgent::requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request)
{
    qCDebug(BLUEDAEMON) << "AGENT-RequestPinCode " << device->name();

    RequestPin *helper = new RequestPin(device, false, this);
    connect(helper, &RequestPin::done, this, [ request](const QString &result) {
        if (!result.isEmpty()) {
            qCDebug(BLUEDAEMON) << "Introducing PIN...";
            request.accept(result);
            return;
        }

        qCDebug(BLUEDAEMON) << "No PIN introduced";
        request.reject();
    });
}

void BluezAgent::requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request)
{
    qCDebug(BLUEDAEMON) << "AGENT-RequestPasskey " << device->name();

    RequestPin *helper = new RequestPin(device, true, this);
    connect(helper, &RequestPin::done, this, [ request](const QString &result) {
        bool ok;
        quint32 passkey = result.toInt(&ok);
        if (ok) {
            qCDebug(BLUEDAEMON) << "Introducing PassKey...";
            request.accept(passkey);
            return;
        }

        qCDebug(BLUEDAEMON) << "No PassKey introduced";
        request.reject();
    });
}

void BluezAgent::requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request)
{
    qCDebug(BLUEDAEMON) << "AGENT-RequestConfirmation " << device->name() << passkey;

    RequestConfirmation *helper = new RequestConfirmation(device, passkey, this);
    connect(helper, &RequestConfirmation::done, this, [ request](RequestConfirmation::Result result) {
        if (result == RequestConfirmation::Accept) {
            qCDebug(BLUEDAEMON) << "Accepting request";
            request.accept();
            return;
        }

        qCDebug(BLUEDAEMON) << "Rejecting request";
        request.reject();
    });
}

void BluezAgent::requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request)
{
    qCDebug(BLUEDAEMON) << "AGENT-RequestAuthorization";

    RequestAuthorization *helper = new RequestAuthorization(device, this);
    connect(helper, &RequestAuthorization::done, this, [ device, request](RequestAuthorization::Result result) {
        processAuthorizationRequest(device, request, result);
    });
}

void BluezAgent::release()
{
    qCDebug(BLUEDAEMON) << "AGENT-Release";

    Q_EMIT agentReleased();
}

void BluezAgent::cancel()
{
    qCDebug(BLUEDAEMON) << "AGENT-Cancel";

    Q_EMIT agentCanceled();
}
