/***************************************************************************
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@eyeos.org>        *
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
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

#ifndef OBEXFTPDAEMON_H
#define OBEXFTPDAEMON_H

#include <obexftpmanager.h>
#include <obexftpsession.h>

#include <QDBusObjectPath>
#include <kdedmodule.h>

namespace BlueDevil {
    class Adapter;
};
using namespace BlueDevil;

class KDE_EXPORT ObexFtpDaemon
    : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ObexFtp")

public:
    /**
     * Stablish basics connections with libbluedevil signals and calls online if interfaces are availables
     */
    ObexFtpDaemon(QObject *parent, const QList<QVariant>&);
    virtual ~ObexFtpDaemon();

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

    void changeCurrentFolder(QString address, QString path);

    QString getAddressFromSession(QString path);

    QString cleanAddress(QString& dirtyAddress) const;
private:
    struct Private;
    Private *d;

private Q_SLOTS:
    /**
     * Called when the default adapter changes, re-initialize the kded with the new
     * default adapter
     */
    void defaultAdapterChanged(Adapter *adapter);

    void SessionConnected(QDBusObjectPath path);

    void SessionClosed(QDBusObjectPath path);

    void sessionDisconnected();

//Dbus interface
public Q_SLOTS:
    /**
     * Stablish a connection to the given address.
     * When the session is connected, the "sessionConnected" signal will be emitted
     */
    Q_SCRIPTABLE void stablishConnection(QString address);

    Q_SCRIPTABLE QString listDir(QString address, QString path);

    Q_SCRIPTABLE void copyRemoteFile(QString address, QString fileName, QString destPath);

    Q_SCRIPTABLE void sendFile(QString address, QString localPath, QString destPath);

    Q_SCRIPTABLE void createFolder(QString address, QString path);

    Q_SCRIPTABLE void deleteRemoteFile(QString address, QString path);

    Q_SCRIPTABLE bool isBusy(QString address);
    Q_SCRIPTABLE void Cancel(QString address);

Q_SIGNALS:
    Q_SCRIPTABLE void sessionConnected(QString address);
    Q_SCRIPTABLE void sessionClosed(QString address);
    Q_SCRIPTABLE void Cancelled();
    Q_SCRIPTABLE void transferProgress(qulonglong);
    Q_SCRIPTABLE void transferCompleted();
    Q_SCRIPTABLE void errorOccurred(QString,QString);
};
#endif /*OBEXFTPDAEMON_H*/
