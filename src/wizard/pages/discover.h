/*
    Copyright (C) 2010 UFO Coders <info@ufocoders.com>

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

#ifndef DISCOVER_H
#define DISCOVER_H

#include "ui_discover.h"

#include <QWizardPage>

#include <BluezQt/Types>

class KMessageWidget;

class BlueWizard;
class DevicesProxyModel;

class DiscoverPage : public QWizardPage, public Ui::Discover
{
    Q_OBJECT

public:
    explicit DiscoverPage(BlueWizard *parent = 0);

    void startDiscovery();

    void initializePage() Q_DECL_OVERRIDE;
    bool isComplete() const Q_DECL_OVERRIDE;
    int nextId() const Q_DECL_OVERRIDE;

private Q_SLOTS:
    void indexSelected(const QModelIndex &index);

    void usableAdapterChanged(BluezQt::AdapterPtr adapter);
    void checkAdapters();
    void fixAdaptersError();

private:
    BlueWizard *m_wizard;
    DevicesProxyModel *m_model;
    KMessageWidget *m_warningWidget;
};

#endif // DISCOVER_H
