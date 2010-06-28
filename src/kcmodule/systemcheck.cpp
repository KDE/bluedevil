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

#include "systemcheck.h"
#include "kded.h"

#include <bluedevil/bluedevil.h>

#include <kglobal.h>
#include <kpushbutton.h>
#include <kconfiggroup.h>
#include <kcolorscheme.h>
#include <kstandarddirs.h>
#include <klocalizedstring.h>

#include <QtGui/QLabel>
#include <QtGui/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QBoxLayout>
#include <QtGui/QPaintEvent>

class ErrorWidget
    : public QWidget
{
public:
    ErrorWidget(QWidget *parent = 0);
    virtual ~ErrorWidget();

    void setIcon(const QString &icon);
    void setReason(const QString &reason);
    void addAction(KPushButton *action);

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    QLabel      *m_icon;
    QLabel      *m_reason;
    QHBoxLayout *m_actions;
};

ErrorWidget::ErrorWidget(QWidget *parent)
    : QWidget(parent)
    , m_icon(new QLabel(this))
    , m_reason(new QLabel(this))
    , m_actions(new QHBoxLayout)
{
    setAutoFillBackground(false);

    m_actions->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_icon);
    layout->addWidget(m_reason, 1);

    QVBoxLayout *outter = new QVBoxLayout;
    outter->addLayout(layout);
    outter->addLayout(m_actions);

    setLayout(outter);
}

ErrorWidget::~ErrorWidget()
{
}

void ErrorWidget::setIcon(const QString &icon)
{
    m_icon->setPixmap(KIconLoader::global()->loadIcon(icon, KIconLoader::NoGroup));
}

void ErrorWidget::setReason(const QString &reason)
{
    m_reason->setText(reason);
}

void ErrorWidget::addAction(KPushButton *action)
{
    action->setAutoFillBackground(false);
    m_actions->addWidget(action);
}

void ErrorWidget::paintEvent(QPaintEvent *event)
{
    const QRect r = event->rect();
    const KColorScheme colorScheme(QPalette::Active, KColorScheme::Window);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(0, 0, r.width(), r.height(), 10, 10);
    p.fillPath(path, colorScheme.background(KColorScheme::NegativeBackground));

    QWidget::paintEvent(event);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SystemCheck::SystemCheck(QWidget *parent)
    : QWidget(parent)
    , m_kded(new KDED("org.kde.kded", "/kded", QDBusConnection::sessionBus()))
    , m_noAdaptersError(0)
    , m_notDiscoverableAdapterError(0)
    , m_disabledNotificationsError(0)
{
}

SystemCheck::~SystemCheck()
{
    m_noAdaptersError = 0;
    m_notDiscoverableAdapterError = 0;
    m_disabledNotificationsError = 0;
    delete m_kded;
}

void SystemCheck::createWarnings(QVBoxLayout *layout)
{
    if (m_noAdaptersError) {
        return;
    }

    m_noAdaptersError = new ErrorWidget(this);
    m_noAdaptersError->setIcon("window-close");
    m_noAdaptersError->setReason(i18n("No Bluetooth adapters have been found."));
    layout->addWidget(m_noAdaptersError);

    m_notDiscoverableAdapterError = new ErrorWidget(this);
    m_notDiscoverableAdapterError->setIcon("layer-visible-off");
    m_notDiscoverableAdapterError->setReason(i18n("Your default Bluetooth adapter is not visible for remote devices."));
    KPushButton *fixNotDiscoverableAdapter = new KPushButton(KIcon("dialog-ok-apply"), i18n("Fix it"), this);
    connect(fixNotDiscoverableAdapter, SIGNAL(clicked()), this, SLOT(fixNotDiscoverableAdapterError()));
    m_notDiscoverableAdapterError->addAction(fixNotDiscoverableAdapter);
    layout->addWidget(m_notDiscoverableAdapterError);

    m_disabledNotificationsError = new ErrorWidget(this);
    m_disabledNotificationsError->setIcon("preferences-desktop-notification");
    m_disabledNotificationsError->setReason(i18n("Interaction with Bluetooth system is not optimal."));
    KPushButton *fixDisabledNotifications = new KPushButton(KIcon("dialog-ok-apply"), i18n("Fix it"), this);
    connect(fixDisabledNotifications, SIGNAL(clicked()), this, SLOT(fixDisabledNotificationsError()));
    m_disabledNotificationsError->addAction(fixDisabledNotifications);
    layout->addWidget(m_disabledNotificationsError);
}

bool SystemCheck::checkKDEDModuleLoaded()
{
    const QStringList res = m_kded->loadedModules();
    bool moduleLoaded = false;
    foreach (const QString &module, res) {
        if (module == "bluedevil") {
            moduleLoaded = true;
            break;
        }
    }
    return moduleLoaded;
}

bool SystemCheck::checkNotificationsOK()
{
    KConfig config("bluedevil.notifyrc", KConfig::NoGlobals);
    config.addConfigSources(KGlobal::dirs()->findAllResources("data", "bluedevil/bluedevil.notifyrc"));

    QStringList confList = config.groupList();
    QRegExp rx("^Event/([^/]*)$");
    confList = confList.filter(rx);

    Q_FOREACH (const QString &group , confList) {
        KConfigGroup cg(&config, group);
        const QString action = cg.readEntry("Action");
        if (!action.contains("Popup")) {
            return false;
        }
    }

    return true;
}

KDED *SystemCheck::kded()
{
    return m_kded;
}

void SystemCheck::updateInformationState()
{
    m_noAdaptersError->setEnabled(true);
    m_noAdaptersError->setVisible(false);
    m_notDiscoverableAdapterError->setVisible(false);
    m_disabledNotificationsError->setVisible(false);
    if (!checkKDEDModuleLoaded()) {
        m_noAdaptersError->setEnabled(false);
        return;
    }
    BlueDevil::Adapter *const defaultAdapter = BlueDevil::Manager::self()->defaultAdapter();
    if (!defaultAdapter) {
        m_noAdaptersError->setVisible(true);
        return;
    }
    if (!defaultAdapter->isDiscoverable()) {
        m_notDiscoverableAdapterError->setVisible(true);
        return;
    }
    if (!checkNotificationsOK()) {
        m_disabledNotificationsError->setVisible(true);
        return;
    }
}

void SystemCheck::fixNotDiscoverableAdapterError()
{
    m_notDiscoverableAdapterError->setVisible(false);
    BlueDevil::Manager::self()->defaultAdapter()->setDiscoverable(true);
    // No need to call to updateInformationState, since we are changing this property, it will be
    // triggered automatically.
}

void SystemCheck::fixDisabledNotificationsError()
{
    m_disabledNotificationsError->setVisible(false);

    KConfig config("bluedevil.notifyrc", KConfig::NoGlobals);
    config.addConfigSources(KGlobal::dirs()->findAllResources("data", "bluedevil/bluedevil.notifyrc"));

    QStringList confList = config.groupList();
    QRegExp rx("^Event/([^/]*)$");
    confList = confList.filter(rx);

    Q_FOREACH (const QString &group , confList) {
        KConfigGroup cg(&config, group);
        cg.writeEntry("Action", "Popup");
    }

    config.sync();

    emit updateInformationStateRequest();
}
