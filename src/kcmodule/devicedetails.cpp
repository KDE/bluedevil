/*
 * Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
 * Copyright (C) 2010 UFO Coders <info@ufocoders.com>
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

#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>

#include <kled.h>
#include <klocalizedstring.h>

#include <QBluez/Device>

DeviceDetails::DeviceDetails(QBluez::Device *device, QWidget *parent)
    : QDialog(parent)
    , m_device(device)
    , m_alias(new QLineEdit(this))
    , m_blocked(new QCheckBox(this))
    , m_trusted(new QCheckBox(this))
    , m_buttonBox(new QDialogButtonBox(this))
{
    m_alias->setClearButtonEnabled(true);
    m_alias->setText(device->alias());

    QFormLayout *layout = new QFormLayout;
    layout->addRow(i18nc("Name of the device", "Name"), new QLabel(device->name()));
    layout->addRow(i18nc("Alias of the device", "Alias"), m_alias);
    QLineEdit *address = new QLineEdit(this);
    address->setReadOnly(true);
    address->setText(device->address());
    layout->addRow(i18nc("Physical address of the device", "Address"), address);
    KLed *paired = new KLed(this);
    paired->setState(device->isPaired() ? KLed::On : KLed::Off);
    layout->addRow(i18nc("Device is paired" ,"Paired"), paired);
    m_blocked->setChecked(device->isBlocked());
    layout->addRow(i18nc("Device is blocked", "Blocked"), m_blocked);
    m_trusted->setChecked(device->isTrusted());
    layout->addRow(i18nc("Device is trusted", "Trusted"), m_trusted);
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Reset | QDialogButtonBox::Cancel);
    layout->addRow(m_buttonBox);
    setLayout(layout);

    connect(m_blocked, &QCheckBox::toggled, this, &DeviceDetails::blockToggled);
    connect(m_buttonBox, &QDialogButtonBox::clicked, this, &DeviceDetails::buttonClicked);
}

void DeviceDetails::buttonClicked(QAbstractButton *button)
{
    switch (m_buttonBox->buttonRole(button)) {
    case QDialogButtonBox::AcceptRole:
        m_device->setAlias(m_alias->text());
        m_device->setTrusted(m_trusted->isChecked());
        m_device->setBlocked(m_blocked->isChecked());
        accept();
        break;
    case QDialogButtonBox::ResetRole:
        m_alias->setText(m_device->alias());
        m_blocked->setChecked(m_device->isBlocked());
        m_trusted->setChecked(m_device->isTrusted());
        break;
    case QDialogButtonBox::RejectRole:
        reject();
        break;
    default:
        break;
    }
}

void DeviceDetails::blockToggled(bool checked)
{
    m_trusted->setEnabled(!checked);
    if (checked) {
        m_trusted->setChecked(false);
    }
}
