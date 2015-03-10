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

#include <KPluginFactory>

#include <KProcess>
#include <KLocalizedString>

K_PLUGIN_FACTORY_WITH_JSON(SendFileItemActionFactory,
                           "bluedevilsendfile.json",
                           registerPlugin<SendFileItemAction>();)

SendFileItemAction::SendFileItemAction(QObject* parent, const QVariantList& args)
    : KAbstractFileItemActionPlugin(parent)
{
    Q_UNUSED(args)

    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<QMapDeviceInfo>();

    m_kded = new org::kde::BlueDevil(QStringLiteral("org.kde.kded5"), QStringLiteral("/modules/bluedevil"),
                                     QDBusConnection::sessionBus(), this);
}

QList< QAction* > SendFileItemAction::actions(const KFileItemListProperties& fileItemInfos, QWidget* parentWidget)
{
    Q_UNUSED(parentWidget)

    QList<QAction*> list;

    // Don't show the action for files that we can't send or when Bluetooth is offline.
    if (!fileItemInfos.isLocal() || !m_kded->isOnline()) {
        return list;
    }

    m_fileItemInfos = fileItemInfos;

    QAction *menuAction = new QAction(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")), i18n("Send via Bluetooth"), this);
    QMenu *menu = new QMenu();

    const QMapDeviceInfo &devices = m_kded->allDevices().value();
    Q_FOREACH (const DeviceInfo &device, devices) {
        if (device[QStringLiteral("UUIDs")].contains(QLatin1String("00001105-0000-1000-8000-00805F9B34FB"))) {
            QAction *action = new QAction(QIcon::fromTheme(device[QStringLiteral("icon")]), device[QStringLiteral("name")], this);
            connect(action, SIGNAL(triggered(bool)), this, SLOT(deviceTriggered()));
            action->setData(device["UBI"]);
            menu->addAction(action);
        }
    }

    QAction *otherAction = new QAction(this);
    connect(otherAction, SIGNAL(triggered(bool)), this, SLOT(otherTriggered()));
    if (menu->actions().isEmpty()) {
        otherAction->setText(i18nc("Find Bluetooth device", "Find Device..."));
    } else {
        menu->addSeparator();
        otherAction->setText(i18nc("Other Bluetooth device", "Other..."));
    }
    menu->addAction(otherAction);

    menuAction->setMenu(menu);
    list << menuAction;
    return list;
}

void SendFileItemAction::deviceTriggered()
{
    QStringList args;
    args.append(QLatin1String("-u") % static_cast<QAction *>(sender())->data().toString());

    const QList<QUrl> &fileList = m_fileItemInfos.urlList();
    Q_FOREACH(const QUrl &url, fileList) {
        args.append(QLatin1String("-f") % url.path());
    }
    qDebug() << args;
    KProcess process;
    process.setProgram(QStringLiteral("bluedevil-sendfile"), args);
    process.startDetached();
}

void SendFileItemAction::otherTriggered()
{
    qDebug();
    QStringList args;

    const QList<QUrl> &fileList = m_fileItemInfos.urlList();
    Q_FOREACH(const QUrl &url, fileList) {
        args.append(QLatin1String("-f") % url.path());
    }

    KProcess process;
    process.setProgram(QStringLiteral("bluedevil-sendfile"), args);
    process.startDetached();
}

#include "sendfileitemaction.moc"
