#ifndef KEYS_H
#define KEYS_H

#include "QtGui"
#include "studiosettings.h"

namespace gams {
namespace studio {

enum class Hotkey {
    BlockEdit,
    DuplicateLine,

};

class KeySeqList
{
public:
    KeySeqList(const QKeySequence &seq, QString title);
    KeySeqList(const char *seq, QString title);
    KeySeqList& operator =(const KeySeqList &other);
    KeySeqList& operator =(const QKeySequence &other);
    KeySeqList& operator <<(const KeySeqList &other);
    KeySeqList& operator <<(const QKeySequence &other);
    bool matches(QKeyEvent *e);
    bool operator ==(KeySeqList other);
private:
    QList<QKeySequence> mSequence;
    QString mTitle;
};

class Keys
{
public:
    static Keys &instance();
    void reset();
    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;
    void setHotkey(Hotkey key, KeySeqList* keySeqList);
private:
    static Keys *mInstance;
    Keys();
    QHash<Hotkey, KeySeqList*> mHotkeyDefs;
};

inline bool operator==(QKeyEvent *e, KeySeqList keySeq) { return (e ? keySeq.matches(e) : false); }
inline bool operator==(KeySeqList keySeq, QKeyEvent *e) { return (e ? keySeq.matches(e) : false); }
constexpr inline uint qHash(Hotkey key) noexcept { return uint(key); }


} // namespace studio
} // namespace gams

#endif // KEYS_H
