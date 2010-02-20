/***************************************************************************
 *   Copyright (C) 2008  Tom Patzig <tpatzig@suse.de>                      *
 *   Copyright (C) 2008  Alex Fiestas <alex@eyeos.org>                     *
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


#ifndef AGENTLISTENERWORKER_H
#define AGENTLISTENERWORKER_H

#include <QtDBus>
#include <QDebug>
#include <QThread>
#include <solid/control/bluetoothinterface.h>

class AgentListenerWorker : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent")

public:
    AgentListenerWorker(QObject *app);
    ~AgentListenerWorker();

public slots:
    void Release();
    void Authorize(QDBusObjectPath device, const QString& uuid, const QDBusMessage &msg);
    QString RequestPinCode(QDBusObjectPath device, const QDBusMessage &msg);
    quint32 RequestPasskey(QDBusObjectPath device, const QDBusMessage &msg);
    void DisplayPasskey(QDBusObjectPath device, quint32 passkey);
    void RequestConfirmation(QDBusObjectPath device, quint32 passkey, const QDBusMessage &msg);
    void ConfirmModeChange(const QString& mode, const QDBusMessage &msg);
    void Cancel();

private:
    Solid::Control::BluetoothInterface *m_adapter;
};
#endif
