#include "treeentry.h"

namespace gams {
namespace ide {

TreeEntry::TreeEntry(TreeEntry* par, QString name, QString identString, bool isGist)
    : QObject(par), mName(name), mIdentString(identString), mIsGist(isGist)
{
    if (par) {
        int sort = par->peekIndex(name, true);
        QObjectList *childrn = const_cast<QObjectList*>(&par->children());
        if (childrn->count() > 1)
            childrn->move(childrn->count()-1, sort);
    }
}

TreeEntry::~TreeEntry()
{
    // QObject should delete the entry in the parents children for us
}

bool TreeEntry::matches(const QString &name, bool isGist) const
{
    return isGist == mIsGist && mName.compare(name, Qt::CaseInsensitive) == 0;
}

TreeEntry* TreeEntry::child(int index) const
{
    if (index < 0 || index >= children().count())
        return nullptr;
    return qobject_cast<TreeEntry*>(children().at(index));
}

TreeEntry*TreeEntry::parentEntry() const
{
    return qobject_cast<TreeEntry*>(parent());
}

int TreeEntry::peekIndex(QString name, bool skipLast)
{
    int res = 0;
    // TODO(JM) need to skip the new appended entry
    int size = skipLast ? children().count()-1 : children().count();
    for (int i = 0; i < size; ++i) {
        if (child(i)->name().compare(name, Qt::CaseInsensitive) <= 0)
            res = i+1;
    }
    return res;
}

bool TreeEntry::isGist() const
{
    return mIsGist;
}

const QString &TreeEntry::name() const
{
    return mName;
}

void TreeEntry::setName(const QString& name)
{
    mName = name;
}

const QString&TreeEntry::identString() const
{
    return mIdentString;
}


} // namespace ide
} // namespace gams
