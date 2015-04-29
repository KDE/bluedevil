/***************************************************************************
 *   Copyright (C) 2010-2013 Alejandro Fiestas Olivares <afiestas@kde.org> *
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

#include "bluewizard.h"
#include "version.h"

#include <QUrl>
#include <QIcon>
#include <QApplication>
#include <QCommandLineParser>

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedString>

int main(int argc, char *argv[])
{
    KAboutData aboutData(QStringLiteral("bluedevilwizard"),
                         i18n("Bluetooth Wizard"),
                         BLUEDEVIL_VERSION,
                         i18n("Bluetooth Wizard"),
                         KAboutLicense::GPL,
                         i18n("(c) 2010, UFO Coders"));

    aboutData.addAuthor(QStringLiteral("David Rosca"), i18n("Maintainer"),
                        QStringLiteral("nowrep@gmail.com"), QStringLiteral("http://david.rosca.cz"));

    aboutData.addAuthor(QStringLiteral("Alejandro Fiestas Olivares"), i18n("Previous Maintainer"),
                        QStringLiteral("afiestas@kde.org"), QStringLiteral("http://www.afiestas.org/"));

    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")));
    app.setQuitOnLastWindowClosed(false);

    KAboutData::setApplicationData(aboutData);
    KDBusService service(KDBusService::Unique);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    BlueWizard *wizard = new BlueWizard;

    QObject::connect(&service, &KDBusService::activateRequested, wizard, [wizard]() {
        wizard->setWindowState((wizard->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    });

    return app.exec();
}
