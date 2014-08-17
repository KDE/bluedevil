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

#include "bluedeviladapters.h"
#include "systemcheck.h"
#include "debug_p.h"

#include <QTimer>
#include <QScrollArea>
#include <QBoxLayout>
#include <QRadioButton>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QFormLayout>
#include <QButtonGroup>
#include <QLineEdit>
#include <QIcon>

#include <kaboutdata.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <QBluez/Manager>
#include <QBluez/InitManagerJob>
#include <QBluez/Adapter>

K_PLUGIN_FACTORY_WITH_JSON(BlueDevilFactory,
                           "bluedeviladapters.json",
                           registerPlugin<KCMBlueDevilAdapters>();)

////////////////////////////////////////////////////////////////////////////////////////////////////

AdapterSettings::AdapterSettings(QBluez::Adapter *adapter, KCModule *parent)
    : QGroupBox(parent)
    , m_adapter(adapter)
    , m_name(new QLineEdit(this))
    , m_hidden(new QRadioButton(i18nc("Radio widget to set if we want the adapter to be hidden", "Hidden"), this))
    , m_alwaysVisible(new QRadioButton(i18nc("Radio widget to set if we want the adapter to be always visible", "Always visible"), this))
    , m_temporaryVisible(new QRadioButton(i18nc("Radio widget to set if we want the adapter to be temporarily visible", "Temporarily visible"), this))
    , m_discoverTime(new QSlider(Qt::Horizontal, this))
    , m_discoverTimeLabel(new QLabel(this))
    , m_discoverTimeWidget(new QWidget(this))
    , m_powered(new QCheckBox(this))
{
    QButtonGroup *const buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(m_hidden);
    buttonGroup->addButton(m_alwaysVisible);
    buttonGroup->addButton(m_temporaryVisible);

    m_name->setText(adapter->alias());
    m_nameOrig = adapter->alias();
    m_hiddenOrig = false;
    m_alwaysVisibleOrig = false;
    m_temporaryVisibleOrig = false;
    if (!adapter->isDiscoverable()) {
        m_hidden->setChecked(true);
        m_hiddenOrig = true;
    } else {
        if (!adapter->discoverableTimeout()) {
            m_alwaysVisible->setChecked(true);
            m_alwaysVisibleOrig = true;
        } else {
            m_temporaryVisible->setChecked(true);
            m_temporaryVisibleOrig = true;
        }
    }
    m_discoverTime->setRange(1, 30);
    m_discoverTime->setValue(adapter->discoverableTimeout() / 60);
    m_discoverTime->setTickPosition(QSlider::TicksBelow);
    m_discoverTime->setTickInterval(1);
    m_discoverTimeOrig = qMax((quint32) 1, adapter->discoverableTimeout() / 60);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_discoverTime);
    layout->addWidget(m_discoverTimeLabel);
    m_discoverTimeWidget->setLayout(layout);
    m_discoverTimeWidget->setEnabled(m_temporaryVisibleOrig);

    m_discoverTimeLabel->setText(i18ncp("Discover time for the adapter", "1 minute", "%1 minutes", m_discoverTime->value()));

    m_powered->setChecked(adapter->isPowered());
    m_poweredOrig = adapter->isPowered();

    m_layout = new QFormLayout;
    m_layout->addRow(i18nc("Name of the adapter", "Name"), m_name);
    m_layout->addRow(i18nc("Whether the adapter is powered or not", "Powered"), m_powered);
    m_layout->addRow(i18nc("Whether the adapter is visible or not", "Visibility"), m_hidden);
    m_layout->addWidget(m_alwaysVisible);
    m_layout->addWidget(m_temporaryVisible);
    m_layout->addRow(i18nc("How long the adapter will be discoverable", "Discover Time"), m_discoverTimeWidget);
    setLayout(m_layout);

    m_layout->labelForField(m_discoverTimeWidget)->setEnabled(m_temporaryVisibleOrig);

    connect(m_adapter, &QBluez::Adapter::adapterChanged, this, &AdapterSettings::readChanges);
    connect(m_name, &QLineEdit::textEdited, this, &AdapterSettings::slotSettingsChanged);
    connect(m_hidden, &QRadioButton::toggled, this, &AdapterSettings::visibilityChanged);
    connect(m_hidden, &QRadioButton::toggled, this, &AdapterSettings::slotSettingsChanged);
    connect(m_alwaysVisible, &QRadioButton::toggled, this, &AdapterSettings::visibilityChanged);
    connect(m_alwaysVisible, &QRadioButton::toggled, this, &AdapterSettings::slotSettingsChanged);
    connect(m_temporaryVisible, &QRadioButton::toggled, this, &AdapterSettings::visibilityChanged);
    connect(m_temporaryVisible, &QRadioButton::toggled, this, &AdapterSettings::slotSettingsChanged);
    connect(m_discoverTime, &QSlider::valueChanged, this, &AdapterSettings::slotSettingsChanged);
    connect(m_powered, &QCheckBox::stateChanged, this, &AdapterSettings::slotSettingsChanged);

    setTitle(i18n("Adapter: %1 (%2)", adapter->alias(), adapter->address()));
}

