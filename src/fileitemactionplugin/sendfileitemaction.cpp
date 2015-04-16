/*
 * Copyright (C) 2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * Copyright (C) 2011 UFO Coders <info@ufocoders.com>
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

#include "sendfileitemaction.h"

#include <QList>
#include <QMenu>
#include <QDebug>
#include <QAction>
#include <QWidget>
#include <QVariantList>
#include <QStringBuilder>

#include <QProcess>

#include <KPluginFactory>
#include <KLocalizedString>

#include <BluezQt/Services>

K_PLUGIN_FACTORY_WITH_JSON(SendFileItemActionFactory,
                           "bluedevilsendfile.json",
                           registerPlugin<SendFileItemAction>();)

SendFileItemAction::SendFileItemAction(QObject *parent, const QVariantList&)
    : KAbstractFileItemActionPlugin(parent)
{
    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<QMapDeviceInfo>();

    m_kded = new org::kde::BlueDevil(QStringLiteral("org.kde.kded5"), QStringLiteral("/modules/bluedevil"),
                                     QDBusConnection::sessionBus(), this);
}

QList<QAction*> SendFileItemAction::actions(const KFileItemListProperties &fileItemInfos, QWidget *parent)
{
    Q_UNUSED(parent)

    QList<QAction*> list;

    // Don't show the action for files that we can't send or when Bluetooth is offline.
    if (!fileItemInfos.isLocal() || !m_kded->isOnline()) {
        return list;
    }

    m_infos = fileItemInfos;

    QAction *menuAction = new QAction(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")), i18n("Send via Bluetooth"), this);
    QMenu *menu = new QMenu();

    const QMapDeviceInfo &devices = m_kded->allDevices().value();
    Q_FOREACH (const DeviceInfo &device, devices) {
        if (device.value(QStringLiteral("UUIDs")).contains(BluezQt::Services::ObexObjectPush)) {
            QAction *action = new QAction(QIcon::fromTheme(device[QStringLiteral("icon")]), device.value(QStringLiteral("name")), this);
            connect(action, &QAction::triggered, this, &SendFileItemAction::deviceTriggered);
            action->setData(device.value(QStringLiteral("UBI")));
            menu->addAction(action);
        }
    }

    QAction *otherAction = new QAction(this);
    if (menu->actions().isEmpty()) {
        otherAction->setText(i18nc("Find Bluetooth device", "Find Device..."));
    } else {
        menu->addSeparator();
        otherAction->setText(i18nc("Other Bluetooth device", "Other..."));
    }
    connect(otherAction, &QAction::triggered, this, &SendFileItemAction::otherTriggered);
    menu->addAction(otherAction);

    menuAction->setMenu(menu);
    list << menuAction;
    return list;
}

void SendFileItemAction::deviceTriggered()
{
    QStringList args;
    args.append(QStringLiteral("-u") % static_cast<QAction*>(sender())->data().toString());

    const QList<QUrl> &fileList = m_infos.urlList();
    Q_FOREACH (const QUrl &url, fileList) {
        args.append(QStringLiteral("-f") % url.path());
    }

    QProcess::startDetached(QStringLiteral("bluedevil-sendfile"), args);
}

void SendFileItemAction::otherTriggered()
{
    QStringList args;

    const QList<QUrl> &fileList = m_infos.urlList();
    Q_FOREACH (const QUrl &url, fileList) {
        args.append(QStringLiteral("-f") % url.path());
    }

    QProcess::startDetached(QStringLiteral("bluedevil-sendfile"), args);
}

#include "sendfileitemaction.moc"
