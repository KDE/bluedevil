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


#ifndef PAIRINGPAGE_H
#define PAIRINGPAGE_H

#include "ui_pairing.h"
#include <QWizardPage>
#include <QDBusMessage>

class BlueWizard;
class QDBusMessage;
class KPixmapSequenceOverlayPainter;
namespace BlueDevil {
    class Device;
}

using namespace BlueDevil;

class PairingPage : public QWizardPage
, Ui::Pairing
{
Q_OBJECT

public:
    PairingPage(QWidget* parent = 0);

    virtual void initializePage();
    virtual bool isComplete() const;
    virtual int nextId() const;
    virtual bool validatePage();
    virtual void cleanupPage();

public Q_SLOTS:
    void doPair();
    void nextPage();
    void pinRequested(const QString &pin);
    void confirmationRequested(quint32 passkey, const QDBusMessage &msg);

private:
    QDBusMessage m_msg;
    KPixmapSequenceOverlayPainter *m_working;
    BlueWizard  *m_wizard;
    Device      *m_device;
};

#endif // PAIRINGPAGE_H
