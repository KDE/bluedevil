/*****************************************************************************
 * This file is part of the KDE project                                      *
 *                                                                           *
 * Copyright (C) 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>     *
 * Copyright (C) 2010-2011 UFO Coders <info@ufocoders.com>                   *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#ifndef SENDFILEWIZARD_H
#define SENDFILEWIZARD_H

#include <QObject>
#include <QWizard>
#include <QStringList>

#include <BluezQt/Manager>

#include "discoverwidget.h"

class SendFilesJob;

class SendFileWizard : public QWizard
{
    Q_OBJECT

public:
    explicit SendFileWizard(const QString &device, const QStringList &files);
    ~SendFileWizard();

    void done(int result) Q_DECL_OVERRIDE;

    BluezQt::Manager *manager() const;

    QStringList files() const;
    void setFiles(const QStringList &files);

    BluezQt::DevicePtr device() const;
    void setDevice(BluezQt::DevicePtr device);

    void startTransfer();

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
