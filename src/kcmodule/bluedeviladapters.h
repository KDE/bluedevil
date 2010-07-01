/*
 * Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
 * Copyright (C) 2010 UFO Coders <info@ufocoders.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _BLUEDEVILADAPTERS_H
#define _BLUEDEVILADAPTERS_H

#include <QtGui/QGroupBox>

#include <kcmodule.h>

class QVBoxLayout;
class QRadioButton;
class QSlider;
class QLabel;
class QCheckBox;
class QFormLayout;

class KLineEdit;

class SystemCheck;
class AdapterSettings;

namespace BlueDevil {
    class Adapter;
}

typedef BlueDevil::Adapter Adapter;

class AdapterSettings
    : public QGroupBox
{
    Q_OBJECT

public:
    enum DiscoverOptions {
        Hidden = 0,
        AlwaysVisible,
        TemporaryVisible
    };

    AdapterSettings(Adapter *adapter, KCModule *parent);
    virtual ~AdapterSettings();

    bool isModified() const;
    void applyChanges();

    QString name() const;
    DiscoverOptions discoverOptions() const;
    quint32 discoverTime() const;
    bool powered() const;

private Q_SLOTS:
    void readChanges();
    void visibilityChanged();
    void slotSettingsChanged();

Q_SIGNALS:
    void settingsChanged(bool changed);

private:
    Adapter      *m_adapter;
    KLineEdit    *m_name;
    QString       m_nameOrig;
    QRadioButton *m_hidden;
    bool          m_hiddenOrig;
    QRadioButton *m_alwaysVisible;
    bool          m_alwaysVisibleOrig;
    QRadioButton *m_temporaryVisible;
    bool          m_temporaryVisibleOrig;
    QSlider      *m_discoverTime;
    QLabel       *m_discoverTimeLabel;
    QWidget      *m_discoverTimeWidget;
    int           m_discoverTimeOrig;
    QCheckBox    *m_powered;
    bool          m_poweredOrig;

    QFormLayout  *m_layout;
};

class KCMBlueDevilAdapters
    : public KCModule
{
    Q_OBJECT

public:
    KCMBlueDevilAdapters(QWidget *parent, const QVariantList&);
    virtual ~KCMBlueDevilAdapters();

    virtual void defaults();
    virtual void save();

private Q_SLOTS:
    void updateAdapters();
    void defaultAdapterChanged(Adapter *adapter);
    void adapterDiscoverableChanged();
    void generateNoAdaptersMessage();
    void updateInformationState();
    void adapterConfigurationChanged(bool modified);

private:
    void fillAdaptersInformation();

private:
    QVBoxLayout                     *m_layout;
    QMap<Adapter*, AdapterSettings*> m_adapterSettingsMap;
    QWidget                         *m_noAdaptersMessage;

    SystemCheck *m_systemCheck;
};

#endif
