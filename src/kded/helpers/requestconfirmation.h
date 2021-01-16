/*
 *   SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <alex@eyeos.org>
 *   SPDX-FileCopyrightText: 2010 Eduardo Robles Elvira <edulix@gmail.com>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *   SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef REQUESTCONFIRMATION_H
#define REQUESTCONFIRMATION_H

#include <QObject>

#include <BluezQt/Device>

class RequestConfirmation : public QObject
{
    Q_OBJECT

public:
    enum Result {
        Deny,
        Accept,
    };

    explicit RequestConfirmation(BluezQt::DevicePtr device, const QString &pin, QObject *parent = nullptr);

Q_SIGNALS:
    void done(Result result);

private Q_SLOTS:
    void pinCorrect();
    void pinWrong();

private:
    BluezQt::DevicePtr m_device;
    QString m_pin;
};

#endif // REQUESTCONFIRMATION_H
