/*
    Copyright (C) 2010 UFO Coders <info@ufocoders.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef SERVICEPLUGIN_H
#define SERVICEPLUGIN_H

#include <QObject>

#include <kdemacros.h>
#include <KPluginFactory>
#include <KPluginLoader>

#define BLUEDEVILSERVICE_PLUGIN_EXPORT( c ) \
  K_PLUGIN_FACTORY( BlueDevilService, registerPlugin< c >(); ) \
  K_EXPORT_PLUGIN( BlueDevilService("c") )

struct Private;
namespace BlueDevil {
    class Device;
}

using namespace BlueDevil;

class KDE_EXPORT ServicePlugin : public QObject
{
Q_OBJECT
public:
    ServicePlugin(QObject* parent = 0);

    virtual void connectService()=0;

    void setDevice(Device*);
    Device* device();

private:
    Private *d;

Q_SIGNALS:
    void finished();
};

Q_DECLARE_INTERFACE(ServicePlugin, "org.bluedevil.serviceplugin");
#endif // SERVICEPLUGIN_H
