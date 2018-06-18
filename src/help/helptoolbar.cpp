/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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

HelpToolBar::HelpToolBar(QAction *aHome,
                         QAction *aBack, QAction *aForward, QAction *aReload, QAction* aStop,
                         QToolButton* tbBookmark, QToolButton* tbHelp,
                         HelpWidget *parent) :
     helpWidget(parent)
{
    actionBack = new QAction(aBack->icon(), aBack->text());
    actionBack->setEnabled(false);
    createWebActionTrigger( actionBack, QWebEnginePage::Back );

    actionForward = new QAction(QIcon(aForward->icon()), aForward->text());
    actionForward->setEnabled(false);
    createWebActionTrigger( actionForward, QWebEnginePage::Forward );

    actionReload = new QAction(QIcon(aReload->icon()), aReload->text());
    actionReload->setEnabled(false);
    createWebActionTrigger( actionReload, QWebEnginePage::Reload );

    actionStop = new QAction(QIcon(aStop->icon()), aStop->text());
    actionStop->setEnabled(false);
    createWebActionTrigger( actionStop, QWebEnginePage::Stop );

    this->addAction(aHome);
    this->addSeparator();
    this->addAction(actionBack);
    this->addAction(actionForward);
    this->addSeparator();
    this->addAction(actionReload);
    this->addSeparator();
    this->addAction(actionStop);
    this->addSeparator();

    this->addWidget(tbBookmark);

    QWidget* spacerWidget = new QWidget();
    spacerWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    this->addWidget(spacerWidget);

    this->addWidget(tbHelp);

}

HelpToolBar::~HelpToolBar()
{
    delete actionBack;
    delete actionForward;
    delete actionReload;
    delete actionStop;
}

void HelpToolBar::on_webActionEnabledChanged(QWebEnginePage::WebAction webAction, bool enabled)
{
    if (webAction == QWebEnginePage::Back) {
        actionBack->setEnabled(enabled);
    } else if (webAction == QWebEnginePage::Forward) {
        actionForward->setEnabled(enabled);
    } else if (webAction == QWebEnginePage::Reload) {
        actionReload->setEnabled(enabled);
    } else if (webAction == QWebEnginePage::Stop) {
        actionStop->setEnabled(enabled);
    }
}

void HelpToolBar::createWebActionTrigger(QAction *action, QWebEnginePage::WebAction webAction)
{
    connect(action, &QAction::triggered, [this, action, webAction](bool checked){
        emit webActionTriggered(webAction, checked);
    });

}

} // namespace studio
} // namespace gams
