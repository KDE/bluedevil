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


#ifndef SERVICESPAGE_H
#define SERVICESPAGE_H

#include "ui_services.h"
#include <QWizardPage>

class KService;
class BlueWizard;
class QButtonGroup;

namespace BlueDevil {
    class Device;
}
using namespace BlueDevil;

class ServicesPage : public QWizardPage
, Ui::Services
{
Q_OBJECT

public:
    ServicesPage(QWidget* parent = 0);
    virtual void initializePage();
    virtual void cleanupPage();
    virtual int nextId() const;
public Q_SLOTS:
    void selected(const KService *);

private:
    void addService(const KService* service);

private:
    BlueWizard *m_wizard;
    KService *m_service;
    QButtonGroup m_buttonGroup;
};

#endif // SERVICESPAGE_H
