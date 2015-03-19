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
#include <KDBusService>
#include <KLocalizedString>

int main(int argc, char *argv[])
{
    KAboutData aboutData(QStringLiteral("bluedevilsendfile"),
                         i18n("Bluetooth Send File Helper"),
                         bluedevil_version,
                         i18n("Bluetooth Send File Helper"),
                         KAboutLicense::GPL,
                         i18n("(c) 2010, UFO Coders"));

    aboutData.addAuthor(i18n("Alejandro Fiestas Olivares"), i18n("Maintainer"),
                        QStringLiteral("afiestas@kde.org"), QStringLiteral("http://www.afiestas.org/"));

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")));
    app.setQuitOnLastWindowClosed(false);

    KAboutData::setApplicationData(aboutData);
    KDBusService service;

    QCommandLineOption kioOption(QStringList() << QStringLiteral("kio") << QStringLiteral("k"));
    kioOption.setValueName(QStringLiteral("bluetooth://mac"));

    QCommandLineOption ubiOption(QStringList() << QStringLiteral("ubi") << QStringLiteral("u"));
    ubiOption.setValueName(QStringLiteral("ubi"));

    QCommandLineOption filesOption(QStringList() << QStringLiteral("files") << QStringLiteral("f"));
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

    SendFileWizard *wizard = new SendFileWizard(deviceInfo, parser.values(filesOption));

    QObject::connect(&service, &KDBusService::activateRequested, wizard, [wizard]() {
        wizard->setWindowState((wizard->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    });

    return app.exec();
}
