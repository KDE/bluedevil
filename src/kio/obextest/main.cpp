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


#include "obextest.h"
#include <KCmdLineArgs>
#include <KApplication>
#include <KAboutData>

int main(int argc, char *argv[])
{
    KAboutData aboutData("obextest", 0, ki18n("Obex Test"), "0.1", ki18n("Obex Test"),
    KAboutData::License_GPL, ki18n("(c) 2010, UFO Coders"));

    aboutData.addAuthor(ki18n("Alejandro Fiestas Olivares"), ki18n("Developer"), "alex@ufocoders.com",
    "http://www.afiestas.org/");
    aboutData.setProgramIconName("preferences-system-bluetooth");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;
    app.setQuitOnLastWindowClosed(false);
    new ObexTest;

    return app.exec();
}
