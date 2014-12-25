/*
 * Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
 * Copyright (C) 2010 UFO Coders <info@ufocoders.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef BLUEDEVIL_SYSTEM_CHECK_H
#define BLUEDEVIL_SYSTEM_CHECK_H
#include <QtCore/QObject>

class QVBoxLayout;

class KDED;
class KMessageWidget;

class SystemCheck : public QObject
{
    Q_OBJECT

public:
    SystemCheck(QWidget *parent);
    virtual ~SystemCheck();

    void createWarnings(QVBoxLayout *layout);

    bool checkKDEDModuleLoaded();
    bool checkNotificationsOK();
    KDED *kded();

Q_SIGNALS:
    void updateInformationStateRequest();

public Q_SLOTS:
    void updateInformationState();

private Q_SLOTS:
    void fixNoKDEDRunning();
    void fixNoUsableAdapterError();
    void fixNotDiscoverableAdapterError();
    void fixDisabledNotificationsError();

private:
    KDED        *m_kded;
    QWidget     *m_parent;
    KMessageWidget *m_noAdaptersError;
    KMessageWidget *m_noUsableAdapterError;
    KMessageWidget *m_noKDEDRunning;
    KMessageWidget *m_notDiscoverableAdapterError;
    KMessageWidget *m_disabledNotificationsError;
};

#endif //BLUEDEVIL_SYSTEM_CHECK_H
