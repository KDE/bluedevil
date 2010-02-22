/***************************************************************************
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
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
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <solid/control/bluetoothmanager.h>
#include <QApplication>

static const KLocalizedString  description = ki18n("Small application part of BlueDevil");

int main(int argc, char *argv[])
{
    KAboutData aboutData("BlueDevil-Helper",
        "bluedevil-helper",
        ki18n("BlueDevil Helper"),
        "1.0",
        description,
        KAboutData::License_GPL,
        ki18n("(c) 2010, Artesanos del Sotware")
    );

    aboutData.addAuthor(ki18n("Eduardo Robles Elvira"), ki18n("Maintainer"), "edulix@gmail.com",
        "http://blog.edulix.es/");

    QApplication app(argc,argv);
    Service *helperService = new Service;

    return app.exec();
}
