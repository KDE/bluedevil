/***************************************************************************
 *   Copyright (C) 2010 Alex Fiestas <alex@eyeos.org>                      *
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

#include "wizardagent.h"

#include <QDBusMessage>
#include <bluedevil/bluedevil.h>
#include <KDebug>
#include <KAboutData>
#include <krandom.h>
#include <kstandarddirs.h>
#include <klocalizedstring.h>
#include <KComponentData>

using namespace BlueDevil;

WizardAgent::WizardAgent(QApplication* application) : QDBusAbstractAdaptor(application), m_fromDatabase(false)
{
    kDebug() << "AGENT registered !";
}

WizardAgent::~WizardAgent()
{
    kDebug() << "Agent deleted";
}

void WizardAgent::Release()
{
    kDebug() << "Agent Release";
    emit agentReleased();
}

void WizardAgent::Authorize(QDBusObjectPath device, const QString& uuid, const QDBusMessage &msg)
{
    Q_UNUSED(device);
    Q_UNUSED(uuid);
    Q_UNUSED(msg);
    kDebug() << "AGENT-Authorize " << device.path() << " Service: " << uuid;
}

quint32 WizardAgent::RequestPasskey(QDBusObjectPath device, const QDBusMessage &msg)
{
    Q_UNUSED(device);
    Q_UNUSED(msg);
    kDebug() << "AGENT-RequestPasskey " << device.path();
    return 0;
}

void WizardAgent::DisplayPasskey(QDBusObjectPath device, quint32 passkey)
{
    Q_UNUSED(device);
    Q_UNUSED(passkey);
    kDebug() << "AGENT-DisplayPasskey " << device.path() << ", " << QString::number(passkey);
}

void WizardAgent::RequestConfirmation(QDBusObjectPath device, quint32 passkey, const QDBusMessage &msg)
{
    Q_UNUSED(device);
    Q_UNUSED(passkey);
    Q_UNUSED(msg);
    kDebug() << "AGENT-RequestConfirmation " << device.path() << ", " << QString::number(passkey);
    emit confirmationRequested(passkey, msg);
}

void WizardAgent::ConfirmModeChange(const QString& mode, const QDBusMessage &msg)
{
    Q_UNUSED(mode);
    Q_UNUSED(msg);
    kDebug() << "AGENT-ConfirmModeChange " << mode;
}

void WizardAgent::Cancel()
{
    kDebug() << "AGENT-Cancel";
}

QString WizardAgent::RequestPinCode(QDBusObjectPath device, const QDBusMessage &msg)
{
    Q_UNUSED(device);
    Q_UNUSED(msg);
    kDebug() << "AGENT-RequestPinCode " << device.path();

    emit pinRequested(m_pin);
    return m_pin;
}

QString WizardAgent::getPin(Device *device)
{
    if(!m_pin.isEmpty()) {
        return m_pin;
    }

    m_pin = QString::number(KRandom::random());
    m_pin = m_pin.left(6);

    KComponentData data("bluedevilwizard");
    QString xmlPath = KStandardDirs::locate("appdata", "pin-code-database.xml", data);

    QFile file(xmlPath);
    if(!file.open(QIODevice::ReadOnly)) {
        kDebug() << "Can't open the device";
        return m_pin;
    }

    if (!device) {
        kDebug() << "could not found the device";
        return m_pin;
    }

    m_device = device;
    QXmlStreamReader m_xml(&file);

    int deviceType = classToType(device->deviceClass());
    int xmlType = 0;

    while(!m_xml.atEnd()) {
        m_xml.readNext();
        if(m_xml.name() != "device") {
            continue;
        }
        QXmlStreamAttributes attr = m_xml.attributes();

        if(attr.count() == 0) {
            continue;
        }

        if(attr.hasAttribute("type") && attr.value("type") != "any") {
            xmlType = stringToType(attr.value("type").toString());
            if(deviceType != xmlType) {
                xmlType = 0; //This is not needed but I like restart the bucle in each interation
                continue;
            }
        }

        if(attr.hasAttribute("oui")) {
            if(!device->address().startsWith(attr.value("oui").toString())) {
                continue;
            }
        }

        if(attr.hasAttribute("name")) {
            if(device->name() != attr.value("name").toString()) {
                continue;
            }
        }

        m_pin = attr.value("pin").toString();
        m_fromDatabase = true;
        if (m_pin.startsWith("max:")) {
            m_fromDatabase = false;
            int num = m_pin.right(m_pin.length() - 4).toInt();
            m_pin = QString::number(KRandom::random()).left(num);
        }
        kDebug() << "PIN: " << m_pin;
        return m_pin;
    }

    return m_pin;
}

void WizardAgent::setPin(const QString& pin)
{
    m_pin = pin;
}

QString WizardAgent::pin()
{
    return m_pin;
}

bool WizardAgent::isFromDatabase()
{
    return m_fromDatabase;
}
