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
#include <KLocalizedString>
#include <bluedevil/bluedevil.h>

#define AGENT_PATH "/blueDevil_agent"

BluezAgent::BluezAgent(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    if (!QDBusConnection::systemBus().registerObject(AGENT_PATH, parent)) {
        qDebug() << "The dbus object can't be registered";
        return;
    }

    BlueDevil::Manager::self()->registerAgent(AGENT_PATH, BlueDevil::Manager::DisplayYesNo);
    BlueDevil::Manager::self()->requestDefaultAgent(AGENT_PATH);

    m_process = new QProcess(this);

    qDebug() << "Agent registered";
}

void BluezAgent::unregister()
{
    qDebug() << "Unregistering object";
    BlueDevil::Manager::self()->unregisterAgent(AGENT_PATH);
    QDBusConnection::systemBus().unregisterObject(AGENT_PATH);
    parent()->deleteLater();
}

void BluezAgent::Release()
{
    qDebug() << "Agent Release";
    emit agentReleased();
}

void BluezAgent::AuthorizeService(const QDBusObjectPath &device, const QString& uuid, const QDBusMessage &msg)
{
    Q_UNUSED(uuid)
    qDebug() << "Authorize called";

    m_msg = msg;
    m_msg.setDelayedReply(true);
    m_currentHelper = "Authorize";

    QStringList list;
    list.append(deviceName(device.path()));
    list.append(device.path());

    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedBool(int)));
    m_process->start(KStandardDirs::findExe("bluedevil-authorize"), list);
}

QString BluezAgent::RequestPinCode(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    qDebug() << "AGENT-RequestPinCode " << device.path();
    m_msg = msg;
    m_msg.setDelayedReply(true);

    QStringList list(deviceName(device.path()));
    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedPin(int)));
    m_process->start(KStandardDirs::findExe("bluedevil-requestpin"), list);

    return QString();
}

quint32 BluezAgent::RequestPasskey(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    qDebug() << "AGENT-RequestPasskey " << device.path();

    m_msg = msg;
    m_msg.setDelayedReply(true);

    QStringList list(deviceName(device.path()));
    list << QLatin1String("numeric");

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

    QStringList list;
    list.append(deviceName(device.path()));
    list.append(QString("%1").arg(passkey, 6, 10, QLatin1Char('0')));

    connect(m_process, SIGNAL(finished(int)), this, SLOT(processClosedBool(int)));
    m_process->start(KStandardDirs::findExe("bluedevil-requestconfirmation"), list);
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

QString BluezAgent::deviceName(const QString& UBI)
{
    BlueDevil::Device *device = BlueDevil::Manager::self()->deviceForUBI(UBI);
    if (!device || device->name().isEmpty()) {
        return i18nc("User will see this as: Bluetooth device is asking if the pin is correct\
        It is mostly a fallback", "Bluetooth device");
    }

    return device->name();
}