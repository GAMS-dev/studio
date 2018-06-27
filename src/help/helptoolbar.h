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
#ifndef HELPTOOLBAR_H
#define HELPTOOLBAR_H

#include <QToolBar>
#include <QToolButton>
#include <QWebEnginePage>

namespace gams {
namespace studio {

class HelpWidget;

class HelpToolBar : public QToolBar
{
    Q_OBJECT

public:
    HelpToolBar(QAction *aHome,
                QAction *Back, QAction *aForward, QAction *aReload, QAction* aStop,
                QToolButton* tbBookmark, QToolButton* tbHelp,
                HelpWidget *parent);
    ~HelpToolBar();

signals:
    void webActionTriggered(QWebEnginePage::WebAction webAction, bool checked);

public slots:
    void on_webActionEnabledChanged(QWebEnginePage::WebAction webAction, bool enabled);

private:
    void createWebActionTrigger(QAction* action, QWebEnginePage::WebAction webAction);

private:
    QAction* actionBack;
    QAction* actionForward;
    QAction* actionReload;
    QAction* actionStop;

    HelpWidget* helpWidget;
};

} // namespace studio
} // namespace gams

#endif // HELPTOOLBAR_H
