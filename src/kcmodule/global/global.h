/*
 * Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
 * Copyright (C) 2010 UFO Coders <info@ufocoders.com>
 * Copyright (C) 2015 David Rosca <nowrep@gmail.com>
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

#ifndef BLUEDEVILGLOBAL_H
#define BLUEDEVILGLOBAL_H

#include <KCModule>

#include <BluezQt/Manager>

namespace Ui
{
    class Global;
}

class SystemCheck;

class KCMBlueDevilGlobal : public KCModule
{
    Q_OBJECT

public:
    KCMBlueDevilGlobal(QWidget *parent, const QVariantList&);

    void save() override;

private Q_SLOTS:
    void initJobResult(BluezQt::InitManagerJob *job);
    void receiveFilesChanged(bool enable);
    void enableBluetoothChanged(bool enable);

private:
    SystemCheck *m_systemCheck;
    Ui::Global *m_ui;
    BluezQt::Manager *m_manager;
    bool m_isEnabled;
};

#endif // BLUEDEVILGLOBAL_H
