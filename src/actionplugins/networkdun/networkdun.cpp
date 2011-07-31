/*************************************************************************************
 *  Copyright (C) 2010 by Alex Fiestas <alex@eyeos.org>                              *
 *  Copyright (C) 2010 UFO Coders <info@ufocoders.com>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "networkdun.h"
#include <KLocalizedString>
#include <KPluginFactory>
#include <KNotification>
#include <KIcon>
#include <KProcess>
#include <KStandardDirs>

#include <QDebug>

#include <bluedevil/bluedevildevice.h>

BLUEDEVILACTION_PLUGIN_EXPORT(NetworkDUNPlugin)

NetworkDUNPlugin::NetworkDUNPlugin(QObject* parent, const QVariantList& args)
    : ActionPlugin(parent)
{
    Q_UNUSED(args);
}

void NetworkDUNPlugin::startAction()
{
    QString deviceInfo;
    deviceInfo = deviceInfo.append("%1 %2").arg(device()->address()).arg("dun");

    QStringList args;
    args << "create" << "--type" <<  "bluetooth" << "--specific-args" << deviceInfo;
    KProcess p;
    p.setProgram(KStandardDirs::findExe("networkmanagement_configshell"), args);
    p.startDetached();

    KNotification::event(
        KNotification::Notification,
        i18n("%1: Setting up...", device()->friendlyName()),
        KIcon(device()->icon()).pixmap(48,48)
    )->sendEvent();

    emit finished();
}
