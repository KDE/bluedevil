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

#include "agentlistenerworker.h"

#include <QtDBus/QDBusConnection>
#include <QtCore/QDebug>

#include <KProcess>
#include <KStandardDirs>
#include <bluedevil/bluedevil.h>

#define AGENT_PATH "/blueDevil_agent"

AgentListenerWorker::AgentListenerWorker(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    if (!QDBusConnection::systemBus().registerObject(AGENT_PATH, parent)) {
        qDebug() << "The dbus object can't be registered";
        return;
    }

    m_adapter = BlueDevil::Manager::self()->defaultAdapter();
    m_adapter->registerAgent(AGENT_PATH, BlueDevil::Adapter::DisplayYesNo);

    qDebug() << "Agent registered";
}

void AgentListenerWorker::unregister()
{
    qDebug() << "Unregistering object";
    BlueDevil::Adapter *const defaultAdapter = BlueDevil::Manager::self()->defaultAdapter();
    if (defaultAdapter) {
        defaultAdapter->unregisterAgent(AGENT_PATH);
    }
}

void AgentListenerWorker::Release()
{
    qDebug() << "Agent Release";
    emit agentReleased();
}

void AgentListenerWorker::Authorize(const QDBusObjectPath &device, const QString& uuid, const QDBusMessage &msg)
{
    Q_UNUSED(uuid)
    qDebug() << "Authorize called";

    BlueDevil::Device *remote = m_adapter->deviceForUBI(device.path());

    QStringList list;
    list.append(remote->name());
    list.append(device.path());

    int result = KProcess::execute(KStandardDirs::findExe("bluedevil-authorize"), list);
    if (result == 0) {
        qDebug() << "Correct";
        return;
    }
    sendBluezError(QString("Authorize"),msg);
}

QString AgentListenerWorker::RequestPinCode(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    qDebug() << "AGENT-RequestPinCode " << device.path();
    BlueDevil::Device *remote = m_adapter->deviceForUBI(device.path());

    QStringList list(remote->name());

    KProcess process;
    process.setOutputChannelMode(KProcess::OnlyStdoutChannel);
    process.setProgram(KStandardDirs::findExe("bluedevil-requestpin"),list);
    process.start();

    if (process.waitForFinished(-1)) {
        return QString(process.readAllStandardOutput());
    }

    qDebug() << "Timeout!";
    QDBusMessage error = msg.createErrorReply("org.bluez.Error.Canceled", "Pincode request failed");
    QDBusConnection::systemBus().send(error);
    return QString();
}

quint32 AgentListenerWorker::RequestPasskey(const QDBusObjectPath &device, const QDBusMessage &msg)
{
    qDebug() << "AGENT-RequestPasskey " << device.path();
    QString ret = RequestPinCode(device, msg);
    return ret.toInt();
}

void AgentListenerWorker::DisplayPasskey(const QDBusObjectPath &device, quint32 passkey)
{
    qDebug() << "AGENT-DisplayPasskey " << device.path() << ", " << QString::number(passkey);
}

void AgentListenerWorker::RequestConfirmation(const QDBusObjectPath &device, quint32 passkey, const QDBusMessage &msg)
{
    qDebug() << "AGENT-RequestConfirmation " << device.path() << ", " << QString::number(passkey);

    BlueDevil::Device *remote = m_adapter->deviceForUBI(device.path());

    QStringList list;
    list.append(remote->name());
    list.append(QString::number(passkey));

    int result = KProcess::execute(KStandardDirs::findExe("bluedevil-requestconfirmation"), list);
    if (result == 0) {
        qDebug() << "Go on camarada!";
        QDBusConnection::systemBus().send(msg.createReply());
        return;
    }
    sendBluezError(QString("RequestConfirmation"),msg);
}

void AgentListenerWorker::ConfirmModeChange(const QString& mode, const QDBusMessage &msg)
{
    qDebug() << "AGENT-ConfirmModechange " << mode;
    QStringList list;
    list.append(mode);

    int result = KProcess::execute(KStandardDirs::findExe("bluedevil-confirmchangemode"),list);
    if (result == 0) {
        qDebug() << "Go on camarada!";
        return;
    }
    sendBluezError(QString("ConfirmModechange"),msg);
}

void AgentListenerWorker::Cancel()
{
    qDebug() << "AGENT-Cancel";
}


void AgentListenerWorker::sendBluezError(const QString &helper, const QDBusMessage &msg)
{
    qDebug() << "Sending canceled msg to bluetooth" << helper;
    QDBusMessage error = msg.createErrorReply("org.bluez.Error.Canceled", "Authorization canceled");
    QDBusConnection::systemBus().send(error);
}