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

#include <QGroupBox>

#include <KCModule>

#include <BluezQt/Manager>

class QVBoxLayout;
class QRadioButton;
class QSlider;
class QLabel;
class QCheckBox;
class QFormLayout;
class QLineEdit;

class SystemCheck;
class AdapterSettings;

class AdapterSettings : public QGroupBox
{
    Q_OBJECT

public:
    enum DiscoverOptions {
        Hidden = 0,
        AlwaysVisible,
        TemporaryVisible
    };

    AdapterSettings(BluezQt::AdapterPtr adapter, KCModule *parent);

    bool isModified() const;
    void applyChanges();

    QString name() const;
    DiscoverOptions discoverOptions() const;
    quint32 discoverTime() const;
    bool powered() const;

public Q_SLOTS:
    void readChanges();

private Q_SLOTS:
    void visibilityChanged();
    void slotSettingsChanged();

Q_SIGNALS:
    void settingsChanged(bool changed);

private:
    BluezQt::AdapterPtr m_adapter;
    QLineEdit *m_name;
    QString m_nameOrig;
    QRadioButton *m_hidden;
    bool m_hiddenOrig;
    QRadioButton *m_alwaysVisible;
    bool m_alwaysVisibleOrig;
    QRadioButton *m_temporaryVisible;
    bool m_temporaryVisibleOrig;
    QSlider *m_discoverTime;
    QLabel *m_discoverTimeLabel;
    QWidget *m_discoverTimeWidget;
    int m_discoverTimeOrig;
    QCheckBox *m_powered;
    bool m_poweredOrig;

    QFormLayout *m_layout;
};

class KCMBlueDevilAdapters : public KCModule
{
    Q_OBJECT

public:
    KCMBlueDevilAdapters(QWidget *parent, const QVariantList&);

    void load() Q_DECL_OVERRIDE;
    void save() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void initJobResult(BluezQt::InitManagerJob *job);
    void updateAdapters();
    void generateNoAdaptersMessage();
    void adapterConfigurationChanged(bool modified);

private:
    void fillAdaptersInformation();

private:
    QVBoxLayout *m_layout;
    QMap<BluezQt::AdapterPtr, AdapterSettings*> m_adapterSettingsMap;
    QWidget *m_noAdaptersMessage;
    BluezQt::Manager *m_manager;

    SystemCheck *m_systemCheck;
};

#endif
