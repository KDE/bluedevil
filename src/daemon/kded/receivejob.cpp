/***************************************************************************
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <afiestas@kde.org>      *
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


#include "receivejob.h"
#include "obexd_transfer.h"

#include <QTime>

#include <KDebug>
#include <KLocale>

ReceiveJob::ReceiveJob(const QString& path, const QString& dest, const QString& from, const int length, QObject* parent)
{
    m_from = from;
    m_dest = dest;
    m_length = length;
    m_transfer = new OrgOpenobexTransferInterface ("org.openobex", path, QDBusConnection::sessionBus());
    connect(m_transfer, SIGNAL(Progress(int,int)), SLOT(Progress(int,int)));

    setCapabilities(Killable);
}

void ReceiveJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void ReceiveJob::failed()
{
    setError(UserDefinedError);
    setErrorText(i18nc("Shown when the file transfer fails", "Transfer failed or cancelled"));
    emitResult();
}

void ReceiveJob::completed()
{
    setProcessedAmount(Bytes, m_length);
    emitResult();
}

void ReceiveJob::Progress(int total, int transfer)
{
    qulonglong lastProcessed = processedAmount(Bytes);
    setProcessedAmount(Bytes, transfer);

    if (m_time.isNull()) {
        m_time = QTime::currentTime();
        return;
    }

    int time = m_time.secsTo(QTime::currentTime());
    if (time <= 0) {
        return;
    }

    qulonglong diff = transfer - lastProcessed;
    float speed = diff / time;

    emitSpeed(speed);

    m_time = QTime::currentTime();
}

bool ReceiveJob::doKill()
{
    m_transfer->Cancel();
    return true;
}

void ReceiveJob::doStart()
{
    QString title(i18n("Receiving file over Bluetooth"));
    QPair<QString, QString> fromLine(i18nc("File transfer origin", "From"), QString(m_from));
    QPair<QString, QString> toLine (i18nc("File transfer destination", "To"), m_dest);

    emit description(this, title, fromLine, toLine);

    //In the cto we're not registered into the tracker, so we need to send this info again
    setTotalAmount(Bytes, m_length);
    setProcessedAmount(Bytes, 0);
}
