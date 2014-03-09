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

#include <QtDBus>
#include <QApplication>
#include <QXmlStreamReader>

namespace BlueDevil {
    class Device;
}

using namespace BlueDevil;
class WizardAgent : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent1")

public:
    WizardAgent(QApplication* application);
    ~WizardAgent();

    void setPin(const QString& pin);
    QString getPin(Device* device);
    QString pin();
    bool isFromDatabase();

//D-Bus interface implementation
public slots:
    void Release();
    void AuthorizeService(const QDBusObjectPath &device, const QString& uuid, const QDBusMessage &msg);
    QString RequestPinCode(const QDBusObjectPath &device, const QDBusMessage &msg);
    quint32 RequestPasskey(const QDBusObjectPath &device, const QDBusMessage &msg);
    void DisplayPinCode(const QDBusObjectPath &device, const QString & pincode);
    void DisplayPasskey(const QDBusObjectPath &device, quint32 passkey, quint8 entered);
    void RequestConfirmation(const QDBusObjectPath &device, quint32 passkey, const QDBusMessage &msg);
    void Cancel();

private:
    bool    m_fromDatabase;
    QString m_pin;
    Device *m_device;

Q_SIGNALS:
    void pinRequested(const QString&);
    void confirmationRequested(quint32 passkey, const QDBusMessage &msg);
    void agentReleased();
};

#endif
