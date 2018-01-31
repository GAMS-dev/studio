#ifndef KEYS_H
#define KEYS_H

#include "QtGui"
#include "studiosettings.h"

namespace gams {
namespace studio {

enum class Hotkey {
    // when key-setup can be persisted ensure the assignments won't change any more

    NewLine = 1,
    Cut = 10,
    Copy = 11,
    Paste = 12,
    Undo = 13,
    Redo = 14,
    BlockEditStart = 51,
    BlockEditEnd = 52,

    // configurable key-sequences > 100
    MinConfigurable = 100,
    DuplicateLine = 101,
    RemoveLine = 102,
    Indent = 103,
    Outdent = 104,

};

class KeySeqList
{
public:
    KeySeqList(const QKeySequence &seq, QString title);
    KeySeqList(const char *seq, QString title);
    KeySeqList& operator =(const KeySeqList &other);
    KeySeqList& operator =(const QKeySequence &other);
    KeySeqList& operator <<(const KeySeqList &other);
    KeySeqList& operator <<(const QKeySequence& other);
    inline bool matches(QKeyEvent *e) const {
        uint searchkey = (e->modifiers() | e->key()) & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier);
        return mSequence.contains(QKeySequence(searchkey));
    }
    inline bool contains(int keycode) const {
        for (QKeySequence seq: mSequence) {
            for (int i = 0; i < seq.count(); ++i) {
                int mask = seq[i] & ~Qt::KeyboardModifierMask;
                if (mask == keycode) return true;
            }
        }
        return false;
    }

    bool operator ==(KeySeqList other) const;
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
    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;
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

inline bool operator==(QKeyEvent *e, KeySeqList keySeq) { return (e ? keySeq.matches(e) : false); }
inline bool operator==(KeySeqList keySeq, QKeyEvent *e) { return (e ? keySeq.matches(e) : false); }
inline bool operator!=(QKeyEvent *e, KeySeqList keySeq) { return !(e ? keySeq.matches(e) : false); }
inline bool operator!=(KeySeqList keySeq, QKeyEvent *e) { return !(e ? keySeq.matches(e) : false); }

constexpr inline uint qHash(Hotkey key) noexcept { return uint(key); }


} // namespace studio
} // namespace gams

#endif // KEYS_H
