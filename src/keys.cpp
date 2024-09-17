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
#include "keys.h"

namespace gams {
namespace studio {

Keys *Keys::mInstance = nullptr;

Keys::Keys()
{

}

KeySeqList::KeySeqList(const QKeySequence& seq, const QString &title)
    : mTitle(title)
{
    mSequence << seq;
}

KeySeqList::KeySeqList(const char* seq, const QString &title)
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

bool KeySeqList::operator==(const KeySeqList &other) const
{
    bool res = mSequence.size() == other.mSequence.size();
    for (const QKeySequence &seq: mSequence) {
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

    seq = new KeySeqList("Print","print current document");
    *seq << QKeySequence("Ctrl+P");
    setHotkey(Hotkey::Print, seq);

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

    seq = new KeySeqList("BlockSelectPgUp","Extend block selection by one page up");
    *seq << QKeySequence("Shift+Alt+PgUp") << QKeySequence("Alt+PgUp");
    setHotkey(Hotkey::BlockSelectPgUp, seq);

    seq = new KeySeqList("BlockSelectPgDown","Extend block selection by one page down");
    *seq << QKeySequence("Shift+Alt+PgDown") << QKeySequence("Alt+PgDown");
    setHotkey(Hotkey::BlockSelectPgDown, seq);

    seq = new KeySeqList("MoveToEndOfDoc","Move to the end of document");
    *seq << QKeySequence(QKeySequence::MoveToEndOfDocument) << QKeySequence(QKeySequence::SelectEndOfDocument);
    setHotkey(Hotkey::MoveToEndOfDoc, seq);

    seq = new KeySeqList("MoveToStartOfDoc","Move to the start of document");
    *seq << QKeySequence(QKeySequence::MoveToStartOfDocument) << QKeySequence(QKeySequence::SelectStartOfDocument);
    setHotkey(Hotkey::MoveToStartOfDoc, seq);

    seq = new KeySeqList("MoveToEndOfLine","Move to the end of line");
    *seq << QKeySequence(QKeySequence::MoveToEndOfLine) << QKeySequence(QKeySequence::SelectEndOfLine);
#ifdef __APPLE__
    *seq << QKeySequence("Ctrl+Right");
#endif
    setHotkey(Hotkey::MoveToEndOfLine, seq);

    seq = new KeySeqList("MoveToStartOfLine","Move to the start of line");
    *seq << QKeySequence(QKeySequence::MoveToStartOfLine) << QKeySequence(QKeySequence::SelectStartOfLine);
#ifdef __APPLE__
    *seq << QKeySequence("Ctrl+Left");
#endif
    setHotkey(Hotkey::MoveToStartOfLine, seq);

    seq = new KeySeqList("MoveCharGroupRight","Move to the next char-group");
    *seq << QKeySequence(QKeySequence::MoveToNextWord) << QKeySequence(QKeySequence::SelectNextWord);
    setHotkey(Hotkey::MoveCharGroupRight, seq);

    seq = new KeySeqList("MoveCharGroupLeft","Move to the previous char-group");
    *seq << QKeySequence(QKeySequence::MoveToPreviousWord) << QKeySequence(QKeySequence::SelectPreviousWord);
    setHotkey(Hotkey::MoveCharGroupLeft, seq);

    seq = new KeySeqList("SelectAll","Select all text");
    *seq << QKeySequence("Ctrl+A");
    setHotkey(Hotkey::SelectAll, seq);

    seq = new KeySeqList("CodeCompleter","Open code completer");
#ifdef __APPLE__
    *seq << QKeySequence("Meta+Space");
#else
    *seq << QKeySequence("Ctrl+Space");
#endif
    setHotkey(Hotkey::CodeCompleter, seq);

//    seq = new KeySeqList("BookmarkToggle","Set or erase bookmark of the current line");
//    *seq << QKeySequence("Ctrl+M") << QKeySequence("Meta+M");
//    setHotkey(Hotkey::BookmarkToggle, seq);

//    seq = new KeySeqList("BookmarkNext","Move to the next stored bookmark");
//    *seq << QKeySequence("Ctrl+.") << QKeySequence("Meta+.");
//    setHotkey(Hotkey::BookmarkNext, seq);

//    seq = new KeySeqList("BookmarkPrev","Move to the previous stored bookmark");
//    *seq << QKeySequence("Ctrl+,") << QKeySequence("Meta+,");
//    setHotkey(Hotkey::BookmarkPrev, seq);

#ifdef __APPLE__
    seq = new KeySeqList("Meta+Shift+Up", "start block edit");
    *seq << QKeySequence("Meta+Shift+Down") << QKeySequence("Meta+Shift+Left")
         << QKeySequence("Meta+Shift+Right") << QKeySequence("Alt+Shift+Up") << QKeySequence("Alt+Shift+Down");
    setHotkey(Hotkey::BlockEditStart, seq);
#else
    seq = new KeySeqList("Shift+Alt+Up","start block edit");
    *seq << QKeySequence("Shift+Alt+Down") << QKeySequence("Shift+Alt+Left") << QKeySequence("Shift+Alt+Right");
    setHotkey(Hotkey::BlockEditStart, seq);
#endif
    seq = new KeySeqList("Esc","end block edit");
    *seq << QKeySequence("Alt+Left") << QKeySequence("Alt+Right")
         << QKeySequence("PgUp") << QKeySequence("PgDown");
    setHotkey(Hotkey::BlockEditEnd, seq);

    seq = new KeySeqList("Ctrl+F", "Open Search Dialog");
    setHotkey(Hotkey::SearchOpen, seq);

    seq = new KeySeqList("F3", "Find Next");
    setHotkey(Hotkey::SearchFindNext, seq);

    seq = new KeySeqList("Shift+F3", "Find Previous");
    setHotkey(Hotkey::SearchFindPrev, seq);

    seq = new KeySeqList("Ctrl+D","duplicate line");
    setHotkey(Hotkey::DuplicateLine, seq);

    seq = new KeySeqList("Alt+L", "Toggle folding of current block");
    setHotkey(Hotkey::ToggleBlockFolding, seq);

    seq = new KeySeqList("Tab","indent selected lines");
    setHotkey(Hotkey::Indent, seq);

    seq = new KeySeqList("Shift+Tab","outdent selected lines");
    *seq << QKeySequence("Shift+Backtab");
    setHotkey(Hotkey::Outdent, seq);

    seq = new KeySeqList("Ctrl+B","goto matching parentheses");
    setHotkey(Hotkey::MatchParentheses, seq);

    seq = new KeySeqList("Ctrl+Shift+B","select to matching parentheses");
    setHotkey(Hotkey::SelectParentheses, seq);

    seq = new KeySeqList("Ctrl+Shift+M","set current file as main file");
    setHotkey(Hotkey::SetMainFile, seq);

    seq = new KeySeqList("F1","Open Help View");
    setHotkey(Hotkey::OpenHelp, seq);

    seq = new KeySeqList("F2","jump to context");
    setHotkey(Hotkey::JumpToContext, seq);

}

void Keys::read(const QVariantMap& keys)
{
    Q_UNUSED(keys)
}

void Keys::write(QVariantMap& keys) const
{
    Q_UNUSED(keys)
}

void Keys::setHotkey(Hotkey key, KeySeqList* keySeqList)
{
    mHotkeyDefs.insert(key, keySeqList);
}


} // namespace studio
} // namespace gams
