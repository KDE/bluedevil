/*
 * Copyright (C) 2015 David Rosca <nowrep@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "devicedetails.h"
#include "ui_devicedetails.h"

#include <QProcess>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>

#include <KLocalizedString>

#include <BluezQt/Device>
#include <BluezQt/Adapter>
#include <BluezQt/Services>
#include <BluezQt/PendingCall>

DeviceDetails::DeviceDetails(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::DeviceDetails)
{
    m_ui->setupUi(this);

    connect(m_ui->connectButton, &QPushButton::clicked, this, &DeviceDetails::connectClicked);
    connect(m_ui->sendFileButton, &QPushButton::clicked, this, &DeviceDetails::sendFileClicked);
    connect(m_ui->napButton, &QPushButton::clicked, this, &DeviceDetails::setupNapClicked);
    connect(m_ui->dunButton, &QPushButton::clicked, this, &DeviceDetails::setupDunClicked);

    connect(m_ui->name, &QLineEdit::textEdited, this, &DeviceDetails::modifiedByUser);
    connect(m_ui->trusted, &QCheckBox::toggled, this, &DeviceDetails::modifiedByUser);
    connect(m_ui->blocked, &QCheckBox::toggled, this, &DeviceDetails::modifiedByUser);
}

void DeviceDetails::setDevice(BluezQt::DevicePtr device)
{
    Q_ASSERT(device);

    m_device = device;

    m_ui->icon->setPixmap(QIcon::fromTheme(m_device->icon(), QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth"))).pixmap(128));
    m_ui->type->setText(deviceType());
    m_ui->address->setText(m_device->address());
    m_ui->name->setText(m_device->name());
    m_ui->name->setCursorPosition(0);
    m_ui->trusted->setChecked(m_device->isTrusted());
    m_ui->blocked->setChecked(m_device->isBlocked());

    connectedChanged(m_device->isConnected());
    connect(m_device.data(), &BluezQt::Device::connectedChanged, this, &DeviceDetails::connectedChanged);

    remoteNameChanged(m_device->remoteName());
    connect(m_device.data(), &BluezQt::Device::remoteNameChanged, this, &DeviceDetails::remoteNameChanged);

    uuidsChanged(m_device->uuids());
    connect(m_device.data(), &BluezQt::Device::uuidsChanged, this, &DeviceDetails::uuidsChanged);

    adapterNameChanged(m_device->adapter()->name());
    connect(m_device->adapter().data(), &BluezQt::Adapter::nameChanged, this, &DeviceDetails::adapterNameChanged);
}

void DeviceDetails::load()
{
    m_ui->name->setText(m_device->name());
    m_ui->name->setCursorPosition(0);
    m_ui->trusted->setChecked(m_device->isTrusted());
    m_ui->blocked->setChecked(m_device->isBlocked());
}

void DeviceDetails::save()
{
    m_device->setName(m_ui->name->text());
    m_device->setTrusted(m_ui->trusted->isChecked());
    m_device->setBlocked(m_ui->blocked->isChecked());

    if (m_ui->name->text().isEmpty()) {
        m_ui->name->setText(m_device->remoteName());
        m_ui->name->setCursorPosition(0);
    }
}

// Returns "hciX" part from UBI "/org/bluez/hciX/dev_xx_xx_xx_xx_xx_xx"
QString DeviceDetails::adapterHciString(const QString &ubi)
{
    int startIndex = ubi.indexOf(QLatin1String("/hci")) + 1;
    if (startIndex < 1) {
        return QString();
    }

    int endIndex = ubi.indexOf(QLatin1Char('/'), startIndex);
    if (endIndex == -1) {
        return ubi.mid(startIndex);
    }

    return ubi.mid(startIndex, endIndex - startIndex);
}

void DeviceDetails::remoteNameChanged(const QString &name)
{
    m_ui->remoteName->setText(QStringLiteral("<b>%1</b>").arg(name));
}

void DeviceDetails::uuidsChanged(const QStringList &uuids)
{
    m_ui->sendFileButton->setVisible(uuids.contains(BluezQt::Services::ObexObjectPush));

    m_ui->napButton->hide();
    m_ui->dunButton->hide();

    if (uuids.contains(BluezQt::Services::Nap)) {
        checkNetworkConnection(QStringLiteral("nap"), [this](bool exists) {
            if (!exists) {
                m_ui->napButton->show();
                updateActions();
            }
        });
    }

    if (uuids.contains(BluezQt::Services::DialupNetworking)) {
        checkNetworkConnection(QStringLiteral("dun"), [this](bool exists) {
            if (!exists) {
                m_ui->dunButton->show();
                updateActions();
            }
        });
    }

    updateActions();
}

void DeviceDetails::adapterNameChanged(const QString &name)
{
    const QString &hci = adapterHciString(m_device->adapter()->ubi());

    if (!hci.isEmpty()) {
        m_ui->adapter->setText(QStringLiteral("%1 (%2)").arg(name, hci));
    } else {
        m_ui->adapter->setText(name);
    }
}

void DeviceDetails::connectedChanged(bool connected)
{
    if (connected) {
        m_ui->connectButton->setText(i18n("Disconnect"));
        m_ui->connectButton->setIcon(QIcon::fromTheme(QStringLiteral("network-disconnect")));
    } else {
        m_ui->connectButton->setText(i18n("Connect"));
        m_ui->connectButton->setIcon(QIcon::fromTheme(QStringLiteral("network-connect")));
    }
}

void DeviceDetails::connectClicked()
{
    parentWidget()->setFocus();
    m_ui->connectButton->setEnabled(false);

    BluezQt::PendingCall *call;
    if (m_device->isConnected()) {
        call = m_device->disconnectFromDevice();
    } else {
        call = m_device->connectToDevice();
    }

    connect(call, &BluezQt::PendingCall::finished, this, [this]() {
        m_ui->connectButton->setEnabled(true);
    });
}

void DeviceDetails::sendFileClicked()
{
    const QStringList &args = { QStringLiteral("-u"), m_device->ubi() };
    QProcess::startDetached(QStringLiteral("bluedevil-sendfile"), args);
}

void DeviceDetails::setupNapClicked()
{
    setupNetworkConnection(QStringLiteral("nap"));
}

void DeviceDetails::setupDunClicked()
{
    setupNetworkConnection(QStringLiteral("dun"));
}

void DeviceDetails::modifiedByUser()
{
    bool modified = m_device->name() != m_ui->name->text()
            || m_device->isTrusted() != m_ui->trusted->isChecked()
            || m_device->isBlocked() != m_ui->blocked->isChecked();

    Q_EMIT changed(modified);
}

QString DeviceDetails::deviceType() const
{
    switch (m_device->type()) {
    case BluezQt::Device::Phone:
        return i18nc("This device is a Phone", "Phone");
    case BluezQt::Device::Modem:
        return i18nc("This device is a Modem", "Modem");
    case BluezQt::Device::Computer:
        return i18nc("This device is a Computer", "Computer");
    case BluezQt::Device::Network:
        return i18nc("This device is of type Network", "Network");
    case BluezQt::Device::Headset:
        return i18nc("This device is a Headset", "Headset");
    case BluezQt::Device::Headphones:
        return i18nc("This device is a Headphones", "Headphones");
    case BluezQt::Device::AudioVideo:
        return i18nc("This device is an Audio device", "Audio");
    case BluezQt::Device::Keyboard:
        return i18nc("This device is a Keyboard", "Keyboard");
    case BluezQt::Device::Mouse:
        return i18nc("This device is a Mouse", "Mouse");
    case BluezQt::Device::Joypad:
        return i18nc("This device is a Joypad", "Joypad");
    case BluezQt::Device::Tablet:
        return i18nc("This device is a Graphics Tablet (input device)", "Tablet");
    case BluezQt::Device::Peripheral:
        return i18nc("This device is a Peripheral device", "Peripheral");
    case BluezQt::Device::Camera:
        return i18nc("This device is a Camera", "Camera");
    case BluezQt::Device::Printer:
        return i18nc("This device is a Printer", "Printer");
    case BluezQt::Device::Imaging:
        return i18nc("This device is an Imaging device (printer, scanner, camera, display, ...)", "Imaging");
    case BluezQt::Device::Wearable:
        return i18nc("This device is a Wearable", "Wearable");
    case BluezQt::Device::Toy:
        return i18nc("This device is a Toy", "Toy");
    case BluezQt::Device::Health:
        return i18nc("This device is a Health device", "Health");
    default:
        return i18nc("Type of device: could not be determined", "Unknown");
    }
}

void DeviceDetails::updateActions()
{
    bool sendFileVisible = m_ui->sendFileButton->isVisible();
    bool napVisible = m_ui->napButton->isVisible();
    bool dunVisible = m_ui->dunButton->isVisible();

    m_ui->actionsLabel->setVisible(sendFileVisible || napVisible || dunVisible);
}

void DeviceDetails::setupNetworkConnection(const QString &service)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmanetworkmanagement"),
                                                      QStringLiteral("/org/kde/plasmanetworkmanagement"),
                                                      QStringLiteral("org.kde.plasmanetworkmanagement"),
                                                      QStringLiteral("addBluetoothConnection"));

    msg << m_device->address();
    msg << service;
    msg << i18nc("DeviceName Network (Service)", "%1 Network (%2)", m_device->name(), service);

    QDBusConnection::sessionBus().asyncCall(msg);
}

void DeviceDetails::checkNetworkConnection(const QString &service, std::function<void (bool)> func)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmanetworkmanagement"),
                                                      QStringLiteral("/org/kde/plasmanetworkmanagement"),
                                                      QStringLiteral("org.kde.plasmanetworkmanagement"),
                                                      QStringLiteral("bluetoothConnectionExists"));

    msg << m_device->address();
    msg << service;

    QDBusPendingCallWatcher *call = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg));
    connect(call, &QDBusPendingCallWatcher::finished, this, [call, func]() {
        QDBusPendingReply<bool> reply = *call;
        func(!reply.isError() && reply.value());
    });
}
