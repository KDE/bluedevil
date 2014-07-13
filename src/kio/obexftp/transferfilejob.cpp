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

#include "kio_obexftp.h"
#include "transferfilejob.h"
#include "obexd_transfer.h"
#include "debug_p.h"

#include <QDebug>
#include <QDBusConnection>

#include <KLocalizedString>

typedef OrgBluezObexTransfer1Interface TransferInterface;
typedef OrgFreedesktopDBusPropertiesInterface PropertiesInterface;

TransferFileJob::TransferFileJob(const QString& path, KioFtp* parent)
    : KJob(parent)
    , m_path(path)
    , m_speedBytes(0)
    , m_parent(parent)
{

}

TransferFileJob::~TransferFileJob()
{
    delete m_transfer;
    delete m_properties;
}

void TransferFileJob::start()
{
    QMetaObject::invokeMethod(this, "createObjects", Qt::QueuedConnection);
}

bool TransferFileJob::doKill()
{
    QDBusPendingReply <void > reply = m_transfer->Cancel();
    reply.waitForFinished();

    return !reply.isError();
}

void TransferFileJob::setSize(int size)
{
    qCDebug(OBEXFTP) << size;
    m_parent->totalSize(size);
}

void TransferFileJob::createObjects()
{
    m_transfer = new TransferInterface(QStringLiteral("org.bluez.obex"), m_path, QDBusConnection::sessionBus());
    m_properties = new PropertiesInterface(QStringLiteral("org.bluez.obex"), m_path, QDBusConnection::sessionBus());

    connect(m_properties, SIGNAL(PropertiesChanged(QString,QVariantMap,QStringList)), SLOT(propertiesChanged(QString,QVariantMap,QStringList)));
}

void TransferFileJob::propertiesChanged(const QString& interface, const QVariantMap& properties, const QStringList &invalidProps)
{
    Q_UNUSED(invalidProps)
    qCDebug(OBEXFTP) << properties;
    if (interface != QLatin1String("org.bluez.obex.Transfer1")) {
        return;
    }

    QStringList changedProps = properties.keys();
    Q_FOREACH(const QString &prop, changedProps) {
        if (prop == QLatin1String("Status")) {
            statusChanged(properties.value(prop));
        } else if (prop == QLatin1String("Transferred")) {
            transferChanged(properties.value(prop));
        }
    }
}

void TransferFileJob::statusChanged(const QVariant& value)
{
    qCDebug(OBEXFTP) << value;
    QString status = value.toString();

    if (status == QLatin1String("active")) {
        m_time = QTime::currentTime();
        return;
    } else if (status == QLatin1String("complete")) {
        m_parent->finished();
        emitResult();
        return;
    } else if (status == QLatin1String("error")) {
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }

    qCDebug(OBEXFTP) << "Not implemented status: " << status;
}

void TransferFileJob::transferChanged(const QVariant& value)
{
    qCDebug(OBEXFTP) << "Transferred: " << value;
    if (m_parent->wasKilled()) {
        qCDebug(OBEXFTP) << "Kio was killed, aborting task";
        m_transfer->Cancel().waitForFinished();
        emitResult();
        return;
    }

    bool ok = false;
    qulonglong bytes = value.toULongLong(&ok);
    if (!ok) {
        qCWarning(OBEXFTP) << "Couldn't cast transferChanged value" << value;
        return;
    }

    // If at least 1 second has passed since last update
    int secondsSinceLastTime = m_time.secsTo(QTime::currentTime());
    if (secondsSinceLastTime > 0) {
        float speed = (bytes - m_speedBytes) / secondsSinceLastTime;

        m_parent->speed(speed);
        m_time = QTime::currentTime();
        m_speedBytes = bytes;
    }

    m_parent->processedSize(bytes);
}
