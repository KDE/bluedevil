/***************************************************************************
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@ufocoders.com>    *
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

#include "sendfilewizard.h"
#include <KCmdLineArgs>
#include <KApplication>
#include <KAboutData>
#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

int main(int argc, char *argv[])
{
    KAboutData aboutData("bluedevilsendfile", "bluedevil", ki18n("Bluetooth Send File Helper"), "0.1", ki18n("Bluetooth Send File Helper"),
    KAboutData::License_GPL, ki18n("(c) 2010, UFO Coders"));

    aboutData.addAuthor(ki18n("Alejandro Fiestas Olivares"), ki18n("Developer"), "alex@ufocoders.org",
    "http://www.afiestas.org/");
    aboutData.setProgramIconName("preferences-system-bluetooth");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("+[URL]", ki18n("Device to send files to"));
    KCmdLineArgs::addCmdLineOptions( options );

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QString wizardArg;
    if (args->count()) {
        wizardArg = KUrl(args->arg(0)).host().replace('-', ':');
    }

    KApplication app;
    app.setQuitOnLastWindowClosed(false);

    SendFileWizard *sendFileWizard = new SendFileWizard(wizardArg);
    sendFileWizard->setDevice(Manager::self()->defaultAdapter()->deviceForAddress(wizardArg));
    sendFileWizard->show();

    return app.exec();
}
