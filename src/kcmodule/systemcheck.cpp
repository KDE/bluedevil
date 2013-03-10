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
#include "globalsettings.h"

#include <bluedevil/bluedevil.h>

#include <kglobal.h>
#include <kaction.h>
#include <kdeversion.h>
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

#if KDE_IS_VERSION(4,6,41)
#include <kmessagewidget.h>
#else

class ErrorWidget
    : public QWidget
{
public:
    ErrorWidget(QWidget *parent = 0);
    virtual ~ErrorWidget();

    void setIcon(const QString &icon);
    void setText(const QString &reason);
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
    m_icon->setPixmap(KIconLoader::global()->loadIcon(icon, KIconLoader::Dialog));
}

void ErrorWidget::setText(const QString &reason)
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
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////

SystemCheck::SystemCheck(QWidget *parent)
    : QObject(parent)
    , m_kded(new KDED("org.kde.kded", "/kded", QDBusConnection::sessionBus()))
    , m_parent(parent)
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

#if KDE_IS_VERSION(4,6,41)
    m_noAdaptersError = new KMessageWidget(m_parent);
    m_noAdaptersError->setMessageType(KMessageWidget::Error);
    m_noAdaptersError->setCloseButtonVisible(false);
#else
    m_noAdaptersError = new ErrorWidget(m_parent);
    m_noAdaptersError->setIcon("window-close");
#endif
    m_noAdaptersError->setText(i18n("No Bluetooth adapters have been found."));
    layout->addWidget(m_noAdaptersError);

#if KDE_IS_VERSION(4,6,41)
    m_notDiscoverableAdapterError = new KMessageWidget(m_parent);
    m_notDiscoverableAdapterError->setMessageType(KMessageWidget::Warning);
    m_notDiscoverableAdapterError->setCloseButtonVisible(false);

    KAction *fixNotDiscoverableAdapter = new KAction(KIcon("dialog-ok-apply"), i18nc("Action to fix a problem", "Fix it"), m_notDiscoverableAdapterError);
    connect(fixNotDiscoverableAdapter, SIGNAL(triggered(bool)), this, SLOT(fixNotDiscoverableAdapterError()));
    m_notDiscoverableAdapterError->addAction(fixNotDiscoverableAdapter);
#else
    m_notDiscoverableAdapterError = new ErrorWidget(m_parent);
    m_notDiscoverableAdapterError->setIcon("edit-find");

    KPushButton *fixNotDiscoverableAdapter = new KPushButton(KIcon("dialog-ok-apply"), i18nc("Action to fix a problem", "Fix it"), m_notDiscoverableAdapterError);
    connect(fixNotDiscoverableAdapter, SIGNAL(clicked()), this, SLOT(fixNotDiscoverableAdapterError()));
    m_notDiscoverableAdapterError->addAction(fixNotDiscoverableAdapter);
#endif
    m_notDiscoverableAdapterError->setText(i18n("Your default Bluetooth adapter is not visible for remote devices."));

    layout->addWidget(m_notDiscoverableAdapterError);

#if KDE_IS_VERSION(4,6,41)
    m_disabledNotificationsError = new KMessageWidget(m_parent);
    m_disabledNotificationsError->setMessageType(KMessageWidget::Warning);
    m_disabledNotificationsError->setCloseButtonVisible(false);

    KAction *fixDisabledNotifications = new KAction(KIcon("dialog-ok-apply"), i18nc("Action to fix a problem", "Fix it"), m_disabledNotificationsError);
    connect(fixDisabledNotifications, SIGNAL(triggered(bool)), this, SLOT(fixDisabledNotificationsError()));
    m_disabledNotificationsError->addAction(fixDisabledNotifications);
#else
    m_disabledNotificationsError = new ErrorWidget(m_parent);
    m_disabledNotificationsError->setIcon("preferences-desktop-notification");

    KPushButton *fixDisabledNotifications = new KPushButton(KIcon("dialog-ok-apply"), i18nc("Action to fix a problem", "Fix it"), m_disabledNotificationsError);
    connect(fixDisabledNotifications, SIGNAL(clicked()), this, SLOT(fixDisabledNotificationsError()));
    m_disabledNotificationsError->addAction(fixDisabledNotifications);
#endif
    m_disabledNotificationsError->setText(i18n("Interaction with Bluetooth system is not optimal."));

    layout->addWidget(m_disabledNotificationsError);

#if KDE_IS_VERSION(4,6,41)
    m_noKDEDRunning = new KMessageWidget(m_parent);
    m_noKDEDRunning ->setMessageType(KMessageWidget::Warning);
    m_noKDEDRunning->setCloseButtonVisible(false);

    KAction *fixNoKDEDRunning = new KAction(KIcon("dialog-ok-apply"), i18nc("Action to fix a problem", "Fix it"), m_noKDEDRunning);
    connect(fixNoKDEDRunning, SIGNAL(triggered(bool)), this, SLOT(fixNoKDEDRunning()));
    m_noKDEDRunning->addAction(fixNoKDEDRunning);
#else
    m_noKDEDRunning = new ErrorWidget(m_parent);
    m_noKDEDRunning->setIcon("dialog-warning");

    KPushButton *fixNoKDEDRunning = new KPushButton(KIcon("dialog-ok-apply"), i18nc("Action to fix a problem", "Fix it"), m_noKDEDRunning);
    connect(fixNoKDEDRunning, SIGNAL(clicked()), this, SLOT(fixNoKDEDRunning()));
    m_noKDEDRunning->addAction(fixNoKDEDRunning);
#endif
    m_noKDEDRunning->setText(i18n("Bluetooth is not completely enabled."));

    layout->addWidget(m_noKDEDRunning);
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
    m_noKDEDRunning->setVisible(false);

    if (!GlobalSettings::self()->enableGlobalBluetooth()) {
        m_noAdaptersError->setEnabled(false);
        return;
    }

    BlueDevil::Adapter *const usableAdapter = BlueDevil::Manager::self()->usableAdapter();
    if (!usableAdapter) {
        m_noAdaptersError->setVisible(true);
        return;
    }
    if (!usableAdapter->isDiscoverable()) {
        m_notDiscoverableAdapterError->setVisible(true);
        return;
    }
    if (!checkNotificationsOK()) {
        m_disabledNotificationsError->setVisible(true);
        return;
    }
    if (!checkKDEDModuleLoaded()) {
        m_noKDEDRunning->setVisible(true);
        return;
    }
}

void SystemCheck::fixNoKDEDRunning()
{
    m_noKDEDRunning->setVisible(false);
    m_kded->loadModule("bluedevil");
}

void SystemCheck::fixNotDiscoverableAdapterError()
{
    m_notDiscoverableAdapterError->setVisible(false);
    BlueDevil::Manager::self()->usableAdapter()->setDiscoverable(true);
    BlueDevil::Manager::self()->usableAdapter()->setDiscoverableTimeout(0);
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
