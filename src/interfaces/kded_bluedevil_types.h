/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KDED_BLUEDEVIL_TYPES_H
#define KDED_BLUEDEVIL_TYPES_H

#include <QMetaType>

typedef QMap<QString, QString> DeviceInfo;
typedef QMap<QString, DeviceInfo> QMapDeviceInfo;
Q_DECLARE_METATYPE(DeviceInfo)
Q_DECLARE_METATYPE(QMapDeviceInfo)

#endif // KDED_BLUEDEVIL_TYPES_H
