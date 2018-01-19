#include "keys.h"

namespace gams {
namespace studio {

KeySeqList::KeySeqList(const QKeySequence& seq, QString title)
{
    mSequence << seq;
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

bool KeySeqList::matches(QKeyEvent* e)
{
    uint searchkey = (e->modifiers() | e->key()) & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier);
    return mSequence.contains(QKeySequence(searchkey));
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

Keys::Keys()
{

}

} // namespace studio
} // namespace gams
