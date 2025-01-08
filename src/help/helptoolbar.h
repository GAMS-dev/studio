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
#ifndef HELPTOOLBAR_H
#define HELPTOOLBAR_H

#include <QToolBar>
#include <QToolButton>
#include <QWebEnginePage>

namespace gams {
namespace studio {
namespace help {

class HelpToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit HelpToolBar(QWidget *parent = nullptr);
    ~HelpToolBar();

    void addActionBack(QAction *action);
    void addActionForward(QAction *action);
    void addActionReload(QAction *action);
    void addActionStop(QAction *action);
    void addSpacer();

signals:
    void webActionTriggered(QWebEnginePage::WebAction webAction, bool checked);

public slots:
    void on_webActionEnabledChanged(QWebEnginePage::WebAction webAction, bool enabled);

private:
    void createWebActionTrigger(QAction* action, QWebEnginePage::WebAction webAction);

private:
    QAction* mActionBack = new QAction(this);
    QAction* mActionForward = new QAction(this);
    QAction* mActionReload = new QAction(this);
    QAction* mActionStop = new QAction(this);
};

} // namespace help
} // namespace studio
} // namespace gams

#endif // HELPTOOLBAR_H
