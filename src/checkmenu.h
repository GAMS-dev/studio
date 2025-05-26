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
#ifndef CHECKMENU_H
#define CHECKMENU_H

#include <QMenu>

namespace gams {
namespace studio {

class CheckMenu : public QMenu
{
    Q_OBJECT
public:
    CheckMenu(QWidget *parent = nullptr);
    virtual ~CheckMenu();
    void addSubMenu(int actionDataValue, CheckMenu *subMenu);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void handleAction(QAction * action);
    bool contains(QPointF globalPos);

private:
    QHash<int, CheckMenu*> mSubMenus;
    CheckMenu* mVisibleSub = nullptr;
    CheckMenu* mParentMenu = nullptr;
};

} // namespace studio
} // namespace gams

#endif // CHECKMENU_H
