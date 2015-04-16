/*************************************************************************************
 *  Copyright (C) 2013 by Alejandro Fiestas Fiestas <afiestas@kde.org>               *
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

#ifndef TRANSFERFILEJOB_H
#define TRANSFERFILEJOB_H

#include <QTime>

#include <KJob>

#include <BluezQt/ObexTransfer>

class KioFtp;

class TransferFileJob : public KJob
{
    Q_OBJECT

public:
    explicit TransferFileJob(BluezQt::ObexTransferPtr transfer, KioFtp *parent = 0);

    void start() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void doStart();
    void statusChanged(BluezQt::ObexTransfer::Status status);
    void transferredChanged(quint64 transferred);

private:
    QTime m_time;
    qlonglong m_speedBytes;
    KioFtp *m_parent;
    BluezQt::ObexTransferPtr m_transfer;
};

#endif // TRANSFERFILEJOB_H
