/*
    Copyright (C) 2010 Alejandro Fiestas Olivares  <alex@ufocoders.com>

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


#ifndef MONOLITIC_H
#define MONOLITIC_H

#include <kstatusnotifieritem.h>

namespace BlueDevil {
    class Adapter;
    class Device;
}

class KAction;

using namespace BlueDevil;

class Monolithic : public KStatusNotifierItem
{

Q_OBJECT
public:
    Monolithic(QObject* parent = 0);

public Q_SLOTS:
    void noAdapters(Adapter *adapter);
    void adapterAdded();

    void generateDeviceEntries();

    void addDevice();
    void configReceive();
    void deviceManager();
    void configAdapter();

private:
    void onlineMode();
    void offlineMode();
};

#endif // MONOLITIC_H
