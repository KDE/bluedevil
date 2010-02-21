/*  This file is part of the KDE project

    Copyright (C) 2010  Alex Fiestas <alex@eyeos.org>
    Copyright (C) 2010 by Eduardo Robles Elvira <edulix@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "requestconfirmation.h"
#include <QDebug>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <solid/control/bluetoothmanager.h>
#include <QApplication>

static const KLocalizedString  description = ki18n("Small application part of BlueDevil");

int main(int argc, char *argv[])
{
    KAboutData aboutData("BlueDevil-Authorize",
                "bluedevil",
                ki18n("BlueDevil"),
                "1.0",
                description,
                KAboutData::License_GPL,
                ki18n("(c) 2008-2009, Artesanos del Sotware")
                );
    aboutData.addAuthor(ki18n("Alex Fiestas"), ki18n("Maintainer"), "alex@eyeos.org", "http://www.afiestas.org/");

//     #warning Check the arguments

    QApplication app(argc,argv);
    RequestConfirmation *confirmation = new RequestConfirmation;

    return app.exec();
}
