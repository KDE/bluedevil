/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#include "agentlistenerworker.h"
#include <QDBusConnection>
#include <QDebug>
#include <KProcess>
#include <solid/control/bluetoothmanager.h>

AgentListenerWorker::AgentListenerWorker(QObject *app) :  QDBusAbstractAdaptor(app)
{
    const QString agentPath = "/blueDevil_agent";

    if(!QDBusConnection::systemBus().registerObject(agentPath, app)){
        qDebug() << "The dbus object can't be registered";
        return;
    }

    Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();
    m_adapter = new Solid::Control::BluetoothInterface(man.defaultInterface());
    m_adapter->registerAgent(agentPath, "DisplayYesNo");
    qDebug() << "Agent registered";
}

AgentListenerWorker::~AgentListenerWorker()
{
}

void AgentListenerWorker::Release()
{
    qDebug() << "Agent Release";
    emit agentReleased();
}

void AgentListenerWorker::Authorize(QDBusObjectPath device, const QString& uuid, const QDBusMessage &msg)
{
    Q_UNUSED(uuid)
    qDebug() << "Authorize called";

    Solid::Control::BluetoothRemoteDevice *remote = m_adapter->findBluetoothRemoteDeviceUBI(device.path());

    QStringList list;
    list.append(remote->name());
    list.append(device.path());

    int result = KProcess::execute("bluedevil-authorize",list);
    if (result == 0) {
        qDebug() << "Go on camarada!";
        return;
    }
    qDebug() << "Sending Authorization cancelled";
    QDBusMessage error = msg.createErrorReply("org.bluez.Error.Canceled", "Authorization canceled");
    QDBusConnection::systemBus().send(error);
}

QString AgentListenerWorker::RequestPinCode(QDBusObjectPath device, const QDBusMessage &msg)
{
    qDebug() << "AGENT-RequestPinCode " << device.path();
    Solid::Control::BluetoothRemoteDevice *remote = m_adapter->findBluetoothRemoteDeviceUBI(device.path());

    QStringList list(remote->name());

    KProcess process;
    process.setOutputChannelMode(KProcess::OnlyStdoutChannel);
    process.setProgram("bluedevil-requestpin",list);
    process.start();

    if (process.waitForFinished()) {
        return QString(process.readAllStandardOutput());
    }

    QDBusMessage error = msg.createErrorReply("org.bluez.Error.Canceled", "Pincode request failed");
    QDBusConnection::systemBus().send(error);
    return QString();
}

quint32 AgentListenerWorker::RequestPasskey(QDBusObjectPath device, const QDBusMessage &msg)
{
    Q_UNUSED(msg)
    qDebug() << "AGENT-RequestPasskey " << device.path();
/*
    remoteDevice = adapter->findBluetoothRemoteDeviceUBI(device.path());

    passkeyDialog->setName(remoteDevice->name());
    passkeyDialog->setAddr(remoteDevice->address());

    bool done = execDialog(passkeyDialog);

    qDebug() << "passkey " << QString::number(passkey);

    if (done)
        return passkey;

    QDBusMessage error = msg.createErrorReply("org.bluez.Error.Canceled", "Passkey request failed");
    QDBusConnection::systemBus().send(error);*/
    return 0;
}

void AgentListenerWorker::DisplayPasskey(QDBusObjectPath device, quint32 passkey)
{
    qDebug() << "AGENT-DisplayPasskey " << device.path() << ", " << QString::number(passkey);
}

void AgentListenerWorker::RequestConfirmation(QDBusObjectPath device, quint32 passkey, const QDBusMessage &msg)
{
    qDebug() << "AGENT-RequestConfirmation " << device.path() << ", " << QString::number(passkey);

    Solid::Control::BluetoothRemoteDevice *remote = m_adapter->findBluetoothRemoteDeviceUBI(device.path());

    QStringList list;
    list.append(remote->name());
    list.append(device.path());

    int result = KProcess::execute("bluedevil-requestconfirmation",list);
    if (result == 0) {
        qDebug() << "Go on camarada!";
        return;
    }
    qDebug() << "Sending Authorization cancelled";
    QDBusMessage error = msg.createErrorReply("org.bluez.Error.Canceled", "Authorization canceled");
    QDBusConnection::systemBus().send(error);
}

void AgentListenerWorker::ConfirmModeChange(const QString& mode, const QDBusMessage &msg)
{
    Q_UNUSED(msg)
//     qDebug() << "AGENT-ConfirmModeChange " << adapter->name() << " " << adapter->address() << " " << mode;
        qDebug() << "AGENT-ConfirmModeChange " << " " << mode;
//     confirmDialog->setName(adapter->name());
//     confirmDialog->setAddr(adapter->address());
//     confirmDialog->setMode(mode);
// 
//     bool confirm = execDialog(confirmDialog);
// 
//     if (confirm)
//         return;
// 
//     QDBusMessage error = msg.createErrorReply("org.bluez.Error.Rejected", "Mode change rejected");
//     QDBusConnection::systemBus().send(error);
}

void AgentListenerWorker::Cancel()
{
    qDebug() << "AGENT-Cancel";

//     if (!currentDialog)
//         return;
// 
//     currentDialog->reject();
//     currentDialog = 0;
}