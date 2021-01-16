/*
    SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef BLUEWIZARD_H
#define BLUEWIZARD_H

#include <QObject>
#include <QWizard>

#include <BluezQt/Manager>

class WizardAgent;

class BlueWizard : public QWizard
{
    Q_OBJECT

public:
    explicit BlueWizard();

    BluezQt::DevicePtr device() const;
    void setDevice(BluezQt::DevicePtr device);

    WizardAgent *agent() const;
    BluezQt::Manager *manager() const;

    enum {
        Discover,
        Pairing,
        Success,
        Fail,
        Connect,
    };

private Q_SLOTS:
    void initJobResult(BluezQt::InitManagerJob *job);
    void operationalChanged(bool operational);

private:
    void done(int result) override;

    BluezQt::Manager *m_manager;
    WizardAgent *m_agent;

    BluezQt::DevicePtr m_device;
};

#endif // BLUEWIZARD_H
