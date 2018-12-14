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
#include "maintabcontextmenu.h"
#include <QDebug>

namespace gams {
namespace studio {

enum TabActions {
    actClose,
    actCloseAll,
    actCloseAllExceptVisible,
    actCloseAllToLeft,
    actCloseAllToRight,
};

MainTabContextMenu::MainTabContextMenu()
{
    mActions.insert(actClose, addAction("&Close", this, &MainTabContextMenu::dummy));
    mActions.insert(actCloseAll, addAction("Close &All", this, &MainTabContextMenu::dummy));
    mActions.insert(actCloseAllExceptVisible, addAction("Close &except visible", this, &MainTabContextMenu::dummy));
    mActions.insert(actCloseAllToLeft, addAction("Close all &left", this, &MainTabContextMenu::dummy));
    mActions.insert(actCloseAllToRight, addAction("Close all &right", this, &MainTabContextMenu::dummy));
}

void MainTabContextMenu::dummy()
{
    qDebug() /*rogo: delete*/ << "dumdumdumdumdumdumdum";
}

void MainTabContextMenu::setTabIndex(int tab)
{
    qDebug() /*rogo: delete*/ << "tab" << tab;
    mTabIndex = tab;
}

void MainTabContextMenu::setParent(QTabBar* tabParent)
{
    mParent = tabParent;
}

}
}
