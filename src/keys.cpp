#include "keys.h"


namespace gams {
namespace studio {

Keys *Keys::mInstance = nullptr;

Keys::Keys()
{

}

Keys&Keys::instance()
{
    if (!mInstance) {
        mInstance = new Keys();
        mInstance->reset();
    }
    return *mInstance;
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

bool KeySeqList::matches(QKeyEvent* e)
{
    uint searchkey = (e->modifiers() | e->key()) & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier);
    return mSequence.contains(QKeySequence(searchkey));
}

void Keys::reset()
{
    KeySeqList *seq = new KeySeqList("Shift+Alt+Up","start block edit");
    *seq << QKeySequence("Shift+Alt+Down");
    setHotkey(Hotkey::BlockEdit, seq);
    seq = new KeySeqList("Ctrl-L","duplicate line");
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
