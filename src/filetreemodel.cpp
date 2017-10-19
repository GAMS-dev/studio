#include "filetreemodel.h"

#include "exception.h"
#include "filerepository.h"
#include "filecontext.h"
#include "filegroupcontext.h"
#include "fileactioncontext.h"

namespace gams {
namespace studio {

FileTreeModel::FileTreeModel(FileRepository* parent, FileGroupContext* root)
    : QAbstractItemModel(parent), mFileRepo(parent), mRoot(root)
{
    if (!mFileRepo)
        FATAL() << "nullptr not allowed. The FileTreeModel needs a valid FileRepository.";
}

QModelIndex FileTreeModel::index(FileSystemContext *entry)
{
     if (!entry)
         return QModelIndex();
     if (!entry->parent())
         return createIndex(0, 0, entry);
     for (int i = 0; i < entry->parentEntry()->childCount(); ++i) {
         if (entry->parentEntry()->childEntry(i) == entry) {
             return createIndex(i, 0, entry);
         }
     }
     return QModelIndex();
}

QModelIndex FileTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    FileSystemContext *fc = mFileRepo->context(parent)->childEntry(row);
    if (!fc)
        FATAL() << "invalid child for row " << row;
    return createIndex(row, column, fc);
}

QModelIndex FileTreeModel::parent(const QModelIndex& child) const
{
    FileSystemContext* eChild = mFileRepo->context(child);
    if (eChild == mRoot)
        return QModelIndex();
    FileGroupContext* eParent = eChild->parentEntry();
    if (!eParent)
        return QModelIndex();
    if (eParent == mRoot)
        return createIndex(0, child.column(), eParent);
    int row = eParent->indexOf(eChild);
    if (row < 0)
        FATAL() << "could not find child in parent";
    return createIndex(row, child.column(), eParent);
}

int FileTreeModel::rowCount(const QModelIndex& parent) const
{
    FileSystemContext* entry = mFileRepo->context(parent);
    if (!entry) return 0;
    return entry->childCount();
}

int FileTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant FileTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    switch (role) {

    case Qt::BackgroundColorRole:
        if (isCurrent(index)) return QColor("#4466BBFF");

    case Qt::DisplayRole:
        return mFileRepo->context(index)->caption();

    case Qt::FontRole:
        if (mFileRepo->context(index)->flags().testFlag(FileSystemContext::cfActive)) {
            QFont f;
            f.setBold(true);
            return f;
        }
        if (mFileRepo->context(index)->type() == FileSystemContext::FileAction) {
            QFont f;
            f.setItalic(true);
            return f;
        }
        break;

    case Qt::ForegroundRole: {
        FileSystemContext::ContextFlags flags = mFileRepo->context(index)->flags();
        if (flags.testFlag(FileSystemContext::cfMissing))
            return QColor(Qt::red);
        if (flags.testFlag(FileSystemContext::cfActive)) {
            return (isCurrent(index)) ? QColor(Qt::blue)
                                      : QColor(Qt::black);
        }
        break;
    }

    case Qt::DecorationRole:
        return mFileRepo->context(index)->icon();

    case Qt::ToolTipRole:
        return mFileRepo->context(index)->location();

    default:
        break;
    }
    return QVariant();
}

QModelIndex FileTreeModel::rootModelIndex() const
{
    return createIndex(0, 0, mRoot);
}

FileGroupContext* FileTreeModel::rootContext() const
{
    return mRoot;
}

bool FileTreeModel::insertChild(int row, FileGroupContext* parent, FileSystemContext* child)
{
    QModelIndex parMi = index(parent);
    if (!parMi.isValid()) return false;
    beginInsertRows(parMi, row, row);
    child->setParentEntry(parent);
    endInsertRows();
    return true;
}

bool FileTreeModel::removeChild(FileSystemContext* child)
{
    QModelIndex mi = index(child);
    if (!mi.isValid()) return false;
    beginRemoveRows(mi, mi.row(), mi.row());
    child->setParentEntry(nullptr);
    endInsertRows();
    return true;
}

bool FileTreeModel::isCurrent(const QModelIndex& index) const
{
    return (mCurrent.isValid() && index == mCurrent);
}

void FileTreeModel::setCurrent(const QModelIndex& index)
{
    if (!isCurrent(index)) {
        QModelIndex mi = mCurrent;
        mCurrent = index;
        if (mi.isValid()) dataChanged(mi, mi);                      // invalidate old
        if (mCurrent.isValid()) dataChanged(mCurrent, mCurrent);    // invalidate new
    }
}

} // namespace studio
} // namespace gams
