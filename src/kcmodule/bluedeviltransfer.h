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

#ifndef _BLUEDEVILTRANSFER_H
#define _BLUEDEVILTRANSFER_H

#include <kcmodule.h>

class SystemCheck;
namespace Ui {
    class Transfer;
}
namespace QBluez {
    class Manager;
    class Adapter;
    class InitManagerJob;
}

class KCMBlueDevilTransfer : public KCModule
{
    Q_OBJECT

public:
    explicit KCMBlueDevilTransfer(QWidget *parent, const QVariantList&);

    void save() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void initJobResult(QBluez::InitManagerJob *job);
    void usableAdapterChanged(QBluez::Adapter *adapter);
    void adapterDiscoverableChanged();
    void updateInformationState();
    void showSharedFilesDialog();
    void slotChanged(bool);

private:
    QBluez::Manager *m_manager;
    SystemCheck *m_systemCheck;
    Ui::Transfer *m_uiTransfer;
    bool m_restartNeeded;
};

#endif
