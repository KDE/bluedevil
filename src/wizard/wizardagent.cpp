/***************************************************************************
 *   Copyright (C) 2010  Alex Fiestas <alex@eyeos.org>                     *
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

#include <QDebug>
#include <bluedevil/bluedevil.h>
#include <kstandarddirs.h>

using namespace BlueDevil;

WizardAgent::WizardAgent(QApplication* application) : QDBusAbstractAdaptor(application)
{
    qDebug() << "AGENT registered !";
}

WizardAgent::~WizardAgent()
{
    qDebug() << "Agent deleted";
}

void WizardAgent::Release()
{
    qDebug() << "Agent Release";
    //TODO: emit a signal
}

void WizardAgent::Authorize(QDBusObjectPath device, const QString& uuid, const QDBusMessage &msg)
{
    Q_UNUSED(device);
    Q_UNUSED(uuid);
    Q_UNUSED(msg);
    qDebug() << "AGENT-Authorize " << device.path() << " Service: " << uuid;
}

quint32 WizardAgent::RequestPasskey(QDBusObjectPath device, const QDBusMessage &msg)
{
    Q_UNUSED(device);
    Q_UNUSED(msg);
    qDebug() << "AGENT-RequestPasskey " << device.path();
    return 0;
}

void WizardAgent::DisplayPasskey(QDBusObjectPath device, quint32 passkey)
{
    Q_UNUSED(device);
    Q_UNUSED(passkey);
    qDebug() << "AGENT-DisplayPasskey " << device.path() << ", " << QString::number(passkey);
}

void WizardAgent::RequestConfirmation(QDBusObjectPath device, quint32 passkey, const QDBusMessage &msg)
{
    Q_UNUSED(device);
    Q_UNUSED(passkey);
    Q_UNUSED(msg);
    qDebug() << "AGENT-RequestConfirmation " << device.path() << ", " << QString::number(passkey);
}

void WizardAgent::ConfirmModeChange(const QString& mode, const QDBusMessage &msg)
{
    Q_UNUSED(mode);
    Q_UNUSED(msg);
    qDebug() << "AGENT-ConfirmModeChange " << mode;
}

void WizardAgent::Cancel()
{
    qDebug() << "AGENT-Cancel";
}

QString WizardAgent::RequestPinCode(QDBusObjectPath device, const QDBusMessage &msg)
{
    Q_UNUSED(device);
    Q_UNUSED(msg);
    qDebug() << "AGENT-RequestPinCode " << device.path();

    QString pin = getPin(device.path());
    emit pinRequested(pin);
    return pin;
}

QString WizardAgent::getPin(const QString& path) const
{
    if(!m_pin.isEmpty()) {
        return m_pin;
    }

    QString xmlPath = KStandardDirs::locate("appdata", "pin-code-database.xml");

    QFile *file = new QFile(xmlPath);
    if(!file->open(QIODevice::ReadOnly)) {
        qDebug() << "Can't open the device";
        return "";
    }

    Device *device = Manager::self()->defaultAdapter()->deviceForUBI(path);
    if (!device) {
        qDebug() << "could not found the device";
        return QString("0000");
    }

    QXmlStreamReader* m_xml = new QXmlStreamReader(file);

    int deviceType = device->deviceClass();
    int xmlType = 0;
    QString pin;

    while(!m_xml->atEnd()) {
        m_xml->readNext();
        if(m_xml->name() != "device") {
            continue;
        }
        QXmlStreamAttributes attr = m_xml->attributes();

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

        return attr.value("pin").toString();
    }

    return QString("0000");
}

void WizardAgent::setPin(const QString& pin)
{
    qDebug() << "Setting ping from ouside: " << pin;
    m_pin = pin;
}