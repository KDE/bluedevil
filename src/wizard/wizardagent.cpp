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
#include <QUrl>
#include <QFile>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QStandardPaths>
#include <QXmlStreamReader>

#include <krandom.h>
#include <klocalizedstring.h>

#include <QBluez/Manager>
#include <QBluez/Device>
#include <QBluez/Utils>

WizardAgent::WizardAgent(QObject *parent)
    : QBluez::Agent(parent)
    , m_fromDatabase(false)
{
}

WizardAgent::~WizardAgent()
{
    qCDebug(WIZARD) << "Agent deleted";
}

void WizardAgent::setPin(const QString& pin)
{
    m_pin = pin;
}

QString WizardAgent::getPin(QBluez::Device *device)
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

    QXmlStreamReader m_xml(&file);

    int deviceType = device->deviceType();
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
            xmlType = QBluez::Device::stringToType(attr.value(QLatin1String("type")).toString());
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

QString WizardAgent::pin()
{
    return m_pin;
}

bool WizardAgent::isFromDatabase()
{
    return m_fromDatabase;
}

QDBusObjectPath WizardAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/wizardAgent"));
}

void WizardAgent::requestPinCode(QBluez::Device *device, const QBluez::Request<QString> &req)
{
    Q_UNUSED(device);

    qCDebug(WIZARD) << "AGENT-RequestPinCode" << device->ubi();

    emit pinRequested(m_pin);
    req.accept(m_pin);
}

void WizardAgent::displayPinCode(QBluez::Device *device, const QString &pinCode)
{
    Q_UNUSED(device);
    Q_UNUSED(pinCode);

    qCDebug(WIZARD) << "AGENT-DisplayPinCode" << device->ubi() << "," << pinCode;

    emit pinRequested(pinCode);
}

void WizardAgent::requestPasskey(QBluez::Device *device, const QBluez::Request<quint32> &req)
{
    Q_UNUSED(device);

    qCDebug(WIZARD) << "AGENT-RequestPasskey" << device->ubi();

    emit pinRequested(m_pin);
    req.accept(m_pin.toUInt());
}

void WizardAgent::displayPasskey(QBluez::Device *device, const QString &passkey, const QString &entered)
{
    Q_UNUSED(device);
    Q_UNUSED(entered);

    qCDebug(WIZARD) << "AGENT-DisplayPasskey" << device->ubi() << "," << passkey;

    emit pinRequested(passkey);
}

void WizardAgent::requestConfirmation(QBluez::Device *device, const QString &passkey, const QBluez::Request<void> &req)
{
    Q_UNUSED(device);
    Q_UNUSED(passkey);

    qCDebug(WIZARD) << "AGENT-RequestConfirmation " << device->ubi() << ", " << passkey;

    emit confirmationRequested(passkey, req);
}

void WizardAgent::requestAuthorization(QBluez::Device *device, const QBluez::Request<void> &req)
{
    Q_UNUSED(device);
    Q_UNUSED(req);

    qCDebug(WIZARD) << "AGENT-RequestAuthorization" << device->ubi();
}

void WizardAgent::authorizeService(QBluez::Device *device, const QString &uuid, const QBluez::Request<void> &req)
{
    Q_UNUSED(device);
    Q_UNUSED(uuid);
    Q_UNUSED(req);

    qCDebug(WIZARD) << "AGENT-AuthorizeService" << device->ubi() << "Service:" << uuid;
}

void WizardAgent::cancel()
{
    qCDebug(WIZARD) << "AGENT-Cancel";
}

void WizardAgent::release()
{
    qCDebug(WIZARD) << "AGENT-Release";
    emit agentReleased();
}
