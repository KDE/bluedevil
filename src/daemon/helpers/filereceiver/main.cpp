/***************************************************************************
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@eyeos.org>        *
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

#include "service.h"
#include <QDebug>
#include <KComponentData>
#include <KCmdLineArgs>
#include <KApplication>
#include <KAboutData>

int main(int argc, char *argv[])
{
    KAboutData aboutData("bluedevilfilereceiverhelper", "bluedevilfilereceiverhelper", ki18n("BlueDevil File Receiver Helper"), "0.1", ki18n("Bluedevil File Receiver Helper"),
                         KAboutData::License_GPL, ki18n("(c) 2010, UFO coders"));

    aboutData.addAuthor(ki18n("Alex Fiestas"), ki18n("Developer"), "alex@eyeos.org",
        "http://www.afiestas.org/");
    aboutData.addAuthor(ki18n("Eduardo Robles Elvira"), ki18n("Developer"), "edulix@gmail.com",
        "http://blog.edulix.es/");
    aboutData.setProgramIconName("preferences-system-bluetooth");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;
    app.setQuitOnLastWindowClosed(false);
    new Service;

    return app.exec();
}
