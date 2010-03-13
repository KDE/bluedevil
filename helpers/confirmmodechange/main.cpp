/*  This file is part of the KDE project

    Copyright (C) 2010  Alex Fiestas <alex@eyeos.org>
    Copyright (C) 2010 by Eduardo Robles Elvira <edulix@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "confirmmodechange.h"
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <solid/control/bluetoothmanager.h>
#include <QApplication>

static const KLocalizedString  description = ki18n("KDE Bluetooth System");

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    KComponentData data("bluedevil", "bluedevil");
    KGlobal::setActiveComponent(data);
    ConfirmModeChange *auth = new ConfirmModeChange;

    return app.exec();
}
