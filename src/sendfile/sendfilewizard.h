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
#include "discoverwidget.h"

class WizardAgent;
class QStringList;
class SendFilesJob;
namespace BlueDevil {
    class Device;
}
using namespace BlueDevil;

class SendFileWizard : public QWizard
{
Q_OBJECT

public:
    SendFileWizard(const QString &deviceUBI, const QStringList &files);
    virtual ~SendFileWizard();

    virtual void done(int result);

    void setFiles(const QStringList &files);

    void setDevice(Device *device);
    void setDevice(QString deviceUrl);
    Device* device();

    void startTransfer();

private Q_SLOTS:
    void wizardDone();

private:
    QStringList  m_files;

    Device       *m_device;
    SendFilesJob *m_job;
};

#endif // SENDFILEWIZARD_H
