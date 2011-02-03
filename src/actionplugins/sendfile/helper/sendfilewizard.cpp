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


#include "sendfilewizard.h"
#include "obexagent.h"

#include "pages/selectdeviceandfilespage.h"
#include "pages/selectdevicepage.h"
#include "pages/selectfilespage.h"
#include "pages/connectingpage.h"

#include <QApplication>

#include <kdebug.h>
#include <kstandardguiitem.h>
#include <klocalizedstring.h>
#include <kpushbutton.h>
#include <kstatusbarjobtracker.h>
#include <kfiledialog.h>

#include <bluedevil/bluedevil.h>
#include <sendfilesjob.h>

using namespace BlueDevil;

SendFileWizard::SendFileWizard(const QString& deviceUBI, const QStringList& files)
    : QWizard()
    , m_device(0)
    , m_job(0)
{
    if (!BlueDevil::Manager::self()->defaultAdapter()) {
        kDebug() << "No Adapters found";
        qApp->exit();
        return;
    }

    setWindowTitle(i18n("Bluetooth Send Files"));

    setButton(QWizard::NextButton, new KPushButton(KIcon("document-export"), i18n("Send Files")));
    setButton(QWizard::CancelButton, new KPushButton(KStandardGuiItem::cancel()));

    setOption(QWizard::DisabledBackButtonOnLastPage);
    setOption(QWizard::NoBackButtonOnStartPage);

    if (deviceUBI.isEmpty() && files.isEmpty()) {
        addPage(new SelectDeviceAndFilesPage());
    } else if (deviceUBI.isEmpty()) {
        addPage(new SelectDevicePage());
        setFiles(files);
    } else if (files.isEmpty()) {
        addPage(new SelectFilesPage());
        setMinimumSize(680, 400);
        setDevice(Manager::self()->defaultAdapter()->deviceForUBI(deviceUBI));
    } else {
        setFiles(files);
        setDevice(Manager::self()->defaultAdapter()->deviceForUBI(deviceUBI));
    }
    addPage(new ConnectingPage());

    m_agent = new ObexAgent(qApp);
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

    m_job = new SendFilesJob(m_files, m_device, m_agent);
    connect(m_job, SIGNAL(destroyed(QObject*)), qApp, SLOT(quit()));

    KIO::getJobTracker()->registerJob(m_job);
    m_job->start();

    QTimer::singleShot(2000, this, SLOT(wizardDone()));
}
