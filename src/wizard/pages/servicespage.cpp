/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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


#include "servicespage.h"
#include "ui_services.h"
#include "serviceoption.h"
#include "../bluewizard.h"

#include <kservice.h>
#include <bluedevil/bluedevil.h>
#include <QDebug>

ServicesPage::ServicesPage(QWidget* parent): QWizardPage(parent)
{
    setTitle("Service selection");
    setupUi(this);
}

void ServicesPage::initializePage()
{
    BlueWizard *m_wizard = static_cast<BlueWizard*>(wizard());
    m_device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
    QStringList uuids = m_device->UUIDs();
    KService::List services =m_wizard->services();
    Q_FOREACH(QString uuid, uuids) {
        uuid = uuid.toUpper();
        Q_FOREACH(const KSharedPtr<KService> service, services) {
            if (service.data()->property("X-BlueDevil-UUIDS").toStringList().contains(uuid)) {
                addService(service.data()->name(), service.data()->comment());
            }
        }
    }
}

void ServicesPage::cleanupPage()
{

}

void ServicesPage::addService(const QString& name, const QString& desc)
{
    d_layout->addWidget(new ServiceOption(name, desc, this));
}
