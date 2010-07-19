/***************************************************************************
 *   This file is part of the KDE project                                  *
 *                                                                         *
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@ufocoders.com>    *
 *   Copyright (C) 2010 UFO Coders <info@ufocoders.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "obex_client.h"
#include "connectingpage.h"
#include "../sendfilewizard.h"

#include <QtCore/QVariant>

#include "klocalizedstring.h"
#include "kfilewidget.h"

#include <bluedevil/bluedevil.h>
#include <kdiroperator.h>
#include <kcompositejob.h>

using namespace BlueDevil;
ConnectingPage::ConnectingPage(QWidget* parent): QWizardPage(parent)
{
    setupUi(this);
}

void ConnectingPage::initializePage()
{
    Device *device = static_cast<SendFileWizard* >(wizard())->device();
    connLabel->setText(i18nc("Conencting to a bluetooth device", "Connecting to %1 ...").arg(device->name()));

    startSending();
}

bool ConnectingPage::isComplete() const
{
    return false;
}

void ConnectingPage::startSending()
{
    OrgOpenobexClientInterface *client = new OrgOpenobexClientInterface("org.openobex.client", "/", QDBusConnection::sessionBus(), this);

    Device *device = static_cast<SendFileWizard* >(wizard())->device();

    QVariantMap map;
    map.insert("Destination", QVariant(device->address()));

    QStringList filesToSend;
    KFileWidget *widget = static_cast<SendFileWizard* >(wizard())->fileWidget();
    KFileItemList list =  widget->dirOperator()->selectedItems();

    Q_FOREACH(const KFileItem &item, list) {
        filesToSend << item.url().path();
    }

    qDebug() << filesToSend;

    client->SendFiles(map, filesToSend, QDBusObjectPath("/BlueDevil_sendAgent"));
}
