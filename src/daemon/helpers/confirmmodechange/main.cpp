/*************************************************************************************
 *  Copyright (C) 2010-2012 by Alejandro Fiestas Olivares <afiestas@kde.org>         *
 *  Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>                      *
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


#include "confirmmodechange.h"

#include <QtGui/QApplication>

#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>

static const KLocalizedString  description = ki18n("KDE Bluetooth System");

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    KComponentData data("bluedevil", "bluedevilconfirmmodechangehelper");
    KGlobal::setActiveComponent(data);
    ConfirmModeChange confirmMode;

    return app.exec();
}
