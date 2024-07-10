/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "logtabcontextmenu.h"
#include "mainwindow.h"

namespace gams {
namespace studio {

enum TabActions {
    actClose,
    actCloseAll,
    actCloseAllExceptVisible,
    actCloseAllToLeft,
    actCloseAllToRight,
};

LogTabContextMenu::LogTabContextMenu(MainWindow* parent) : mParent(parent)
{
    mActions.insert(actClose, addAction("&Close", this, &LogTabContextMenu::close));
    mActions.insert(actCloseAll, addAction("Close &All", this, &LogTabContextMenu::closeAll));
    mActions.insert(actCloseAllExceptVisible, addAction("Close &except visible", this, &LogTabContextMenu::closeAllExceptVisible));
    mActions.insert(actCloseAllToLeft, addAction("Close all &left", this, &LogTabContextMenu::closeAllLeft));
    mActions.insert(actCloseAllToRight, addAction("Close all &right", this, &LogTabContextMenu::closeAllRight));
}

void LogTabContextMenu::close()
{
    mParent->on_logTabs_tabCloseRequested(mTabIndex);
}

void LogTabContextMenu::closeAll()
{
    int tabs = mParent->logTabCount();

    for (int i = tabs - 1; i >= 0; i--)
        mParent->on_logTabs_tabCloseRequested(i);
}

void LogTabContextMenu::closeAllExceptVisible()
{
    int tabs = mParent->logTabCount();

    for (int i = tabs - 1; i >= 0; i--) {
        if (i == mParent->currentLogTab()) continue;
        mParent->on_logTabs_tabCloseRequested(i);
    }
}

void LogTabContextMenu::closeAllLeft()
{
    for (int i = mTabIndex - 1; i >= 0; i--)
        mParent->on_logTabs_tabCloseRequested(i);
}

void LogTabContextMenu::closeAllRight()
{
    for (int i = mParent->logTabCount(); i > mTabIndex; i--)
        mParent->on_logTabs_tabCloseRequested(i);
}


void LogTabContextMenu::setTabIndex(int tab)
{
    mTabIndex = tab;
    mActions.value(actCloseAllExceptVisible)->setEnabled(mParent->mainTabs()->count() > 1);
    mActions.value(actCloseAllToLeft)->setEnabled(mTabIndex);
    mActions.value(actCloseAllToRight)->setEnabled(mTabIndex < mParent->mainTabs()->count()-1);
}

}
}
