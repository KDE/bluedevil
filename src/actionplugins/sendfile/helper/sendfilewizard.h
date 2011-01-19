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


#ifndef SENDFILEWIZARD_H
#define SENDFILEWIZARD_H

#include <QObject>
#include <QWizard>
#include "discoverwidget.h"

class WizardAgent;
class KFileDialog;
class ObexAgent;
class SendFilesJob;
namespace BlueDevil {
    class Device;
}
using namespace BlueDevil;

class SendFileWizard : public QWizard
{
Q_OBJECT

public:
    SendFileWizard(const QString &deviceUri);
    virtual ~SendFileWizard();

    virtual void done(int result);

    void setFileDialog(KFileDialog *);
    KFileDialog * fileDialog();

    void setDevice(Device *device);
    Device* device();

    void startTransfer();

private Q_SLOTS:
    void wizardDone();

private:
    KFileDialog  *m_fileDialog;
    Device       *m_device;
    ObexAgent    *m_agent;
    SendFilesJob *m_job;
};

#endif // SENDFILEWIZARD_H
