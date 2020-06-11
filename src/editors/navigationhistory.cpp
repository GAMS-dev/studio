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

NavigationHistory::~NavigationHistory()
{
}

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
    if (mStackPosition >= mHistory.size()-1) return mInvalidItem; // reached the end

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

void NavigationHistory::insertCursorItem(QWidget *widget, int pos)
{
    if (mStopRecord) return;

    // remove all future entries if existing
    if (mStackPosition != mHistory.size()-1) {
        for (int i = mHistory.size()-1; i > mStackPosition; i--) {
            mHistory.remove(i);
        }
    }

    // remove oldest entry when limit is reached
    if (mHistory.size() >= MAX_SIZE) {
        mHistory.removeAt(0);
        mStackPosition--;
    }

    CursorHistoryItem chi;
    chi.tab = widget;
    chi.pos = pos;
    chi.filePath = ViewHelper::location(widget);

    if (chi.filePath.isEmpty()) return; // do not insert empty path (e.g. welcome page)

    bool replaceLast = false;
    if (mStackPosition > -1) {
        CursorHistoryItem lastItem = mHistory.at(mStackPosition);

        // do some filtering:
        if (lastItem.tab == widget) {
            // do not save same pos
            if (lastItem.pos == pos) return;

            // remove last when being in next/prev position
            if (lastItem.pos == pos-1) replaceLast = true;
            if (lastItem.pos == pos+1) replaceLast = true;

            // do not save same pos in next/prev line
            if (mCurrentEditor) {
                QTextCursor lastCursor = mCurrentEditor->textCursor();
                lastCursor.setPosition(lastItem.pos);
                int hDiff = lastCursor.blockNumber() - mCurrentEditor->textCursor().blockNumber();
                int vDiff = lastCursor.positionInBlock() - mCurrentEditor->textCursor().positionInBlock();

                if (vDiff == 0 && (hDiff == 1 || hDiff == -1)) replaceLast = true;
            }
        }
    }
    if (replaceLast)
        mHistory.removeLast();
    else
        mStackPosition++;

    mHistory.push(chi);
}

/// this function is used to get a cursor position change event and retrieve the new position
/// \brief NavigationHistory::receiveCursorPosChange
///
void NavigationHistory::receiveCursorPosChange()
{
    insertCursorItem(mCurrentTab, mCurrentEditor ? mCurrentEditor->textCursor().position() : -1);
    emit historyChanged();
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
        mCurrentEditor->disconnect(mCurrentEditor, &AbstractEdit::cursorPositionChanged, this, &NavigationHistory::receiveCursorPosChange);

        mCurrentTab = newTab;
        connect(ae, &AbstractEdit::cursorPositionChanged, this, &NavigationHistory::receiveCursorPosChange);
    } else {
        mCurrentEditor = nullptr; // current tab is not an editor with cursor
        insertCursorItem(newTab); // we only save the tab, no position
    }
}

AbstractEdit *NavigationHistory::currentEditor() const
{
    return mCurrentEditor;
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

