#ifndef KEYS_H
#define KEYS_H

#include "QtGui"
#include "studiosettings.h"

namespace gams {
namespace studio {

enum class Hotkey {
    BlockEdit,

};

class KeySeqList
{
public:
    KeySeqList(const QKeySequence &seq, QString title);
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
    bool init(StudioSettings &settings);
private:
    Keys();
    QHash<Hotkey, KeySeqList> mHotkeyDefs;
};

inline bool operator==(QKeyEvent *e, KeySeqList keySeq) { return (e ? keySeq.matches(e) : false); }
inline bool operator==(KeySeqList keySeq, QKeyEvent *e) { return (e ? keySeq.matches(e) : false); }

} // namespace studio
} // namespace gams

#endif // KEYS_H
