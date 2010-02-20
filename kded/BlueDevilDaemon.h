/***************************************************************************
 *   Copyright (C) 2008 by Eduardo Robles Elvira <edulix@gmail.com>        *
 *   Copyright (C) 2008 by Alex Fiestas <alex@eyeos.org>                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef BLUEDEVILDAEMON_H
#define BLUEDEVILDAEMON_H

#include <kdedmodule.h>
#include <KComponentData>
#include <QStringList>

class SuspensionLockHandler;

class KDE_EXPORT BlueDevilDaemon : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.BlueDevil")

public:
    BlueDevilDaemon(QObject *parent, const QList<QVariant>&);
    virtual ~BlueDevilDaemon();

private:
    void onlineMode();
    void offlineMode();
private Q_SLOTS:
    void adapterAdded(const QString&);
    void adapterRemoved(const QString&);
    void defaultAdapterChanged(const QString&);
    
private:
    struct Private;
    Private *d;
};

#endif /*BLUEDEVILDAEMON_H*/
