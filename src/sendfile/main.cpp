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

#include "sendfilewizard.h"
#include "version.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <KAboutData>
#include <KLocalizedString>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

int main(int argc, char *argv[])
{
    KAboutData aboutData(QStringLiteral("bluedevilsendfile"),
                         i18n("Bluetooth Send File Helper"),
                         bluedevil_version,
                         i18n("Bluetooth Send File Helper"),
                         KAboutData::License_GPL,
                         i18n("(c) 2010, UFO Coders"));

    aboutData.addAuthor(i18n("Alejandro Fiestas Olivares"), i18n("Maintainer"),
                        QStringLiteral("afiestas@kde.org"), QStringLiteral("http://www.afiestas.org/"));
    aboutData.setProgramIconName("preferences-system-bluetooth");

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("bluedevilsendfile"));
    app.setApplicationVersion(bluedevil_version);
    app.setApplicationDisplayName(i18n("Bluetooth Send File Helper"));
    app.setOrganizationDomain(QStringLiteral("kde.org"));
    app.setQuitOnLastWindowClosed(false);

    QCommandLineOption kioOption({QLatin1String("kio"), QLatin1String("k")}, i18n("Device UUID where the files will be sent"));
    kioOption.setValueName(QStringLiteral("bluetooth://mac"));

    QCommandLineOption ubiOption({QLatin1String("ubi"), QLatin1String("u")}, i18n("Device UUID where the files will be sent"));
    ubiOption.setValueName(QStringLiteral("ubi"));

    QCommandLineOption filesOption({QLatin1String("files"), QLatin1String("f")}, i18n("Files that will be sent"));
    filesOption.setValueName(QStringLiteral("files"));

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Bluetooth Send File Helper"));
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(kioOption);
    parser.addOption(ubiOption);
    parser.addOption(filesOption);

    parser.process(app);

    QString deviceInfo = parser.value(ubiOption);
    if (deviceInfo.isEmpty()) {
        deviceInfo = parser.value(kioOption);
    }

    SendFileWizard *sendFileWizard = new SendFileWizard(deviceInfo, parser.values(filesOption));
    sendFileWizard->show();

    return app.exec();
}
