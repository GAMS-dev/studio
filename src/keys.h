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
#ifndef KEYS_H
#define KEYS_H

#include "settings.h"

#include <QKeyEvent>
#include <QKeySequence>

namespace gams {
namespace studio {

enum struct Hotkey {
    // when key-setup can be persisted ensure the assignments won't change any more

    NewLine = 1,
    Print = 2,
    Cut = 10,
    Copy = 11,
    Paste = 12,
    Undo = 13,
    Redo = 14,
    MoveViewLineUp = 15,
    MoveViewLineDown = 16,
    MoveViewPageUp = 17,
    MoveViewPageDown = 18,
    BlockSelectPgUp = 19,
    BlockSelectPgDown = 20,
    MoveToEndOfDoc = 21,
    MoveToStartOfDoc = 22,
    MoveToEndOfLine = 23,
    MoveToStartOfLine = 24,
    MoveCharGroupRight = 25,
    MoveCharGroupLeft = 26,
    SelectAll = 27,
    CodeCompleter = 28,

//    BookmarkToggle = 30,
//    BookmarkNext = 31,
//    BookmarkPrev = 32,
    BlockEditStart = 51,
    BlockEditEnd = 52,
    SearchOpen = 60,
    SearchFindNext = 61,
    SearchFindPrev = 62,
    ToggleBlockFolding = 63,
    OpenHelp = 64,

    // configurable key-sequences > 100
    MinConfigurable = 100,
    DuplicateLine = 101,
    RemoveLine = 102,
    Indent = 103,
    Outdent = 104,
    MatchParentheses = 105,
    SelectParentheses = 106,
    SetMainFile = 107,
    JumpToContext = 108,

};

class KeySeqList
{
public:
    KeySeqList(const QKeySequence &seq, const QString &title);
    KeySeqList(const char *seq, const QString &title);
    KeySeqList& operator =(const KeySeqList &other);
    KeySeqList& operator =(const QKeySequence &other);
    KeySeqList& operator <<(const KeySeqList &other);
    KeySeqList& operator <<(const QKeySequence& other);
    inline bool matches(QKeyEvent *e) const {
        int searchkey = (e->modifiers() | e->key()) & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier);
        return mSequence.contains(QKeySequence(searchkey));
    }
    inline bool contains(int keycode) const {
        for (const QKeySequence &seq: mSequence) {
            for (int i = 0; i < seq.count(); ++i) {
                int mask = seq[uint(i)].toCombined() & ~Qt::KeyboardModifierMask;
                if (mask == keycode) return true;
            }
        }
        return false;
    }
    QKeySequence first() const { return mSequence.isEmpty() ? QKeySequence() : mSequence.first(); }
    bool operator==(const KeySeqList &other) const;
private:
    QList<QKeySequence> mSequence;
    QString mTitle;
};

class Keys
{
public:
    inline static Keys &instance() {
        if (!mInstance) {
            mInstance = new Keys();
            mInstance->reset();
        }
        return *mInstance;
    }

    void reset();
    void read(const QVariantMap &json);
    void write(QVariantMap &keys) const;
    void setHotkey(Hotkey key, KeySeqList* keySeqList);
    inline const KeySeqList &keySequence(Hotkey hotkey) const {
        return *mHotkeyDefs.value(hotkey, mDefault);
    }
private:
    static Keys *mInstance;
    KeySeqList *mDefault;
    QHash<Hotkey, KeySeqList*> mHotkeyDefs;
private:
    Keys();
};

inline bool operator==(QKeyEvent *e, Hotkey hotkey) { return (e ? Keys::instance().keySequence(hotkey).matches(e) : false); }
inline bool operator==(Hotkey hotkey, QKeyEvent *e) { return (e ? Keys::instance().keySequence(hotkey).matches(e) : false); }
inline bool operator!=(QKeyEvent *e, Hotkey hotkey) { return !(e ? Keys::instance().keySequence(hotkey).matches(e) : false); }
inline bool operator!=(Hotkey hotkey, QKeyEvent *e) { return !(e ? Keys::instance().keySequence(hotkey).matches(e) : false); }

inline bool operator==(int keycode, Hotkey hotkey) { return Keys::instance().keySequence(hotkey).contains(keycode); }
inline bool operator==(Hotkey hotkey, int keycode) { return Keys::instance().keySequence(hotkey).contains(keycode); }
inline bool operator!=(int keycode, Hotkey hotkey) { return !Keys::instance().keySequence(hotkey).contains(keycode); }
inline bool operator!=(Hotkey hotkey, int keycode) { return !Keys::instance().keySequence(hotkey).contains(keycode); }

inline bool operator==(QKeyEvent *e, const KeySeqList& keySeq) { return (e ? keySeq.matches(e) : false); }
inline bool operator==(const KeySeqList& keySeq, QKeyEvent *e) { return (e ? keySeq.matches(e) : false); }
inline bool operator!=(QKeyEvent *e, const KeySeqList& keySeq) { return !(e ? keySeq.matches(e) : false); }
inline bool operator!=(const KeySeqList& keySeq, QKeyEvent *e) { return !(e ? keySeq.matches(e) : false); }

constexpr inline uint qHash(Hotkey key) noexcept { return uint(key); }


} // namespace studio
} // namespace gams

#endif // KEYS_H
