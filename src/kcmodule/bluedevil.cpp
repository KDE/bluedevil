/*
 * Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
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

#include "bluedevil.h"

#include <QtGui/QCheckBox>
#include <QtGui/QBoxLayout>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

#include <kpushbutton.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>

K_PLUGIN_FACTORY(BlueDevilFactory, registerPlugin<KCMBlueDevil>();)
K_EXPORT_PLUGIN(BlueDevilFactory("bluedevil"))

KCMBlueDevil::KCMBlueDevil(QWidget *parent, const QVariantList&)
    : KCModule(BlueDevilFactory::componentData(), parent)
    , m_enable(new QCheckBox(i18n("Enable Bluetooth"), this))
{
    checkKDEDModuleLoaded();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_enable);
    layout->addStretch();
    setLayout(layout);

    connect(m_enable, SIGNAL(stateChanged(int)), SLOT(stateChanged(int)));
}

KCMBlueDevil::~KCMBlueDevil()
{
}

void KCMBlueDevil::defaults()
{
}

void KCMBlueDevil::save()
{
    if (!m_isEnabled && m_enable->isChecked()) {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.kded", "/kded", QString(), "loadModule");
        msg.setArguments(QVariantList() << "bluedevil");
        QDBusConnection::sessionBus().call(msg);
    } else if (m_isEnabled && !m_enable->isChecked()) {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.kded", "/kded", QString(), "unloadModule");
        msg.setArguments(QVariantList() << "bluedevil");
        QDBusConnection::sessionBus().call(msg);
    }

    checkKDEDModuleLoaded();
}

void KCMBlueDevil::stateChanged(int)
{
    if (sender() == m_enable) {
        emit changed(m_enable->isChecked() != m_isEnabled);
    }
}

void KCMBlueDevil::checkKDEDModuleLoaded()
{
    const QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.kded", "/kded", QString(), "loadedModules");
    const QList<QVariant> res = QDBusConnection::sessionBus().call(msg).arguments()[0].toList();
    bool moduleLoaded = false;
    foreach (const QVariant &module, res) {
        if (module.toString() == "bluedevil") {
            moduleLoaded = true;
            break;
        }
    }
    m_enable->setChecked(moduleLoaded);
    m_isEnabled = moduleLoaded;
}
