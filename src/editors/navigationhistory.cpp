/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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

#include "navigationhistory.h"
#include "viewhelper.h"

#include <QDebug>

namespace gams {
namespace studio {

NavigationHistory::NavigationHistory(QObject *parent) : QObject(parent)
{

}

void NavigationHistory::setActiveTab(QWidget *newTab)
{
    if (!newTab) return;

    AbstractEdit *ae = ViewHelper::toCodeEdit(newTab);
    if (!ae) {
        if (TextView *tv = ViewHelper::toTextView(newTab))
            ae = tv->edit();
    }

    // we have an editor and save actual cursor postion
    if (ae) {

        // remove connection from old editor
        if (mCurrentEditor)
            qDebug() << "disconnected?" << mCurrentEditor->disconnect(mCurrentEditor, &AbstractEdit::cursorPositionChanged, this, &NavigationHistory::receiveCursorPosChange);

        mCurrentEditor = ae;
        connect(mCurrentEditor, &AbstractEdit::cursorPositionChanged, this, &NavigationHistory::receiveCursorPosChange);

        // call once to store first cursor postion in new tab
        receiveCursorPosChange();
    } else {
        // we only save the tab
        insertCursorItem(newTab, -1);
    }
}

/// this function is used to get a cursor position change event and retrieve the new position
/// \brief NavigationHistory::receiveCursorPosChange
///
void NavigationHistory::receiveCursorPosChange()
{
    insertCursorItem(mCurrentEditor, mCurrentEditor->textCursor().position());
}

void NavigationHistory::insertCursorItem(QWidget *widget, int pos)
{
    // TODO(RG): add check to remove items close to last item
    qDebug() << QTime::currentTime() << "insertCursorItem" << widget << pos; // rogo: delete
    CursorHistoryItem chi;
    chi.tab = widget;
    chi.pos = pos;

    mHistory.push(chi);

    qDebug() << "mHistory size" << mHistory.size(); // rogo: delete
}

} // namespace studio
} // namespace gams

