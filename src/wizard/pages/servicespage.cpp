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
#include <QDebug>
void ServicesPage::initializePage()
{
    m_wizard = static_cast<BlueWizard*>(wizard());
    KService::List services = m_wizard->services();
    Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());

    QStringList uuids = device->UUIDs();
    QByteArray preselectedUuid = m_wizard->preselectedUuid();
    Q_FOREACH(QString uuid, uuids) {
        uuid = uuid.toUpper();
        Q_FOREACH(const KSharedPtr<KService> service, services) {

            if (preselectedUuid.isEmpty()) {
                if (service.data()->property("X-BlueDevil-UUIDS").toStringList().contains(uuid)) {
                    addService(service.data());
                }
            } else {
                if (service.data()->property("X-BlueDevil-UUIDS").toStringList().contains(preselectedUuid)) {
                    m_wizard->setService(service.data());
                    m_wizard->done(1);
                }
            }
        }
    }

    //If no service has been added (no compatible services)
    if (d_layout->count() == 0) {
        QString desc(i18n("%1 has been paired successfully").arg(device->friendlyName()));

        KNotification::event(
            KNotification::Notification,
            desc,
            KIcon(device->icon()).pixmap(48,48)
        )->sendEvent();
        m_wizard->done(0);
        return;
    }

    ServiceOption *noneOption = new ServiceOption(i18n("None"), i18n("Do not initialize any service"), m_buttonGroup);
    connect(noneOption, SIGNAL(selected(const KService*)), this, SLOT(selected(const KService*)));
    d_layout->addWidget(noneOption);
}

void ServicesPage::cleanupPage()
{
    QList <QAbstractButton *>  buttonList =  m_buttonGroup.buttons();
    Q_FOREACH(QAbstractButton *btn, buttonList) {
         m_buttonGroup.removeButton(btn);
    }

    QLayoutItem *child;
    while ((child = d_layout->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }
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
