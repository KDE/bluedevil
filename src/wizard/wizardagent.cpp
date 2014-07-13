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
#include "debug_p.h"

#include <QDebug>
#include <QDBusMessage>
#include <QStandardPaths>

#include <KAboutData>
#include <krandom.h>
#include <klocalizedstring.h>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

WizardAgent::WizardAgent(QApplication* application)
    : QDBusAbstractAdaptor(application)
    , m_fromDatabase(false)
{
    qCDebug(WIZARD) << "AGENT registered !";
    BlueDevil::Manager::self()->registerAgent(QStringLiteral("/wizardAgent"), BlueDevil::Manager::DisplayYesNo);
}

WizardAgent::~WizardAgent()
{
    qCDebug(WIZARD) << "Agent deleted";
    BlueDevil::Manager::self()->unregisterAgent(QStringLiteral("/wizardAgent"));
}

void WizardAgent::Release()
{
    qCDebug(WIZARD) << "Agent Release";
    emit agentReleased();
}

void WizardAgent::AuthorizeService(const QDBusObjectPath& device, const QString& uuid, const QDBusMessage& msg)
{
    Q_UNUSED(device);
    Q_UNUSED(uuid);
    Q_UNUSED(msg);
    qCDebug(WIZARD) << "AGENT-Authorize " << device.path() << " Service: " << uuid;
}

quint32 WizardAgent::RequestPasskey(const QDBusObjectPath& device, const QDBusMessage& msg)
{
    Q_UNUSED(device);
    Q_UNUSED(msg);
    qCDebug(WIZARD) << "AGENT-RequestPasskey " << device.path();
    emit pinRequested(m_pin);
    return m_pin.toUInt();
}

void WizardAgent::DisplayPasskey(const QDBusObjectPath& device, quint32 passkey, quint8 entered)
{
    Q_UNUSED(device);
    Q_UNUSED(entered);
    qCDebug(WIZARD) << "AGENT-DisplayPasskey " << device.path() << ", " << QString::number(passkey);
    emit pinRequested(QString(QStringLiteral("%1")).arg(passkey, 6, 10, QLatin1Char('0')));
}

void WizardAgent::DisplayPinCode(const QDBusObjectPath& device, const QString& pincode)
{
    Q_UNUSED(device);
    Q_UNUSED(pincode);
    qCDebug(WIZARD) << "AGENT-DisplayPasskey " << device.path() << ", " << pincode;
    emit pinRequested(pincode);
}

void WizardAgent::RequestConfirmation(const QDBusObjectPath& device, quint32 passkey, const QDBusMessage& msg)
{
    Q_UNUSED(device);
    Q_UNUSED(passkey);
    Q_UNUSED(msg);
    qCDebug(WIZARD) << "AGENT-RequestConfirmation " << device.path() << ", " << QString::number(passkey);
    emit confirmationRequested(passkey, msg);
}

void WizardAgent::Cancel()
{
    qCDebug(WIZARD) << "AGENT-Cancel";
}

QString WizardAgent::RequestPinCode(const QDBusObjectPath& device, const QDBusMessage& msg)
{
    Q_UNUSED(device);
    Q_UNUSED(msg);
    qCDebug(WIZARD) << "AGENT-RequestPinCode " << device.path();

    emit pinRequested(m_pin);
    return m_pin;
}

QString WizardAgent::getPin(Device *device)
{
    if (!m_pin.isEmpty()) {
        return m_pin;
    }

    m_pin = QString::number(KRandom::random());
    m_pin = m_pin.left(6);

    const QString &xmlPath = QStandardPaths::locate(QStandardPaths::DataLocation,
                                                    QStringLiteral("pin-code-database.xml"));

    QFile file(xmlPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(WIZARD) << "Can't open the device";
        return m_pin;
    }

    if (!device) {
        qCDebug(WIZARD) << "could not found the device";
        return m_pin;
    }

    m_device = device;
    QXmlStreamReader m_xml(&file);

    int deviceType = classToType(device->deviceClass());
    int xmlType = 0;

    while (!m_xml.atEnd()) {
        m_xml.readNext();
        if (m_xml.name() != QLatin1String("device")) {
            continue;
        }
        QXmlStreamAttributes attr = m_xml.attributes();

        if (attr.count() == 0) {
            continue;
        }

        if (attr.hasAttribute(QLatin1String("type")) && attr.value(QLatin1String("type")) != QLatin1String("any")) {
            xmlType = stringToType(attr.value(QLatin1String("type")).toString());
            if (deviceType != xmlType) {
                xmlType = 0; //This is not needed but I like restart the bucle in each interation
                continue;
            }
        }

        if (attr.hasAttribute(QLatin1String("oui"))) {
            if (!device->address().startsWith(attr.value(QLatin1String("oui")).toString())) {
                continue;
            }
        }

        if (attr.hasAttribute(QLatin1String("name"))) {
            if (device->name() != attr.value(QLatin1String("name")).toString()) {
                continue;
            }
        }

        m_pin = attr.value(QLatin1String("pin")).toString();
        m_fromDatabase = true;
        if (m_pin.startsWith(QLatin1String("max:"))) {
            m_fromDatabase = false;
            int num = m_pin.right(m_pin.length() - 4).toInt();
            m_pin = QString::number(KRandom::random()).left(num);
        }
        qCDebug(WIZARD) << "PIN: " << m_pin;
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
