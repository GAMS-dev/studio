/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include "keys.h"

namespace gams {
namespace studio {

Keys *Keys::mInstance = nullptr;

Keys::Keys()
{

}

KeySeqList::KeySeqList(const QKeySequence& seq, QString title)
    : mTitle(title)
{
    mSequence << seq;
}

KeySeqList::KeySeqList(const char* seq, QString title)
    : mTitle(title)
{
    mSequence << QKeySequence(seq);
}

KeySeqList& KeySeqList::operator=(const KeySeqList& other)
{
    mSequence = other.mSequence;
    mTitle = other.mTitle;
    return *this;
}

KeySeqList&KeySeqList::operator=(const QKeySequence& other)
{
    mSequence.clear();
    mSequence << other;
    return *this;
}

KeySeqList&KeySeqList::operator <<(const KeySeqList& other)
{
    mSequence.append(other.mSequence);
    return *this;
}

KeySeqList&KeySeqList::operator <<(const QKeySequence& other)
{
    mSequence << other;
    return *this;
}

bool KeySeqList::operator ==(KeySeqList other) const
{
    bool res = mSequence.size() == other.mSequence.size();
    for (QKeySequence seq: mSequence) {
        if (!res) break;
        res = other.mSequence.contains(seq);
    }
    return res;
}

void Keys::reset()
{
    KeySeqList *seq;
    seq = new KeySeqList("Return","new line");
    *seq << QKeySequence("Enter");
    setHotkey(Hotkey::NewLine, seq);

    seq = new KeySeqList("Cut","cut selected text");
    *seq << QKeySequence("Ctrl+X");
    setHotkey(Hotkey::Cut, seq);

    seq = new KeySeqList("Copy","copy selected text");
    *seq << QKeySequence("Ctrl+C");
    setHotkey(Hotkey::Copy, seq);

    seq = new KeySeqList("Paste","paste clipboard content");
    *seq << QKeySequence("Ctrl+V");
    setHotkey(Hotkey::Paste, seq);

    seq = new KeySeqList("Undo","undo last modification");
    *seq << QKeySequence("Ctrl+Z");
    setHotkey(Hotkey::Undo, seq);

    seq = new KeySeqList("Redo","redo last modification");
    *seq << QKeySequence("Shift+Ctrl+Z");
    setHotkey(Hotkey::Redo, seq);

    seq = new KeySeqList("MoveViewLineUp","Move the view one line up");
    *seq << QKeySequence("Ctrl+Up");
    setHotkey(Hotkey::MoveViewLineUp, seq);

    seq = new KeySeqList("MoveViewLineDown","Move the view one line down");
    *seq << QKeySequence("Ctrl+Down");
    setHotkey(Hotkey::MoveViewLineDown, seq);

    seq = new KeySeqList("MoveViewPageUp","Move the view one page up");
    *seq << QKeySequence("Ctrl+PgUp");
    setHotkey(Hotkey::MoveViewPageUp, seq);

    seq = new KeySeqList("MoveViewPageDown","Move the view one page down");
    *seq << QKeySequence("Ctrl+PgDown");
    setHotkey(Hotkey::MoveViewPageDown, seq);

    seq = new KeySeqList("MoveToEndOfLine","Move to the end of line");
#ifdef Q_OS_OSX
    *seq << QKeySequence("Ctrl+Right") << QKeySequence("Shift+Ctrl+Right");
#else
    *seq << QKeySequence("End") << QKeySequence("Shift+End");
#endif
    setHotkey(Hotkey::MoveToEndOfLine, seq);

    seq = new KeySeqList("MoveToStartOfLine","Move to the start of line");
#ifdef Q_OS_OSX
    *seq << QKeySequence("Ctrl+Left") << QKeySequence("Shift+Ctrl+Left");
#else
    *seq << QKeySequence("Home") << QKeySequence("Shift+Home");
#endif
    setHotkey(Hotkey::MoveToStartOfLine, seq);

    seq = new KeySeqList("MoveCharGroupRight","Move to the next char-group");
    *seq << QKeySequence("Ctrl+Right");
    setHotkey(Hotkey::MoveCharGroupRight, seq);

    seq = new KeySeqList("MoveCharGroupLeft","Move to the previous char-group");
    *seq << QKeySequence("Ctrl+Left");
    setHotkey(Hotkey::MoveCharGroupLeft, seq);

    seq = new KeySeqList("SelectCharGroupRight","Select to the next char-group");
    *seq << QKeySequence("Shift+Ctrl+Right");
    setHotkey(Hotkey::SelectCharGroupRight, seq);

    seq = new KeySeqList("SelectCharGroupLeft","Select to the previous char-group");
    *seq << QKeySequence("Shift+Ctrl+Left");
    setHotkey(Hotkey::SelectCharGroupLeft, seq);

//    seq = new KeySeqList("BookmarkToggle","Set or erase bookmark of the current line");
//    *seq << QKeySequence("Ctrl+M") << QKeySequence("Meta+M");
//    setHotkey(Hotkey::BookmarkToggle, seq);

//    seq = new KeySeqList("BookmarkNext","Move to the next stored bookmark");
//    *seq << QKeySequence("Ctrl+.") << QKeySequence("Meta+.");
//    setHotkey(Hotkey::BookmarkNext, seq);

//    seq = new KeySeqList("BookmarkPrev","Move to the previous stored bookmark");
//    *seq << QKeySequence("Ctrl+,") << QKeySequence("Meta+,");
//    setHotkey(Hotkey::BookmarkPrev, seq);

    seq = new KeySeqList("Shift+Alt+Up","start block edit");
    *seq << QKeySequence("Shift+Alt+Down") << QKeySequence("Shift+Alt+Left") << QKeySequence("Shift+Alt+Right");
    setHotkey(Hotkey::BlockEditStart, seq);

    seq = new KeySeqList("Esc","end block edit");
    *seq << QKeySequence("Up") << QKeySequence("Down") << QKeySequence("Left") << QKeySequence("Right")
         << QKeySequence("PgUp") << QKeySequence("PgDown") << QKeySequence("Home") << QKeySequence("End");
    setHotkey(Hotkey::BlockEditEnd, seq);

    seq = new KeySeqList("Ctrl+F", "Open Search Dialog");
    setHotkey(Hotkey::SearchOpen, seq);

    seq = new KeySeqList("F3", "Find Next");
    setHotkey(Hotkey::SearchFindNext, seq);

    seq = new KeySeqList("Shift+F3", "Find Previous");
    setHotkey(Hotkey::SearchFindPrev, seq);

    seq = new KeySeqList("Shift+Ctrl+L","duplicate line");
    setHotkey(Hotkey::DuplicateLine, seq);

    seq = new KeySeqList("Tab","indent selected lines");
    setHotkey(Hotkey::Indent, seq);

    seq = new KeySeqList("Shift+Tab","outdent selected lines");
    *seq << QKeySequence("Shift+Backtab");
    setHotkey(Hotkey::Outdent, seq);

    seq = new KeySeqList("Ctrl+Y","remove line");
    setHotkey(Hotkey::RemoveLine, seq);

    seq = new KeySeqList("F8","goto matching parentheses");
    setHotkey(Hotkey::MatchParentheses, seq);

    seq = new KeySeqList("Shift+F8","select to matching parentheses");
    setHotkey(Hotkey::SelectParentheses, seq);
}

void Keys::read(const QJsonObject& json)
{
    Q_UNUSED(json)
}

void Keys::write(QJsonObject& json) const
{
    Q_UNUSED(json)
}

void Keys::setHotkey(Hotkey key, KeySeqList* keySeqList)
{
    mHotkeyDefs.insert(key, keySeqList);
}


} // namespace studio
} // namespace gams
