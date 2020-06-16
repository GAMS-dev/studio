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

namespace gams {
namespace studio {

NavigationHistory::NavigationHistory(QObject *parent) : QObject(parent)
{ }

/// Goes back in stack and returns CursorHistoryItem. This is not a simple pop as we need to keep
/// the items in case the user wants to go forward again!
/// \brief NavigationHistory::goBack
/// \return CursorHistoryItem*
///
CursorHistoryItem NavigationHistory::goBack()
{
    if (mStackPosition > 0) {
        mStackPosition--; // the latest cursor position is not the one we want to jump to
        CursorHistoryItem chi = mHistory.at(mStackPosition);
        return chi;
    } else {
        return CursorHistoryItem();
    }
}

///
/// \brief NavigationHistory::goFroward Goes forward in stack and returns CursorHistoryItem.
/// Only works when not at the end.
/// \return CursorHistoryItem* or nullptr
///
CursorHistoryItem NavigationHistory::goForward()
{
    if (mStackPosition >= mHistory.size()-1) return CursorHistoryItem(); // reached the end

    mStackPosition++;
    CursorHistoryItem chi = mHistory.at(mStackPosition);
    return chi;
}

bool NavigationHistory::canGoForward()
{
    return mStackPosition < mHistory.size()-1;
}

bool NavigationHistory::canGoBackward()
{
    return mStackPosition > 0;
}

void NavigationHistory::insertCursorItem(QWidget *widget, int line, int pos)
{
    if (mStopRecord) return;
    if (ViewHelper::location(widget).isEmpty()) return; // do not insert empty path (e.g. welcome page)

    // remove oldest entry when limit is reached
    if (mHistory.size() >= MAX_SIZE) {
        mHistory.removeAt(0);
        mStackPosition--;
    }

    // remove all future entries if existing
    if (mStackPosition != mHistory.size()-1) {
        for (int i = mHistory.size()-1; i > mStackPosition; i--)
            mHistory.remove(i);
    }

    bool replaceLast = false;
    if (mStackPosition > -1) {
        CursorHistoryItem lastItem = mHistory.at(mStackPosition);

        // do some filtering:
        if (lastItem.tab == widget) {
            // do not save same pos
            if (lastItem.col == pos) return;

            // remove last when being in next/prev position
            if (lastItem.col == pos-1) replaceLast = true;
            if (lastItem.col == pos+1) replaceLast = true;

            // do not save same pos in next/prev line
            if (mHasCursor) {
                int hDiff = lastItem.lineNr - line;
                int vDiff = lastItem.col - pos;

                if (vDiff == 0 && (hDiff == 1 || hDiff == -1)) replaceLast = true;
            }
        }
    }

    if (replaceLast)
        mHistory.removeLast();
    else
        mStackPosition++;

    CursorHistoryItem chi;
    chi.tab = widget;
    chi.col = pos;
    chi.lineNr = line;
    chi.filePath = ViewHelper::location(widget);
    mHistory.push(chi);
}

/// this function is used to get a cursor position change event and retrieve the new position
/// \brief NavigationHistory::receiveCursorPosChange
///
void NavigationHistory::receiveCursorPosChange()
{
    AbstractEdit *ae = ViewHelper::toCodeEdit(mCurrentTab);
    TextView *tv = ViewHelper::toTextView(mCurrentTab);

    if (ae)
        insertCursorItem(mCurrentTab,
                         ae->textCursor().blockNumber(),
                         ae->textCursor().positionInBlock());
    else if (tv)
        insertCursorItem(mCurrentTab, tv->position().y(), tv->position().x());
    else
        insertCursorItem(mCurrentTab);

    emit historyChanged();
}

void NavigationHistory::setActiveTab(QWidget *newTab)
{
    if (!newTab) return;

    AbstractEdit *ae = ViewHelper::toCodeEdit(newTab);
    TextView *tv = ViewHelper::toTextView(newTab);

    mCurrentTab = newTab;

    if (ae || tv) {
        mHasCursor = true;
        AbstractEdit* edit = ae ? ae : tv->edit();

        // remove connection from old editor
        edit->disconnect(edit, &AbstractEdit::cursorPositionChanged, this, &NavigationHistory::receiveCursorPosChange);
        connect(edit, &AbstractEdit::cursorPositionChanged, this, &NavigationHistory::receiveCursorPosChange);
    } else {
        mHasCursor = false;
    }
    receiveCursorPosChange();
}

QWidget *NavigationHistory::currentTab() const
{
    return mCurrentTab;
}

bool NavigationHistory::itemValid(CursorHistoryItem item)
{
    return item.tab != nullptr;
}

void NavigationHistory::stopRecord()
{
    mStopRecord = true;
}

void NavigationHistory::startRecord()
{
    mStopRecord = false;
}

bool NavigationHistory::isRecording()
{
    return !mStopRecord;
}

} // namespace studio
} // namespace gams

