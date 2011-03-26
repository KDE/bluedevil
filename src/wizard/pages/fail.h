/*****************************************************************************
 * This file is part of the KDE project                                      *
 *                                                                           *
 * Copyright (C) 2010 Alejandro Fiestas Olivares <afiestas@kde.org>          *
 * Copyright (C) 2010-2011 UFO Coders <info@ufocoders.com>                   *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/


#ifndef FAIL_H
#define FAIL_H

#include "ui_fail.h"
#include <QWizardPage>

class BlueWizard;
class KPixmapSequenceOverlayPainter;

namespace BlueDevil {
    class Device;
    class Adapter;
}

using namespace BlueDevil;

class FailPage : public QWizardPage
, Ui::Fail
{
Q_OBJECT

public:
    FailPage(BlueWizard* parent = 0);

    virtual void initializePage();

private:
    BlueWizard  *m_wizard;
};

#endif // FAIL_H