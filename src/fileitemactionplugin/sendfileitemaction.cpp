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
#include <QAction>
#include <QWidget>
#include <QVariantList>
#include <QStringBuilder>

#include <KProcess>
#include <KPluginFactory>
#include <KLocalizedString>

#include <QBluez/Manager>
#include <QBluez/InitManagerJob>
#include <QBluez/Adapter>
#include <QBluez/Device>
#include <QBluez/LoadDeviceJob>

K_PLUGIN_FACTORY_WITH_JSON(SendFileItemActionFactory,
                           "bluedevilsendfile.json",
                           registerPlugin<SendFileItemAction>();)

SendFileItemAction::SendFileItemAction(QObject *parent, const QVariantList &args)
    : KAbstractFileItemActionPlugin(parent)
{
    Q_UNUSED(args)

    // FIXME: This uses synchronous job->exec()

    // Initialize QBluez
    m_manager = new QBluez::Manager(this);
    QBluez::InitManagerJob *initJob = m_manager->init(QBluez::Manager::InitManagerAndAdapters);
    initJob->exec();
}

QList<QAction*> SendFileItemAction::actions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget)
{
    Q_UNUSED(parentWidget)

    QList<QAction*> list;

    m_fileItemInfos = fileItemInfos;

    if (!m_manager->isBluetoothOperational()) {
        return list;
    }

    QBluez::Adapter *adapter = m_manager->usableAdapter();

    QAction *menuAction = new QAction(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")), i18n("Send via Bluetooth"), this);
    QMenu *menu = new QMenu();

    // If we have configured devices, put them first
    Q_FOREACH (QBluez::Device *device, adapter->devices()) {
        if (!device->isLoaded()) {
            device->load()->exec();
        }
        if (device->uuids().contains(QLatin1String("00001105-0000-1000-8000-00805F9B34FB"), Qt::CaseInsensitive)) {
            QAction *action = new QAction(QIcon::fromTheme(device->icon()), device->name(), this);
            connect(action, SIGNAL(triggered(bool)), this, SLOT(deviceTriggered()));
            action->setData(device->ubi());
            menu->addAction(action);
        }
    }

    QAction *otherAction = new QAction(this);
    connect(otherAction, &QAction::triggered, this, &SendFileItemAction::otherTriggered);
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
    args.append(QLatin1String("-u") % static_cast<QAction*>(sender())->data().toString());

    const QList<QUrl> &fileList = m_fileItemInfos.urlList();
    Q_FOREACH(const QUrl &url, fileList) {
        args.append(QLatin1String("-f") % url.path());
    }

    qCDebug(FILEITEMACTION) << "Starting bluedevil-sendfile with args:" << args;

    KProcess process;
    process.setProgram(QStringLiteral("bluedevil-sendfile"), args);
    process.startDetached();
}

void SendFileItemAction::otherTriggered()
{
    QStringList args;

    const QList<QUrl> &fileList = m_fileItemInfos.urlList();
    Q_FOREACH(const QUrl &url, fileList) {
        args.append(QLatin1String("-f") % url.path());
    }

    qCDebug(FILEITEMACTION) << "Starting bluedevil-sendfile with args:" << args;

    KProcess process;
    process.setProgram(QStringLiteral("bluedevil-sendfile"), args);
    process.startDetached();
}

Q_LOGGING_CATEGORY(FILEITEMACTION, "BlueSendFileItemAction")

#include "sendfileitemaction.moc"
