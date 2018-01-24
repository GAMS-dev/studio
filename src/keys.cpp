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

    seq = new KeySeqList("Shift+Alt+Up","start block edit");
    *seq << QKeySequence("Shift+Alt+Down") << QKeySequence("Shift+Alt+Left") << QKeySequence("Shift+Alt+Right");
    setHotkey(Hotkey::BlockEditStart, seq);

    seq = new KeySeqList("Up","end block edit");
    *seq << QKeySequence("Down") << QKeySequence("Left") << QKeySequence("Right")
         << QKeySequence("PgUp") << QKeySequence("PgDown") << QKeySequence("Home") << QKeySequence("End");
    setHotkey(Hotkey::BlockEditEnd, seq);

    seq = new KeySeqList("Shift+Ctrl+L","duplicate line");
    setHotkey(Hotkey::DuplicateLine, seq);
}

void Keys::read(const QJsonObject& json)
{

}

void Keys::write(QJsonObject& json) const
{

}

void Keys::setHotkey(Hotkey key, KeySeqList* keySeqList)
{
    mHotkeyDefs.insert(key, keySeqList);
}


} // namespace studio
} // namespace gams
