#include "treemodel.h"

namespace gams {
namespace ide {

TreeModel::TreeModel(QObject* parent)
    : QAbstractItemModel(parent), mRoot(new TreeEntry(nullptr, "Root", false))
{

    // Testdata
    TreeEntry* block = new TreeEntry(mRoot, "Block-1", "/first/block/data/", true);
//    new TreeEntry(block, "Entry-1", true);
//    new TreeEntry(block, "Entry-2", true);
//    new TreeEntry(block, "Entry-3", true);
//    block = new TreeEntry(mRoot, "Block-2", true);
//    new TreeEntry(block, "Entry-1", true);
}

TreeModel::~TreeModel()
{
    delete mRoot;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    return createIndex(row, column, entry(parent)->child(row));
}

QModelIndex TreeModel::parent(const QModelIndex& child) const
{
    TreeEntry* eChild = entry(child);
    if (eChild == mRoot)
        return QModelIndex();
    TreeEntry* eParent = eChild->parentEntry();
    if (eParent == mRoot)
        return createIndex(0, child.column(), eParent);
    int row = eParent->children().indexOf(eChild);
    if (row < 0) {
        qDebug() << "could not find child in parent";
        throw std::exception("could not find child in parent");
    }
    return createIndex(row, child.column(), eParent);
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    TreeEntry* entr = entry(parent);
    if (!entr) return 0;
    return entr->children().count();
}

int TreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    switch (role) {
    case Qt::DisplayRole:
        return entry(index)->name();
        break;
    case Qt::ToolTipRole:
        return entry(index)->identString();
        break;
    default:
        return QVariant();
        break;
    }
}

QModelIndex TreeModel::addEntry(QString name, QString identString, bool isGist, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = rootModelIndex();
    int offset = entry(parentIndex)->peekIndex(name);
    beginInsertRows(parentIndex, offset, offset);
    new TreeEntry(entry(parentIndex), name, identString, isGist);
    endInsertRows();
    return index(offset, 0, parentIndex);
}

QModelIndex TreeModel::rootModelIndex()
{
    return createIndex(0, 0, mRoot);
}

QModelIndex TreeModel::find(const QString &identString, QModelIndex parent)
{
    TreeEntry* par = entry(parent);
    for (int i = 0; i < par->children().count(); ++i) {
        TreeEntry* child = par->child(i);
        if (QFileInfo(child->identString()) == QFileInfo(identString)) {
            return createIndex(i, 0, child);
        }
    }
    return QModelIndex();
}

void TreeModel::entryNameChanged(const QString& identString, const QString& newName)
{
    QFileInfo fi(identString);
    QModelIndex par = find(fi.path(), rootModelIndex());
    if (!par.isValid()) {
        // maybe a root child has changed
        par = find(fi.filePath(), rootModelIndex());
        if (par.isValid()) {
            changeName(par, newName);
        }
    } else {
        QModelIndex entr = find(fi.filePath(), par);
        if (entr.isValid()) {
            changeName(entr, newName);
        }
    }
}

TreeEntry*TreeModel::entry(const QModelIndex& index) const
{
    return static_cast<TreeEntry*>(index.internalPointer());
}

void TreeModel::changeName(QModelIndex index, QString newName)
{
    entry(index)->setName(newName);
    dataChanged(index, index);
}

} // namespace ide
} // namespace gams
