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

#include <QtDBus/QDBusConnection>
#include <QtCore/QDebug>
#include <QtCore/QProcess>

#include <KStandardDirs>
#include <bluedevil/bluedevil.h>

#define AGENT_PATH "/blueDevil_agent"

BluezAgent::BluezAgent(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    if (!QDBusConnection::systemBus().registerObject(AGENT_PATH, parent)) {
        qDebug() << "The dbus object can't be registered";
        return;
    }

    m_adapter = BlueDevil::Manager::self()->defaultAdapter();
    m_adapter->registerAgent(AGENT_PATH, BlueDevil::Adapter::DisplayYesNo);

    m_process = new QProcess(this);

    qDebug() << "Agent registered";
}

void BluezAgent::unregister()
{
    qDebug() << "Unregistering object";
    BlueDevil::Adapter *const defaultAdapter = BlueDevil::Manager::self()->defaultAdapter();
    if (defaultAdapter) {
        defaultAdapter->unregisterAgent(AGENT_PATH);
    }
    QDBusConnection::systemBus().unregisterObject(AGENT_PATH);
    parent()->deleteLater();
}

void BluezAgent::Release()
{
    qDebug() << "Agent Release";
    emit agentReleased();
}

void BluezAgent::Authorize(const QDBusObjectPath &device, const QString& uuid, const QDBusMessage &msg)
{
    Q_UNUSED(uuid)
    qDebug() << "Authorize called";

    m_msg = msg;
    m_msg.setDelayedReply(true);
    m_currentHelper = "Authorize";

    BlueDevil::Device *remote = m_adapter->deviceForUBI(device.path());

    QStringList list;
    list.append(remote->name());
    list.append(device.path());

    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedBool(int)));
    m_process->start(KStandardDirs::findExe("bluedevil-authorize"), list);
}

QString BluezAgent::RequestPinCode(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    qDebug() << "AGENT-RequestPinCode " << device.path();
    m_msg = msg;
    m_msg.setDelayedReply(true);

    BlueDevil::Device *remote = m_adapter->deviceForUBI(device.path());

    QStringList list(remote->name());

    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedPin(int)));
    m_process->start(KStandardDirs::findExe("bluedevil-requestpin"), list);

    return QString();
}

quint32 BluezAgent::RequestPasskey(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    qDebug() << "AGENT-RequestPasskey " << device.path();

    m_msg = msg;
    m_msg.setDelayedReply(true);

    BlueDevil::Device *remote = m_adapter->deviceForUBI(device.path());

    QStringList list(remote->name());

    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedPasskey(int)));
    m_process->start(KStandardDirs::findExe("bluedevil-requestpin"), list);

    return 0;
}

void BluezAgent::DisplayPasskey(const QDBusObjectPath &device, quint32 passkey)
{
    qDebug() << "AGENT-DisplayPasskey " << device.path() << ", " << QString::number(passkey);
}

void BluezAgent::RequestConfirmation(const QDBusObjectPath &device, quint32 passkey, const QDBusMessage &msg)
{
    qDebug() << "AGENT-RequestConfirmation " << device.path() << ", " << QString::number(passkey);

    m_msg = msg;
    m_msg.setDelayedReply(true);
    m_currentHelper = "RequestConfirmation";

    BlueDevil::Device *remote = m_adapter->deviceForUBI(device.path());

    QStringList list;
    list.append(remote->name());
    list.append(QString::number(passkey));

    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedBool(int)));
    m_process->start(KStandardDirs::findExe("bluedevil-requestconfirmation"), list);
}

void BluezAgent::ConfirmModeChange(const QString& mode, const QDBusMessage &msg)
{
    qDebug() << "AGENT-ConfirmModechange " << mode;

    m_msg = msg;
    m_msg.setDelayedReply(true);
    m_currentHelper = "ConfirmModechange";

    QStringList list;
    list.append(mode);

    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedBool(int)));
    m_process->start(KStandardDirs::findExe("bluedevil-confirmchangemode"), list);
}

void BluezAgent::Cancel()
{
    qDebug() << "AGENT-Cancel";
}

void BluezAgent::processClosedBool(int exitCode)
{
    qDebug() << "ProcessClosed: " << exitCode;
    disconnect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedBool(int)));

    if (!exitCode) {
        qDebug() << "Sending empty reply";
        QDBusConnection::systemBus().send(m_msg.createReply());
        return;
    }

    qDebug() << "Sending error";
    sendBluezError(m_currentHelper, m_msg);
}

void BluezAgent::processClosedPin(int exitCode)
{
    qDebug() << "ProcessClosedPin: " << exitCode;
    disconnect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedPin(int)));

    if (!exitCode) {
        QVariant arg = QVariant::fromValue<QString>(m_process->readAllStandardOutput());
        QDBusMessage reply = m_msg.createReply(arg);
        QDBusConnection::systemBus().send(reply);
        return;
    }

    QDBusMessage error = m_msg.createErrorReply("org.bluez.Error.Canceled", "Pincode request failed");
    QDBusConnection::systemBus().send(error);
}

void BluezAgent::processClosedPasskey(int exitCode)
{
    disconnect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedPasskey(int)));

    if (!exitCode) {
        QVariant arg = QVariant::fromValue<quint32>(m_process->readAllStandardOutput().toInt());
        QDBusMessage reply = m_msg.createReply(arg);
        QDBusConnection::systemBus().send(reply);
        return;
    }

    QDBusMessage error = m_msg.createErrorReply("org.bluez.Error.Canceled", "Pincode request failed");
    QDBusConnection::systemBus().send(error);
}

void BluezAgent::sendBluezError(const QString &helper, const QDBusMessage &msg)
{
    qDebug() << "Sending canceled msg to bluetooth" << helper;
    QDBusMessage error = msg.createErrorReply("org.bluez.Error.Canceled", "Authorization canceled");
    QDBusConnection::systemBus().send(error);
}