/*
    Implementation of the client side of the OBEX FTP protocol.
    Copyright (c) 2010 Eduardo Robles Elvira <edulix@gmail.com>

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

#ifndef QOBEXFTP_H
#define QOBEXFTP_H

// Qt
#include <QtCore/QObject>

class QObexFtpPrivate;
class QUrlInfo;

/**
 * @short Implementation of the client side of the OBEX FTP protocol.
 * 
 * This class is a wrapper over the OpenOBEX ftp client library, using C++ and Qt, and it's pretty
 * similar to QFtp Qt class.
 */
class QObexFtp : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs a QObexFtp object with the given @a parent.
     */
    QObexFtp(QObject *parent = 0);

    /**
     * Destructor
     */
    virtual ~QObexFtp();

    /**
     * Gives information about a directory.
     */
    QUrlInfo stat(const QString &path);

    /**
     * Connects to the ObexFTP server @a host.
     *
     * @return less than zero means an error occurred an connection was not created.
     */
    int connectToHost(const QString &host);

    /**
     * Closes the connection with the server.
     */
    void close();

    /**
     * Lists a directory.
     */
    QList<QUrlInfo> list(const QString &pathToDir);

    /**
     * Retrieves a file from the server.
     */
    QByteArray get(const QString &pathToFile);

    /**
     * Writes a file onto the server.
     * 
     * @return less than zero means an error occurred.
     */
    int put(const QByteArray& data, const QString &pathToFile);

    /**
     * Writes a file or directory from the server.
     *
     * @return less than zero means an error occurred.
     */
    int remove(const QString &path);

    /**
     * Renames a file or directory in the server.
     *
     * @return less than zero means an error occurred.
     */
    int rename(const QString &originPath, const QString &destinationPath);

    /**
     * Returns whether we are currently connected to the server or not
     */
    bool isConnected() const;

    /**
     * Creates a directory.
     */
    int mkdir(const QString &path);

    QString host() const;

private:
    QObexFtpPrivate * const d;
};

#endif // QOBEXFTP_H