bool AdapterSettings::isModified() const
{
    return m_name->text() != m_nameOrig || m_hidden->isChecked() != m_hiddenOrig ||
           m_alwaysVisible->isChecked() != m_alwaysVisibleOrig ||
           m_temporaryVisible->isChecked() != m_temporaryVisibleOrig ||
           m_discoverTime->value() != m_discoverTimeOrig || m_powered->isChecked() != m_poweredOrig;
}

void AdapterSettings::applyChanges()
{
    if (m_name->text() != m_nameOrig) {
        m_adapter->setAlias(m_name->text());
    }

    if (m_hidden->isChecked()) {
        m_adapter->setDiscoverable(false);
    } else if (m_alwaysVisible->isChecked()) {
        m_adapter->setDiscoverable(true);
        m_adapter->setDiscoverableTimeout(0);
    } else {
        m_adapter->setDiscoverable(true);
        m_adapter->setDiscoverableTimeout(m_discoverTime->value() * 60);
    }

    if (m_powered->isChecked() != m_poweredOrig) {
        m_adapter->setPowered(m_powered->isChecked());
    }
}

QString AdapterSettings::name() const
{
    return m_name->text();
}

AdapterSettings::DiscoverOptions AdapterSettings::discoverOptions() const
{
    if (m_hidden->isChecked()) {
        return Hidden;
    }
    if (m_alwaysVisible->isChecked()) {
        return AlwaysVisible;
    }
    return TemporaryVisible;
}

quint32 AdapterSettings::discoverTime() const
{
    return m_discoverTime->value() * 60;
}

bool AdapterSettings::powered() const
{
    return m_powered->isChecked();
}

void AdapterSettings::readChanges()
{
    blockSignals(true);

    m_nameOrig = m_adapter->alias();
    m_hiddenOrig = !m_adapter->isDiscoverable();
    m_alwaysVisibleOrig = m_adapter->isDiscoverable() && !m_adapter->discoverableTimeout();
    m_temporaryVisibleOrig = m_adapter->isDiscoverable() && m_adapter->discoverableTimeout();
    m_discoverTimeOrig = qMax((quint32) 1, m_adapter->discoverableTimeout() / 60);
    m_poweredOrig = m_adapter->isPowered();

    m_name->setText(m_nameOrig);
    m_hidden->setChecked(m_hiddenOrig);
    m_alwaysVisible->setChecked(m_alwaysVisibleOrig);
    m_temporaryVisible->setChecked(m_temporaryVisibleOrig);
    m_discoverTime->setValue(m_discoverTimeOrig);
    m_powered->setChecked(m_poweredOrig);

    m_discoverTimeLabel->setText(i18np("1 minute", "%1 minutes", m_discoverTime->value()));
    setTitle(i18n("Adapter: %1 (%2)", m_adapter->alias(), m_adapter->address()));

    blockSignals(false);

    emit settingsChanged(false);
}

void AdapterSettings::visibilityChanged()
{
    QRadioButton *const sdr = static_cast<QRadioButton*>(sender());
    if (!sdr->isChecked()) {
        return;
    }
    const bool enabled = sender() == m_temporaryVisible;
    m_discoverTimeWidget->setEnabled(enabled);
    m_layout->labelForField(m_discoverTimeWidget)->setEnabled(enabled);
}

