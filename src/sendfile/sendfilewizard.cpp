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

#include "sendfilewizard.h"
#include "sendfilesjob.h"
#include "debug_p.h"

#include "pages/selectdeviceandfilespage.h"
#include "pages/selectdevicepage.h"
#include "pages/selectfilespage.h"
#include "pages/connectingpage.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QUrl>

#include <kio/jobclasses.h>
#include <kstandardguiitem.h>
#include <klocalizedstring.h>
#include <kstatusbarjobtracker.h>

#include <QBluez/Manager>
#include <QBluez/InitManagerJob>
#include <QBluez/Adapter>
#include <QBluez/Device>

SendFileWizard::SendFileWizard(const QString &deviceUrl, const QStringList &files)
    : QWizard()
    , m_deviceUrl(deviceUrl)
    , m_files(files)
    , m_manager(0)
    , m_device(0)
    , m_job(0)
{
    setWindowTitle(i18n("Bluetooth Send Files"));
    setOption(NoCancelButton, false);
    setButton(QWizard::NextButton, new QPushButton(QIcon::fromTheme(QStringLiteral("document-export")), i18n("Send Files")));
    setButton(QWizard::CancelButton, new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-cancel")), i18n("Cancel")));
    setOption(QWizard::DisabledBackButtonOnLastPage);
    setOption(QWizard::NoBackButtonOnStartPage);

    qCDebug(SENDFILE) << "DeviceUrl: " << m_deviceUrl;
    qCDebug(SENDFILE) << "Files" << m_files;

    // Initialize QBluez
    m_manager = new QBluez::Manager(this);
    QBluez::InitManagerJob *initJob = m_manager->init();
    initJob->start();
    connect(initJob, &QBluez::InitManagerJob::result, this, &SendFileWizard::initResult);
}

SendFileWizard::~SendFileWizard()
{
    if (m_job) {
        m_job->doKill();
    }
}

void SendFileWizard::done(int result)
{
    QWizard::done(result);
    if (!m_job) {
        qApp->quit();
    }
}

void SendFileWizard::setFiles(const QStringList &files)
{
    qCDebug(SENDFILE) << files;
    m_files = files;
}

QBluez::Manager *SendFileWizard::manager() const
{
    return m_manager;
}

void SendFileWizard::setDevice(QBluez::Device* device)
{
    qCDebug(SENDFILE) << device;
    m_device = device;
}

QBluez::Device* SendFileWizard::device() const
{
    return m_device;
}

void SendFileWizard::startTransfer()
{
    if (m_files.isEmpty()) {
        qCDebug(SENDFILE) << "No files to send";
        return;
    }
    if (!m_device) {
        qCDebug(SENDFILE) << "No device selected";
    }

    m_device->adapter()->stopDiscovery();

    m_job = new SendFilesJob(m_files, m_device);
    connect(m_job, SIGNAL(destroyed(QObject*)), qApp, SLOT(quit()));

    KIO::getJobTracker()->registerJob(m_job);
    m_job->start();

    QTimer::singleShot(2000, this, SLOT(wizardDone()));
}

void SendFileWizard::initResult(QBluez::InitManagerJob *job)
{
    if (job->error()) {
        qCDebug(SENDFILE) << "Error initializing manager:" << job->errorText();
        qApp->exit();
        return;
    }

    if (!m_manager->isBluetoothOperational()) {
        qCDebug(SENDFILE) << "Bluetooth not operational!";
        qApp->exit();
        return;
    }

    if (m_deviceUrl.startsWith(QLatin1String("bluetooth"))) {
        m_deviceUrl.remove(QStringLiteral("bluetooth:"));
        m_deviceUrl.replace(QLatin1Char(':'), QLatin1Char('-'));
        m_deviceUrl.prepend(QLatin1String("bluetooth:"));
        QUrl url(m_deviceUrl);
        m_device = m_manager->deviceForAddress(url.host().replace(QLatin1Char('-'), QLatin1Char(':')));
    } else {
        m_device = m_manager->deviceForUbi(m_deviceUrl);
    }

    if (m_device) {
        if (m_files.isEmpty()) {
            addPage(new SelectFilesPage(this));
        } else {
            setFiles(m_files);
        }
    } else {
        if (m_files.isEmpty()) {
            addPage(new SelectDeviceAndFilesPage(this));
        } else {
            addPage(new SelectDevicePage(this));
            setFiles(m_files);
        }
    }

    addPage(new ConnectingPage(this));

    // Only show wizard after init is completed
    show();
}

void SendFileWizard::wizardDone()
{
    done(1);
}
