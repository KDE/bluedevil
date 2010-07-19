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


#include "servicespage.h"
#include "ui_services.h"
#include "serviceoption.h"
#include "../bluewizard.h"

#include <QButtonGroup>
#include <kservice.h>
#include <bluedevil/bluedevil.h>
#include <KNotification>

ServicesPage::ServicesPage(QWidget* parent): QWizardPage(parent)
{
    setTitle("Service selection");
    setupUi(this);
}

void ServicesPage::initializePage()
{
    m_wizard = static_cast<BlueWizard*>(wizard());
    KService::List services = m_wizard->services();
    Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());

    if (services.isEmpty()) {
        QString desc = device->alias();
        if (device->alias() != device->name() && !device->name().isEmpty()) {
            desc.append(" ("+device->name()+")");
        }
        desc.append(i18n(" has been paired successfully"));

        KNotification::event(
            KNotification::Notification,
            desc,
            KIcon(device->icon()).pixmap(48,48)
        )->sendEvent();
        m_wizard->done(1);
    }

    QStringList uuids = device->UUIDs();
    Q_FOREACH(QString uuid, uuids) {
        uuid = uuid.toUpper();
        Q_FOREACH(const KSharedPtr<KService> service, services) {
            if (service.data()->property("X-BlueDevil-UUIDS").toStringList().contains(uuid)) {
                addService(service.data());
            }
        }
    }
}


void ServicesPage::cleanupPage()
{

}

void ServicesPage::addService(const KService* service)
{
    ServiceOption *widget = new ServiceOption(service, m_buttonGroup, this);
    connect(widget, SIGNAL(selected(const KService*)), this, SLOT(selected(const KService*)));

    if (d_layout->count() == 0) {
        m_wizard->setService(service);
        widget->setChecked(true);
    }

    d_layout->addWidget(widget);
}

void ServicesPage::selected(const KService* service)
{
    m_wizard->setService(service);
}
