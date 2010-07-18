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
#include "pages/selectfilespage.h"
#include "pages/selectdevicepage.h"
#include "pages/connectingpage.h"

#include <kstandardguiitem.h>
#include <klocalizedstring.h>
#include <kpushbutton.h>

SendFileWizard::SendFileWizard() : QWizard()
{
    setWindowTitle(i18n("BlueDevil Send Files"));

    addPage(new SelectFilesPage());
    addPage(new SelectDevicePage());
    addPage(new ConnectingPage());

    setButton(QWizard::BackButton, new KPushButton(KStandardGuiItem::back(KStandardGuiItem::UseRTL)));
    setButton(QWizard::NextButton, new KPushButton(KStandardGuiItem::forward(KStandardGuiItem::UseRTL)));
    setButton(QWizard::CancelButton, new KPushButton(KStandardGuiItem::cancel()));

    //We do not want "Forward" as text
    setButtonText(QWizard::NextButton, i18n("Next"));
    //First show, then do the rest
    show();
}

SendFileWizard::~SendFileWizard()
{

}
