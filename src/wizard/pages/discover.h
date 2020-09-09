/*
    SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>

    SPDX-License-Identifier: GPL-3.0-or-later
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
    explicit DiscoverPage(BlueWizard *parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;
    int nextId() const override;

private Q_SLOTS:
    void indexSelected(const QModelIndex &index);
    void itemDoubleClicked(const QModelIndex &index);

    void usableAdapterChanged(BluezQt::AdapterPtr adapter);
    void checkAdapters();
    void fixAdaptersError();

private:
    BlueWizard *m_wizard;
    DevicesProxyModel *m_model;
    BluezQt::Manager *m_manager;
    BluezQt::AdapterPtr m_adapter;
    KMessageWidget *m_warningWidget;
};

#endif // DISCOVER_H
