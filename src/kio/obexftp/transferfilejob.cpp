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

#include <QDBusConnection>

#include <KDebug>
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
    return m_parent->cancelTransfer(m_path);
}

void TransferFileJob::createObjects()
{
    m_transfer = new TransferInterface("org.bluez.obex", m_path, QDBusConnection::sessionBus());
    m_properties = new PropertiesInterface("org.bluez.obex", m_path, QDBusConnection::sessionBus());

    connect(m_properties, SIGNAL(PropertiesChanged(QString,QVariantMap,QStringList)), SLOT(propertiesChanged(QString,QVariantMap,QStringList)));
}

void TransferFileJob::propertiesChanged(const QString& interface, const QVariantMap& properties, const QStringList &invalidProps)
{
    Q_UNUSED(invalidProps)
    kDebug() << properties;
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
    kDebug() << value;
    QString status = value.toString();

    if (status == QLatin1String("active")) {
        m_time = QTime::currentTime();
        return;
    } else if (status == QLatin1String("complete")) {
        emitResult();
        return;
    } else if (status == QLatin1String("error")) {
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }

    kDebug() << "Not implemented status: " << status;
}

void TransferFileJob::transferChanged(const QVariant& value)
{
    kDebug() << "Transferred: " << value;
    if (m_parent->wasKilled()) {
        kDebug() << "Kio was killed, aborting task";
        doKill();
        emitResult();
        return;
    }

    bool ok = false;
    qulonglong bytes = value.toULongLong(&ok);
    if (!ok) {
        kWarning() << "Couldn't cast transferChanged value" << value;
        return;
    }

    //If a least 1 second has passed since last update
    int secondsSinceLastTime = m_time.secsTo(QTime::currentTime());
    if (secondsSinceLastTime > 0) {
        float speed = (bytes - m_speedBytes) / secondsSinceLastTime;

        m_parent->speed(speed);
        m_time = QTime::currentTime();
        m_speedBytes = bytes;
    }

    m_parent->processedSize(bytes);
}
