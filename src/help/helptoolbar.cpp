/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "helptoolbar.h"

namespace gams {
namespace studio {
namespace help {

HelpToolBar::HelpToolBar(QWidget *parent)
    : QToolBar(parent)
{
    setIconSize(QSize(16, 16));
    mActionBack->setEnabled(false);
    createWebActionTrigger( mActionBack, QWebEnginePage::Back );

    mActionForward->setEnabled(false);
    createWebActionTrigger( mActionForward, QWebEnginePage::Forward );

    mActionReload->setEnabled(false);
    createWebActionTrigger( mActionReload, QWebEnginePage::Reload );

    mActionStop->setEnabled(false);
    createWebActionTrigger( mActionStop, QWebEnginePage::Stop );
    this->setIconSize(QSize(16,16));
}

HelpToolBar::~HelpToolBar()
{
    delete mActionBack;
    delete mActionForward;
    delete mActionReload;
    delete mActionStop;
}

void HelpToolBar::addActionBack(QAction *action)
{
    mActionBack->setIcon(action->icon());
    mActionBack->setText(action->text());
    addAction(mActionBack);
}

void HelpToolBar::addActionForward(QAction *action)
{
    mActionForward->setIcon(action->icon());
    mActionForward->setText(action->text());
    addAction(mActionForward);
}

void HelpToolBar::addActionReload(QAction *action)
{
    mActionReload->setIcon(action->icon());
    mActionReload->setText(action->text());
    addAction(mActionReload);
}

void HelpToolBar::addActionStop(QAction *action)
{
    mActionStop->setIcon(action->icon());
    mActionStop->setText(action->text());
    addAction(mActionStop);
}

void HelpToolBar::addSpacer()
{
    QWidget* spacerWidget = new QWidget();
    spacerWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    this->addWidget(spacerWidget);
}

void HelpToolBar::on_webActionEnabledChanged(QWebEnginePage::WebAction webAction, bool enabled)
{
    if (webAction == QWebEnginePage::Back) {
        mActionBack->setEnabled(enabled);
    } else if (webAction == QWebEnginePage::Forward) {
        mActionForward->setEnabled(enabled);
    } else if (webAction == QWebEnginePage::Reload) {
        mActionReload->setEnabled(enabled);
    } else if (webAction == QWebEnginePage::Stop) {
        mActionStop->setEnabled(enabled);
    }
}

void HelpToolBar::createWebActionTrigger(QAction *action, QWebEnginePage::WebAction webAction)
{
    connect(action, &QAction::triggered, this, [this, webAction](bool checked){
        emit webActionTriggered(webAction, checked);
    });
}

} // namespace help
} // namespace studio
} // namespace gams
