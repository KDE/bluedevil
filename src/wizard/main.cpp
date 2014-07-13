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
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <KAboutData>
#include <KLocalizedString>

int main(int argc, char *argv[])
{
    KAboutData aboutData(QStringLiteral("bluedevilwizard"),
                         i18n("Bluetooth Wizard"),
                         bluedevil_version,
                         i18n("Bluetooth Wizard"),
                         KAboutLicense::GPL,
                         i18n("(c) 2010, UFO Coders"));

    aboutData.addAuthor(i18n("Alejandro Fiestas Olivares"), i18n("Maintainer"),
                        QStringLiteral("afiestas@kde.org"), QStringLiteral("http://www.afiestas.org/"));
    aboutData.setProgramIconName(QStringLiteral("preferences-system-bluetooth"));

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("bluedevilwizard"));
    app.setApplicationVersion(bluedevil_version);
    app.setApplicationDisplayName(i18n("Bluetooth Wizard"));
    app.setOrganizationDomain(QStringLiteral("kde.org"));
    app.setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Bluetooth Wizard"));
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("URL"), i18n("Device to pair with"), QStringLiteral("[URL]"));

    parser.process(app);

    const QStringList &args = parser.positionalArguments();
    QUrl url;
    if (!args.isEmpty()) {
        url = args.first();
    }

    new BlueWizard(url);

    return app.exec();
}
