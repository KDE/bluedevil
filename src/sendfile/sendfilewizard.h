/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SENDFILEWIZARD_H
#define SENDFILEWIZARD_H

#include <QObject>
#include <QStringList>
#include <QWizard>

#include <BluezQt/Manager>

#include "discoverwidget.h"

class QDBusObjectPath;

class SendFilesJob;

class SendFileWizard : public QWizard
{
    Q_OBJECT

public:
    explicit SendFileWizard(const QString &device, const QStringList &files);
    ~SendFileWizard() override;

    void done(int result) override;

    BluezQt::Manager *manager() const;

    QStringList files() const;
    void setFiles(const QStringList &files);

    BluezQt::DevicePtr device() const;
    void setDevice(BluezQt::DevicePtr device);

    void startTransfer(const QDBusObjectPath &session);

private Q_SLOTS:
    void initJobResult(BluezQt::InitManagerJob *job);

private:
    QString m_deviceUrl;
    QStringList m_files;

    BluezQt::Manager *m_manager;
    BluezQt::DevicePtr m_device;
    SendFilesJob *m_job;
};

#endif // SENDFILEWIZARD_H
