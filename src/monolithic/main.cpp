/*************************************************************************************
 *  Copyright (C) 2010-2012 by Alejandro Fiestas Olivares <afiestas@kde.org>         *
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

#include "monolithic.h"
#include "version.h"
#include <KCmdLineArgs>
#include <kuniqueapplication.h>
#include <KAboutData>

int main(int argc, char *argv[])
{
    KAboutData aboutData("bluedevilmonolithic", "bluedevil", ki18n("Bluetooth"), bluedevil_version, ki18n("Bluetooth"),
    KAboutData::License_GPL, ki18n("(c) 2010, UFO Coders"));

    aboutData.addAuthor(ki18n("Alejandro Fiestas Olivares"), ki18n("Maintainer"), "afiestas@kde.org", "http://www.afiestas.org/");
    aboutData.addAuthor(ki18n("Rafael Fernández López"), ki18n("Developer"), "ereslibre@kde.org", "http://www.ereslibre.es/");
    aboutData.setProgramIconName("preferences-system-bluetooth");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KUniqueApplication app;
    app.setQuitOnLastWindowClosed(false);
    new Monolithic;

    return app.exec();
}
