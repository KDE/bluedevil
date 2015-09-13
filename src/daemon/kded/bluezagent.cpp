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

#include "bluezagent.h"
#include "config.h"
#include "debug_p.h"

#include <QProcess>
#include <QDBusObjectPath>
#include <QStandardPaths>

#include <BluezQt/Device>

BluezAgent::BluezAgent(QObject *parent)
    : BluezQt::Agent(parent)
{
    m_process = new QProcess(this);
}

QDBusObjectPath BluezAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/BlueDevilAgent"));
}

void BluezAgent::authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request)
{
    Q_UNUSED(uuid)

    qCDebug(BLUEDAEMON) << "AGENT-AuthorizeService";

    m_device = device;
    m_boolRequest = request;

    QStringList list;
    list.append(device->name());

    const QString &exe = QStandardPaths::findExecutable(QStringLiteral("bluedevil-authorize"),
                                                        QStringList(HELPER_INSTALL_PATH));

    m_process->start(exe, list);
    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedAuthorize(int)));
}

void BluezAgent::requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request)
{
    qCDebug(BLUEDAEMON) << "AGENT-RequestPinCode " << device->name();

    m_pinRequest = request;

    QStringList list(device->name());

    const QString &exe = QStandardPaths::findExecutable(QStringLiteral("bluedevil-requestpin"),
                                                        QStringList(HELPER_INSTALL_PATH));

    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedPin(int)));
    m_process->start(exe, list);
}

void BluezAgent::requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request)
{
    qCDebug(BLUEDAEMON) << "AGENT-RequestPasskey " << device->name();

    m_passkeyRequest = request;

    QStringList list(device->name());
    list << QStringLiteral("numeric");

    const QString &exe = QStandardPaths::findExecutable(QStringLiteral("bluedevil-requestpin"),
                                                        QStringList(HELPER_INSTALL_PATH));

    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedPasskey(int)));
    m_process->start(exe, list);
}

void BluezAgent::requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request)
{
    qCDebug(BLUEDAEMON) << "AGENT-RequestConfirmation " << device->name() << passkey;

    m_boolRequest = request;

    QStringList list;
    list.append(device->name());
    list.append(passkey);

    const QString &exe = QStandardPaths::findExecutable(QStringLiteral("bluedevil-requestconfirmation"),
                                                        QStringList(HELPER_INSTALL_PATH));

    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedBool(int)));
    m_process->start(exe, list);
}

void BluezAgent::requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request)
{
    qCDebug(BLUEDAEMON) << "AGENT-RequestAuthorization";

    m_device = device;
    m_boolRequest = request;

    QStringList list;
    list.append(device->name());

    const QString &exe = QStandardPaths::findExecutable(QStringLiteral("bluedevil-authorize"),
                                                        QStringList(HELPER_INSTALL_PATH));

    m_process->start(exe, list);
    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedAuthorize(int)));
}

void BluezAgent::release()
{
    qCDebug(BLUEDAEMON) << "AGENT-Release";

    Q_EMIT agentReleased();
}

void BluezAgent::processClosedBool(int exitCode)
{
    qCDebug(BLUEDAEMON) << "ProcessClosed: " << exitCode;
    disconnect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedBool(int)));

    if (exitCode == 0) {
        qCDebug(BLUEDAEMON) << "Accepting request";
        m_boolRequest.accept();
        return;
    }

    qCDebug(BLUEDAEMON) << "Rejecting request";
    m_boolRequest.reject();
}

void BluezAgent::processClosedAuthorize(int exitCode)
{
    qCDebug(BLUEDAEMON) << "ProcessClosedAuthorize: " << exitCode;
    disconnect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedBool(int)));

    switch (exitCode) {
    case 0:
        qCDebug(BLUEDAEMON) << "Accepting request";
        m_boolRequest.accept();
        break;

    case 1:
        qCDebug(BLUEDAEMON) << "Accepting request and trusting device";
        m_boolRequest.accept();
        m_device->setTrusted(true);
        break;

    default:
        qCDebug(BLUEDAEMON) << "Rejecting request";
        m_boolRequest.reject();
        break;
    }

    m_device.clear();
}

void BluezAgent::processClosedPin(int exitCode)
{
    qCDebug(BLUEDAEMON) << "ProcessClosedPin: " << exitCode;
    disconnect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedPin(int)));

    if (exitCode == 0) {
        qCDebug(BLUEDAEMON) << "Accepting request";
        const QString &arg = QString::fromLocal8Bit(m_process->readAllStandardOutput());
        m_pinRequest.accept(arg);
        return;
    }

    qCDebug(BLUEDAEMON) << "Rejecting request";
    m_pinRequest.reject();
}

void BluezAgent::processClosedPasskey(int exitCode)
{
    qCDebug(BLUEDAEMON) << "ProcessClosedPasskey: " << exitCode;
    disconnect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedPasskey(int)));

    if (exitCode == 0) {
        qCDebug(BLUEDAEMON) << "Accepting request";
        quint32 arg = m_process->readAllStandardOutput().toInt();
        m_passkeyRequest.accept(arg);
        return;
    }

    qCDebug(BLUEDAEMON) << "Rejecting request";
    m_passkeyRequest.reject();
}
