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
#include "pages/selectfilespage.h"
#include "pages/selectdevicepage.h"
#include "pages/connectingpage.h"
#include "pages/sendintropage.h"

#include <QApplication>

#include <kstandardguiitem.h>
#include <klocalizedstring.h>
#include <kpushbutton.h>
#include <kstatusbarjobtracker.h>
#include <kfilewidget.h>

#include <bluedevil/bluedevil.h>
#include <sendfilesjob.h>
#include <kdiroperator.h>

using namespace BlueDevil;

SendFileWizard::SendFileWizard() : QWizard(), m_device(0), m_job(0)
{
    if (!BlueDevil::Manager::self()->defaultAdapter()) {
        qDebug() << "No Adapters found";
        qApp->exit();
    } else {
        setWindowTitle(i18n("BlueDevil Send Files"));

        setButton(QWizard::BackButton, new KPushButton(KStandardGuiItem::back(KStandardGuiItem::UseRTL)));
        setButton(QWizard::NextButton, new KPushButton(KStandardGuiItem::forward(KStandardGuiItem::UseRTL)));
        setButton(QWizard::CancelButton, new KPushButton(KStandardGuiItem::cancel()));

        setOption(QWizard::DisabledBackButtonOnLastPage);

        //We do not want "Forward" as text
        setButtonText(QWizard::NextButton, i18n("Next"));

        addPage(new SendIntroPage());
        addPage(new SelectFilesPage());
        addPage(new SelectDevicePage());
        addPage(new ConnectingPage());

        show();

        m_agent = new ObexAgent(qApp);
    }
}

SendFileWizard::~SendFileWizard()
{
    if (m_job) {
        m_job->doKill();
    }
}

void SendFileWizard::setFileWidget(KFileWidget* fileWidget)
{
    m_fileWidget = fileWidget;
}

KFileWidget* SendFileWizard::fileWidget()
{
    return m_fileWidget;
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
    QWizard::done(1);
}

void SendFileWizard::startTransfer()
{
    m_job = new SendFilesJob(m_fileWidget->dirOperator()->selectedItems(), m_device, m_agent);
    connect(m_job, SIGNAL(destroyed(QObject*)), qApp, SLOT(quit()));

    KIO::getJobTracker()->registerJob(m_job);
    m_job->start();

    QTimer::singleShot(2000, this, SLOT(wizardDone()));
}