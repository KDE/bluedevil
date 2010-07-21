/*  This file is part of the KDE libraries

    Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
    Copyright (C) 2010 UFO Coders <info@ufocoders.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KIO_OBEXFTP_H
#define KIO_OBEXFTP_H

#include <kio/slavebase.h>

class KioFtp
    : public KIO::SlaveBase
{
public:
    KioFtp(const QByteArray &pool, const QByteArray &app);
    virtual ~KioFtp();

    virtual void copy(const KUrl &src, const KUrl &dest, int permissions, KIO::JobFlags flags);
    virtual void listDir(const KUrl &url);
    virtual void setHost(const QString &host, quint16 port, const QString &user, const QString &pass);
    virtual void slave_status();
    virtual void stat(const KUrl &url);
    virtual void del(const KUrl &url, bool isfile);
    virtual void mkdir(const KUrl&url, int permissions);

private:
    class Private;
    Private *const d;
};

#endif // KIO_OBEXFTP_H