void AdapterSettings::slotSettingsChanged()
{
    m_discoverTimeLabel->setText(i18np("1 minute", "%1 minutes", m_discoverTime->value()));
    emit settingsChanged(isModified());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

KCMBlueDevilAdapters::KCMBlueDevilAdapters(QWidget *parent, const QVariantList&)
    : KCModule(parent)
    , m_noAdaptersMessage(0)
    , m_systemCheck(0)
{
    KAboutData *ab = new KAboutData(QStringLiteral("kcmbluedeviladapters"),
                                    i18n("Bluetooth Adapters"),
                                    QStringLiteral("1.0"),
                                    i18n("Bluetooth Adapters Control Panel Module"),
                                    KAboutLicense::GPL,
                                    i18n("(c) 2010 Rafael Fernández López"));

    ab->addAuthor(i18n("Rafael Fernández López"), i18n("Developer and Maintainer"), QStringLiteral("ereslibre@kde.org"));
    setAboutData(ab);

    QVBoxLayout *layout = new QVBoxLayout;
    QScrollArea *mainArea = new QScrollArea(this);
    QWidget *widget = new QWidget(mainArea);
    m_layout = new QVBoxLayout;
    widget->setLayout(m_layout);
    mainArea->setWidget(widget);
    mainArea->setWidgetResizable(true);
    layout->addWidget(mainArea);
    setLayout(layout);

    // Initialize QBluez
    m_manager = new QBluez::Manager(this);
    QBluez::InitManagerJob *job = m_manager->init();
    job->start();
    connect(job, &QBluez::InitManagerJob::result, this, &KCMBlueDevilAdapters::initJobResult);
}

void KCMBlueDevilAdapters::save()
{
    Q_FOREACH (AdapterSettings *const adapterSettings, m_adapterSettingsMap) {
        adapterSettings->applyChanges();
    }
    QTimer::singleShot(300, this, SLOT(updateInformationState()));
}

void KCMBlueDevilAdapters::initJobResult(QBluez::InitManagerJob *job)
{
    if (job->error()) {
        qCWarning(KCMBLUETOOTH) << "Error initializing manager" << job->errorText();
        return;
    }

    QVBoxLayout *l = static_cast<QVBoxLayout*>(layout());

    m_systemCheck = new SystemCheck(m_manager, this);
    m_systemCheck->createWarnings(l);

    connect(m_systemCheck, &SystemCheck::updateInformationStateRequest,
            this, &KCMBlueDevilAdapters::updateInformationState);

    connect(m_manager, &QBluez::Manager::adapterAdded,
            this, &KCMBlueDevilAdapters::updateAdapters);
    connect(m_manager, &QBluez::Manager::adapterRemoved,
            this, &KCMBlueDevilAdapters::updateAdapters);
    connect(m_manager, &QBluez::Manager::usableAdapterChanged,
            this, &KCMBlueDevilAdapters::usableAdapterChanged);

    QBluez::Adapter *const usableAdapter = m_manager->usableAdapter();
    if (usableAdapter) {
        connect(usableAdapter, &QBluez::Adapter::discoverableChanged,
                this, &KCMBlueDevilAdapters::adapterDiscoverableChanged);
    }

    fillAdaptersInformation();
    updateInformationState();
}

void KCMBlueDevilAdapters::updateAdapters()
{
    fillAdaptersInformation();
    QTimer::singleShot(300, this, SLOT(updateInformationState()));
}

void KCMBlueDevilAdapters::usableAdapterChanged(QBluez::Adapter *adapter)
{
    if (adapter) {
        connect(adapter, &QBluez::Adapter::discoverableChanged,
                this, &KCMBlueDevilAdapters::adapterDiscoverableChanged);
    }
    QTimer::singleShot(300, this, SLOT(updateInformationState()));
}

void KCMBlueDevilAdapters::adapterDiscoverableChanged()
{
    QTimer::singleShot(300, this, SLOT(updateInformationState()));
}

void KCMBlueDevilAdapters::generateNoAdaptersMessage()
{
    QGridLayout *layout = new QGridLayout;
    m_noAdaptersMessage = new QWidget(this);
    QLabel *label = new QLabel(m_noAdaptersMessage);
    label->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-information")).pixmap(128, 128));
    layout->addWidget(label, 0, 1, Qt::AlignHCenter);
    layout->addWidget(new QLabel(i18n("No adapters found. Please connect one."), m_noAdaptersMessage),
                                 1, 1, Qt::AlignHCenter);
    layout->setRowStretch(2, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    m_noAdaptersMessage->setLayout(layout);
    m_noAdaptersMessage->setVisible(false);
}

void KCMBlueDevilAdapters::updateInformationState()
{
    m_systemCheck->updateInformationState();
}

void KCMBlueDevilAdapters::adapterConfigurationChanged(bool modified)
{
    if (modified) {
        emit changed(true);
        return;
    }
    Q_FOREACH (AdapterSettings *const adapterSettings, m_adapterSettingsMap) {
        if (adapterSettings->isModified()) {
            return;
        }
    }
    emit changed(false);
}

void KCMBlueDevilAdapters::fillAdaptersInformation()
{
    qDeleteAll(m_adapterSettingsMap);
    m_adapterSettingsMap.clear();

    for (int i = 0; i < m_layout->count(); ++i) {
        m_layout->takeAt(0);
    }

    if (m_manager->adapters().isEmpty()) {
        generateNoAdaptersMessage();
        m_layout->addWidget(m_noAdaptersMessage);
        m_noAdaptersMessage->setVisible(true);
        return;
    }

    if (m_noAdaptersMessage) {
        m_noAdaptersMessage->setVisible(false);
    }

    Q_FOREACH (QBluez::Adapter *adapter, m_manager->adapters()) {
        AdapterSettings *adapterSettings = new AdapterSettings(adapter, this);
        connect(adapterSettings, &AdapterSettings::settingsChanged,
                this, &KCMBlueDevilAdapters::adapterConfigurationChanged);
        m_adapterSettingsMap.insert(adapter, adapterSettings);
        m_layout->addWidget(adapterSettings);
    }

    m_layout->addStretch();
}

#include "bluedeviladapters.moc"
