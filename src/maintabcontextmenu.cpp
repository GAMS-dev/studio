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
#include "maintabcontextmenu.h"
#include "mainwindow.h"
#include "keys.h"

namespace gams {
namespace studio {

enum TabActions {
    actSplitH,
    actSplitV,
    actSetMain,
    actClose,
    actCloseAll,
    actCloseAllExceptVisible,
    actCloseAllToLeft,
    actCloseAllToRight,
};

MainTabContextMenu::MainTabContextMenu(MainWindow* parent) : mParent(parent)
{
    setToolTipsVisible(true);
    mActions.insert(actSplitH, addAction("Pin &right", this, [this]() {
        emit openPinView(mTabIndex, Qt::Horizontal);
    }));
    mActions.value(actSplitH)->setToolTip("Pin edit to the right <b>Ctrl+Click</b>");
    mActions.insert(actSplitV, addAction("Pin &below", this, [this]() {
        emit openPinView(mTabIndex, Qt::Vertical);
    }));
    mActions.value(actSplitV)->setToolTip("Pin edit to the bottom <b>Shift+Ctrl+Click</b>");

    addSeparator();
    mActions.insert(actSetMain, addAction("Set as &main file", this, [this]() {
        emit handleSetMainFile(mTabIndex);
    }));
    mActions.value(actSetMain)->setToolTip("Set as main file in current project");
    mActions.value(actSetMain)->setShortcut(Keys::instance().keySequence(Hotkey::SetMainFile).first());

    addSeparator();
    mActions.insert(actClose, addAction("&Close", this, &MainTabContextMenu::close));
    mActions.insert(actCloseAll, addAction("Close &All", mParent, &MainWindow::on_actionClose_All_triggered));
    mActions.insert(actCloseAllExceptVisible, addAction("Close &except visible", mParent, &MainWindow::on_actionClose_All_Except_triggered));
    mActions.insert(actCloseAllToLeft, addAction("Close all &left", this, &MainTabContextMenu::closeAllLeft));
    mActions.insert(actCloseAllToRight, addAction("Close all &right", this, &MainTabContextMenu::closeAllRight));
}

void MainTabContextMenu::close()
{
    mParent->on_mainTabs_tabCloseRequested(mTabIndex);
}

void MainTabContextMenu::closeAllLeft()
{
    for (int i = mTabIndex - 1; i >= 0; i--)
        mParent->on_mainTabs_tabCloseRequested(i);
}

void MainTabContextMenu::closeAllRight()
{
    QTabWidget* tabs = mParent->mainTabs();
    QWidget* idxPtr = tabs->widget(mTabIndex+1); // start with tab to the right

    while (idxPtr) {
        QWidget *old = idxPtr;

        int next = tabs->indexOf(idxPtr);
        idxPtr = tabs->widget(++next);

        mParent->on_mainTabs_tabCloseRequested(tabs->indexOf(old));
    }
}

void MainTabContextMenu::setTabIndex(int tab, bool canSplit, bool isFile)
{
    mTabIndex = tab;
    mActions.value(actSplitH)->setVisible(canSplit);
    mActions.value(actSplitV)->setVisible(canSplit);
    mActions.value(actCloseAllExceptVisible)->setEnabled(mParent->mainTabs()->count() > 1);
    mActions.value(actCloseAllToLeft)->setEnabled(mTabIndex);
    mActions.value(actCloseAllToRight)->setEnabled(mTabIndex < mParent->mainTabs()->count()-1);
    mActions.value(actSetMain)->setVisible(isFile);
    if (isFile) {
        bool canBeMain = false;
        emit handleSetMainFile(mTabIndex, &canBeMain);
        mActions.value(actSetMain)->setEnabled(canBeMain);
    }
}

}
}
