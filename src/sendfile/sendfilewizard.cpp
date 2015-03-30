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

#include <QUrl>
#include <QPushButton>
#include <QApplication>

#include <KIO/JobTracker>
#include <KLocalizedString>
#include <KStatusBarJobTracker>

#include <BluezQt/Device>
#include <BluezQt/Adapter>
#include <BluezQt/PendingCall>
#include <BluezQt/InitManagerJob>

SendFileWizard::SendFileWizard(const QString &device, const QStringList &files)
    : QWizard()
    , m_deviceUrl(device)
    , m_files(files)
    , m_job(0)
{
    setWindowTitle(i18n("Bluetooth Send Files"));
    setOption(NoCancelButton, false);
    setButton(QWizard::NextButton, new QPushButton(QIcon::fromTheme(QStringLiteral("document-export")), i18n("Send Files")));
    setButton(QWizard::CancelButton, new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-cancel")), i18n("Cancel")));
    setOption(QWizard::DisabledBackButtonOnLastPage);
    setOption(QWizard::NoBackButtonOnStartPage);

    qCDebug(SENDFILE) << "Device" << m_deviceUrl;
    qCDebug(SENDFILE) << "Files" << m_files;

    // Initialize BluezQt
    m_manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *initJob = m_manager->init();
    initJob->start();
    connect(initJob, &BluezQt::InitManagerJob::result, this, &SendFileWizard::initJobResult);
}

SendFileWizard::~SendFileWizard()
{
    if (m_job) {
        m_job->doKill();
    }
}

void SendFileWizard::done(int result)
{
    qCDebug(SENDFILE) << "Wizard done: " << result;

    QWizard::done(result);

    if (!m_job) {
        qApp->quit();
    }
}

BluezQt::Manager *SendFileWizard::manager() const
{
    return m_manager;
}

QStringList SendFileWizard::files() const
{
    return m_files;
}

void SendFileWizard::setFiles(const QStringList &files)
{
    m_files = files;
}

BluezQt::DevicePtr SendFileWizard::device() const
{
    return m_device;
}

void SendFileWizard::setDevice(BluezQt::DevicePtr device)
{
    m_device = device;
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

    m_job = new SendFilesJob(m_files, m_device);
    connect(m_job, &SendFilesJob::destroyed, qApp, &QCoreApplication::quit);

    KIO::getJobTracker()->registerJob(m_job);
    m_job->start();

    done(1);
}

void SendFileWizard::initJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error()) {
        qCWarning(SENDFILE) << "Error initializing manager:" << job->errorText();
        qApp->exit();
        return;
    }

    qCDebug(SENDFILE) << "Manager initialized";

    // KIO address: bluetooth://40-87-2b-1a-39-28/00001105-0000-1000-8000-00805F9B34FB
    if (m_deviceUrl.startsWith(QLatin1String("bluetooth"))) {
        QString address = QUrl(m_deviceUrl).host();
        address.replace(QLatin1Char('-'), QLatin1Char(':'));
        m_device = m_manager->deviceForAddress(address.toUpper());
    } else {
        m_device = m_manager->deviceForUbi(m_deviceUrl);
    }

    // If the device's adapter is powered off, sending file would fail.
    // Let the user know about it!
    if (m_device && !m_device->adapter()->isPowered()) {
        m_device.clear();
    }

    if (m_device) {
        if (m_files.isEmpty()) {
            addPage(new SelectFilesPage(this));
        }
    } else {
        if (m_files.isEmpty()) {
            addPage(new SelectDeviceAndFilesPage(this));
        } else {
            addPage(new SelectDevicePage(this));
        }
    }

    addPage(new ConnectingPage());

    // Only show wizard after init is completed
    show();
}
