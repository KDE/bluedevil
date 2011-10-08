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


#include "actionplugin.h"

#include "bluedevil/bluedevildevice.h"

using namespace BlueDevil;

struct Private {
    Device *device;
};

ActionPlugin::ActionPlugin(QObject* parent): QObject(parent), d(new Private)
{}

void ActionPlugin::setDevice(Device* device)
{
    connect(device, SIGNAL(destroyed(QObject*)), this, SLOT(deviceDestroyed()));
    d->device= device;
}

Device* ActionPlugin::device()
{
    return d->device;
}

void ActionPlugin::deviceDestroyed()
{
    d->device = 0;
    emit finished();
}