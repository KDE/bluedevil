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
    /**
     * Stablish basics connections with solid siganls and calls online if interfaces are availables
     */
    BlueDevilDaemon(QObject *parent, const QList<QVariant>&);
    virtual ~BlueDevilDaemon();

private:
    /**
     * Called by constructor or eventually by adapterAdded initialize all the helpers
     * @see helpers
     */
    void onlineMode();

    /**
     * Called eventually adapterRemoved shutdown all the helpers
     * @see helpers
     */
    void offlineMode();

private Q_SLOTS:
    /**
     * Called when a new adapter is available calls onlineMode if needed
     */
    void adapterAdded(const QString&);

    /**
     * Called when an adapter is removed calls offlineMode if needed
     */
    void adapterRemoved(const QString&);

    /**
     * Called when the default adapter changes, re-initialize the kded with the new
     * default adapter
     */
    void defaultAdapterChanged(const QString&);

    /**
     * AgentListner is a QThread, so we've to delete it after call QThread::quit();
     */
    void agentThreadStopped();

    /**
     * When the agent is released this is called to unload it
     */
    void agentReleased();

private:
    /**
     * Tries to start the helper process via dbus and returns true if successful
     */
    bool serviceStarted();
private:
    struct Private;
    Private *d;
};

#endif /*BLUEDEVILDAEMON_H*/
