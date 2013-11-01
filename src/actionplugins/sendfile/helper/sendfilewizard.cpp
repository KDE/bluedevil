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

#include "pages/selectdeviceandfilespage.h"
#include "pages/selectdevicepage.h"
#include "pages/selectfilespage.h"
#include "pages/connectingpage.h"

#include <QApplication>
#include <QTimer>

#include <kdebug.h>
#include <kstandardguiitem.h>
#include <klocalizedstring.h>
#include <kpushbutton.h>
#include <kstatusbarjobtracker.h>
#include <kfiledialog.h>

#include <bluedevil/bluedevil.h>
#include <sendfilesjob.h>

using namespace BlueDevil;

SendFileWizard::SendFileWizard(const QString& deviceInfo, const QStringList& files)
    : QWizard()
    , m_device(0)
    , m_job(0)
{
    if (!BlueDevil::Manager::self()->usableAdapter()) {
        kDebug() << "No Adapters found";
        qApp->exit();
        return;
    }

    kDebug() << "DeviceUbi: " << deviceInfo;
    kDebug() << "Files";
    kDebug() << files;

    setWindowTitle(i18n("Bluetooth Send Files"));
    setOption(NoCancelButton, false);
    setButton(QWizard::NextButton, new KPushButton(KIcon("document-export"), i18n("Send Files")));
    setButton(QWizard::CancelButton, new KPushButton(KStandardGuiItem::cancel()));
    setOption(QWizard::DisabledBackButtonOnLastPage);
    setOption(QWizard::NoBackButtonOnStartPage);

    kDebug() << "DeviceUbi: " << deviceInfo;
    kDebug() << "Files";
    kDebug() << files;

    setDevice(deviceInfo);

    if (!m_device || files.isEmpty()) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        setMinimumSize(680, 400);
        updateGeometry();
    }

    if (!m_device && files.isEmpty()) {
        addPage(new SelectDeviceAndFilesPage());
    } else if (!m_device) {
        addPage(new SelectDevicePage());
        setFiles(files);
    } else {

        if (files.isEmpty()) {
            addPage(new SelectFilesPage());
        } else {
            setFiles(files);
        }
    }

    addPage(new ConnectingPage());
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

void SendFileWizard::setFiles(const QStringList& files)
{
    kDebug() << files;
    m_files = files;
}

void SendFileWizard::setDevice(Device* device)
{
    kDebug() << device;
    m_device = device;
}

void SendFileWizard::setDevice(QString deviceUrl)
{
    kDebug() << deviceUrl;

    BlueDevil::Device *device = 0;
    if (deviceUrl.startsWith("bluetooth")) {
        deviceUrl.remove("bluetooth:");
        deviceUrl.replace(":", "-");
        deviceUrl.prepend("bluetooth:");
        KUrl url(deviceUrl);
        device = Manager::self()->usableAdapter()->deviceForAddress(url.host().replace("-", ":"));
    } else {
        device = Manager::self()->usableAdapter()->deviceForUBI(deviceUrl);
    }

    m_device = device;
}

Device* SendFileWizard::device()
{
    return m_device;
}

void SendFileWizard::wizardDone()
{
    done(1);
}

void SendFileWizard::startTransfer()
{
    if (m_files.isEmpty()) {
        kDebug() << "No files to send";
        return;
    }
    if (!m_device) {
        kDebug() << "No device selected";
    }

    m_job = new SendFilesJob(m_files, m_device);
    connect(m_job, SIGNAL(destroyed(QObject*)), qApp, SLOT(quit()));

    KIO::getJobTracker()->registerJob(m_job);
    m_job->start();

    QTimer::singleShot(2000, this, SLOT(wizardDone()));
}
