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
#include <solid/control/bluetoothmanager.h>

AgentListenerWorker::AgentListenerWorker(QObject *app) :  QDBusAbstractAdaptor(app)
{
    const QString agentPath = "/blueDevil_agent";

    if(!QDBusConnection::systemBus().registerObject(agentPath, app)){
        qDebug() << "The dbus object can't be registered";
    }
    
    Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface *adapter = new Solid::Control::BluetoothInterface(man.defaultInterface());
    adapter->registerAgent(agentPath,"DisplayYesNo");
    qDebug() << "Agent registered";
}

AgentListenerWorker::~AgentListenerWorker()
{

}

void AgentListenerWorker::Release()
{
    qDebug() << "Agent Release";
//     if ( exitOnRelease() )
//         app->quit();
}

void AgentListenerWorker::Authorize(QDBusObjectPath device, const QString& uuid, const QDBusMessage &msg)
{
        qDebug() << "AAAAAAAAAAAAAAAAA"; 
//     qDebug() << "AGENT-Authorize " << device.path() << " Service: " << uuid;
//     remoteDevice = adapter->findBluetoothRemoteDeviceUBI(device.path());
//     authDialog->setName(remoteDevice->name());
//     authDialog->setAddr(remoteDevice->address());
//     authDialog->setService(uuid);
// 
//     int trust = execDialog(authDialog);
//     qDebug() << "trust value = " << trust;
// 
//     switch(trust) {
//         case(2): {
//             remoteDevice->setTrusted(true);
//             qDebug() << "Set Always Trust for " << remoteDevice->name();
//             return;
// //      }
//         case(1): {
//             //No error no anything means OK, and this case is for the Trust button
//             return;
//         }
//         case(0): {
//             QDBusMessage error = msg.createErrorReply("org.bluez.Error.Canceled", "Authorization canceled");
//             QDBusConnection::systemBus().send(error);
//             break;
//         }
//     }
}

QString AgentListenerWorker::RequestPinCode(QDBusObjectPath device, const QDBusMessage &msg)
{
    qDebug() << "AGENT-RequestPinCode " << device.path();
/*
    remoteDevice = adapter->findBluetoothRemoteDeviceUBI(device.path());

    passkeyDialog->setName(remoteDevice->name());
    passkeyDialog->setAddr(remoteDevice->address());
    passkeyDialog->clearPinCode();

    bool done = execDialog(passkeyDialog);

    qDebug() << "pinCode " << pincode;

    if (done)
        return pincode;

    QDBusMessage error = msg.createErrorReply("org.bluez.Error.Canceled", "Pincode request failed");
    QDBusConnection::systemBus().send(error);
    return 0;*/
}

quint32 AgentListenerWorker::RequestPasskey(QDBusObjectPath device, const QDBusMessage &msg)
{
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
    QDBusConnection::systemBus().send(error);
    return 0;*/
}

void AgentListenerWorker::DisplayPasskey(QDBusObjectPath device, quint32 passkey)
{
    qDebug() << "AGENT-DisplayPasskey " << device.path() << ", " << QString::number(passkey);
}

void AgentListenerWorker::RequestConfirmation(QDBusObjectPath device, quint32 passkey, const QDBusMessage &msg)
{
    qDebug() << "AGENT-RequestConfirmation " << device.path() << ", " << QString::number(passkey);
//     remoteDevice = adapter->findBluetoothRemoteDeviceUBI(device.path());
//     confirmDialog->setName(remoteDevice->name());
//     confirmDialog->setAddr(remoteDevice->address());
//     confirmDialog->setPassKey(QString::number(passkey));
// 
//     bool confirm = execDialog(confirmDialog);
// 
//     if (confirm)
//         return;
// 
//     QDBusMessage error = msg.createErrorReply("org.bluez.Error.Rejected", "Confirmation rejected");
//     QDBusConnection::systemBus().send(error);
}

void AgentListenerWorker::ConfirmModeChange(const QString& mode, const QDBusMessage &msg)
{
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