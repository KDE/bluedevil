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


#ifndef DISCOVERPAGE_H
#define DISCOVERPAGE_H

#include "ui_discover.h"
#include <QWizard>

class QTimer;
class BlueWizard;

namespace BlueDevil {
    class Device;
}
using namespace BlueDevil;

class DiscoverPage : public QWizardPage
, public Ui::Discover
{
Q_OBJECT

public:
    DiscoverPage(QWidget* parent = 0);
    virtual ~DiscoverPage();

    virtual void initializePage();
    virtual void cleanupPage();
    virtual bool isComplete() const;

private Q_SLOTS:
    void startScan();
    void deviceFound(Device * device);
    void timeout();
    void itemSelected(QListWidgetItem* item);
    void leavePage(int id);

private:
    void stopScan();

private:
    char m_counter;

    QTimer *m_timer;
    BlueWizard *m_wizard;
};

#endif // DISCOVERPAGE_H
