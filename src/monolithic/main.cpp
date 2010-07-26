/*
 *    Copyright (C) 2010 Alejandro Fiestas Olivares  <alex@ufocoders.com>
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "monolithic.h"
#include <KCmdLineArgs>
#include <kuniqueapplication.h>
#include <KAboutData>

int main(int argc, char *argv[])
{
    KAboutData aboutData("bluedevil_monolithic", "bluedevil", ki18n("BlueDevil"), "0.1", ki18n("Bluedevil helper"),
    KAboutData::License_GPL, ki18n("(c) 2010, Artesanos del Software"));

    aboutData.addAuthor(ki18n("Alex Fiestas"), ki18n("Developer"), "alex@ufocoders.com", "http://www.afiestas.org/");
    aboutData.addAuthor(ki18n("Rafael Fernández López"), ki18n("Developer"), "ereslibre@kde.org", "http://www.ereslibre.es/");
    aboutData.setProgramIconName("preferences-system-bluetooth");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KUniqueApplication app;
    app.setQuitOnLastWindowClosed(false);
    new Monolithic;

    return app.exec();
}