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
class KFileWidget;
namespace BlueDevil {
    class Device;
}
using namespace BlueDevil;

class SendFileWizard : public QWizard
{
Q_OBJECT

public:
    SendFileWizard();
    virtual ~SendFileWizard();

    void setFileWidget(KFileWidget *);
    KFileWidget * fileWidget();

    void setDevice(Device *device);
    Device* device();

private:
    KFileWidget *m_fileWidget;
    Device      *m_device;
};

#endif // SENDFILEWIZARD_H
