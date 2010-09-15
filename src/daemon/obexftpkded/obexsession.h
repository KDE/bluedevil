/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#ifndef OBEXSESSION_H
#define OBEXSESSION_H

#include "obexftpsession.h"
#include "obexftpmanager.h"

class ObexSession : public OrgOpenobexSessionInterface
{
    public:
        ObexSession(const QString& service, const QString& path, const QDBusConnection& connection, QObject* parent = 0);

        enum Status {
            Connected = 0,
            Connecting = 1
        };

        Status getStatus() const;
        void setStatus(const Status&);

    private:
        Status m_status;

};

#endif // OBEXSESSION_H
