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

void NavigationHistory::insertCursorItem(int line, int pos)
{
    if (ViewHelper::location(mCurrentEdit).isEmpty()) return; // do not insert empty path (e.g. welcome page)

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
        if (lastItem.edit == mCurrentEdit) {
            // do not save identical
            if (lastItem.lineNr == line && lastItem.col == pos) return;

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
    chi.edit = mCurrentEdit;
    chi.pinKind = mPinKind;
    chi.col = pos;
    chi.lineNr = line;
    chi.filePath = ViewHelper::location(mCurrentEdit);
    mHistory.push(chi);
}

/// this function is used to get a cursor position change event and retrieve the new position
/// \brief NavigationHistory::receiveCursorPosChange
///
void NavigationHistory::receiveCursorPosChange()
{
    if (mStopRecord) return;

    AbstractEdit *ae = ViewHelper::toCodeEdit(mCurrentEdit);
    TextView *tv = ViewHelper::toTextView(mCurrentEdit);

    if (ae)
        insertCursorItem(ae->textCursor().blockNumber(),
                         ae->textCursor().positionInBlock());
    else if (tv)
        insertCursorItem(tv->position().y(), tv->position().x());
    else
        insertCursorItem();

    emit historyChanged();
}

void NavigationHistory::setCurrentEdit(QWidget *edit, PinKind pinKind)
{
    if (!edit) return;

    AbstractEdit *ae = ViewHelper::toCodeEdit(edit);
    TextView *tv = ViewHelper::toTextView(edit);

    mCurrentEdit = edit;
    mPinKind = pinKind;

    if (ae || tv) {
        mHasCursor = true;
        AbstractEdit* edit = ae ? ae : tv->edit();

        // remove connection from old editor
        edit->disconnect(edit, &AbstractEdit::cursorPositionChanged, this, &NavigationHistory::receiveCursorPosChange);
        connect(edit, &AbstractEdit::cursorPositionChanged, this, &NavigationHistory::receiveCursorPosChange);
    } else {
        mHasCursor = false;
    }
}

QWidget *NavigationHistory::currentEdit() const
{
    return mCurrentEdit;
}

bool NavigationHistory::itemValid(const CursorHistoryItem &item)
{
    return item.edit != nullptr;
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

