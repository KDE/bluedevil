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


#include "introductionpage.h"
#include "ui_introduction.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;
IntroductionPage::IntroductionPage(QWidget* parent): QWizardPage(parent)
{
    setTitle(i18n("Introduction"));
    setupUi(this);

    if (!Manager::self()->defaultAdapter()) {
        noAdapter(0);
    }

    connect(Manager::self(), SIGNAL(adapterAdded(Adapter*)), this,
                    SLOT(adapterAdded()));
    connect(Manager::self(), SIGNAL(defaultAdapterChanged(Adapter*)), this,
                    SLOT(noAdapter(Adapter*)));
}

IntroductionPage::~IntroductionPage()
{

}

void IntroductionPage::noAdapter(Adapter* adapter)
{
    if (!adapter) {
        labelsStack->setCurrentIndex(1);
        emit completeChanged();
    }
}

void IntroductionPage::adapterAdded()
{
    labelsStack->setCurrentIndex(0);
    emit completeChanged();
}

bool IntroductionPage::isComplete() const
{
    if (!Manager::self()->defaultAdapter()) {
        return false;
    }
    return true;
}

