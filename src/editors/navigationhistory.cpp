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
    mInvalidItem.tab = nullptr;
}

void NavigationHistory::setActiveTab(QWidget *newTab)
{
    if (!newTab) return;

    AbstractEdit *ae = ViewHelper::toCodeEdit(newTab);
    if (!ae) {
        // if not succeeded, try again with textview
        if (TextView *tv = ViewHelper::toTextView(newTab))
            ae = tv->edit();
    }

    if (ae) {
        mCurrentEditor = ae;
        // remove connection from old editor
        if (mCurrentEditor)
            qDebug() << "disconnected?" << mCurrentEditor->disconnect(mCurrentEditor, &AbstractEdit::cursorPositionChanged, this, &NavigationHistory::receiveCursorPosChange);

        mCurrentTab = newTab;
        connect(ae, &AbstractEdit::cursorPositionChanged, this, &NavigationHistory::receiveCursorPosChange);

        // call once to store first cursor postion in new tab
        receiveCursorPosChange();
    } else {
        mCurrentEditor = nullptr;     // current tab is not an editor with cursor
        insertCursorItem(newTab, -1); // we only save the tab, no position
    }
}

/// Goes back in stack and returns CursorHistoryItem. This is not a simple pop as we need to keep
/// the items in case the user wants to go forward again!
/// \brief NavigationHistory::goBack
/// \return CursorHistoryItem*
///
CursorHistoryItem NavigationHistory::goBack()
{
    qDebug() << "goBack" << mStackPosition; // rogo: delete
    if (mStackPosition > 0) {
        mStackPosition--; // the latest cursor position is not the one we want to jump to
        CursorHistoryItem chi = mHistory.at(mStackPosition);
        return chi;
    } else {
        return mInvalidItem;
    }
}

///
/// \brief NavigationHistory::goFroward Goes forward in stack and returns CursorHistoryItem.
/// Only works when not at the end.
/// \return CursorHistoryItem* or nullptr
///
CursorHistoryItem NavigationHistory::goForward()
{
    qDebug() << "goForward" << mStackPosition; // rogo: delete
    if (mStackPosition == 0) return mInvalidItem;

    if (mStackPosition >= 0) {
        CursorHistoryItem chi = mHistory.at(mStackPosition);
        mStackPosition++;
        return chi;
    } else {
        return mInvalidItem;
    }

}

void NavigationHistory::insertCursorItem(QWidget *widget, int pos)
{
    if (mStopRecord) return;

    // TODO(RG): check if pointer is != 0, then drop all following items and reset to 0.
    // TODO(RG): add check to NOT insert items close to last item
    CursorHistoryItem chi;
    chi.tab = widget;
    chi.pos = pos;

    mHistory.push(chi);
    mStackPosition++;

    qDebug() << QTime::currentTime() << "insertCursorItem" << mStackPosition; // rogo: delete
}

/// this function is used to get a cursor position change event and retrieve the new position
/// \brief NavigationHistory::receiveCursorPosChange
///
void NavigationHistory::receiveCursorPosChange()
{
    insertCursorItem(mCurrentTab, mCurrentEditor->textCursor().position());
}

bool NavigationHistory::itemValid(CursorHistoryItem item)
{
    return item.tab != nullptr;
}

void NavigationHistory::stopRecord()
{
    mStopRecord = true;
}

void NavigationHistory::reenableRecord()
{
    mStopRecord = false;
}

} // namespace studio
} // namespace gams

