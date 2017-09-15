#include "filerepository.h"

namespace gams {
namespace ide {

FileRepository::FileRepository(QObject* parent)
    : QAbstractItemModel(parent), mNextId(0)
{
    mRoot = new FileGroupContext(nullptr, mNextId++, "Root", "", false);
    mTreeRoot = new FileGroupContext(mRoot, mNextId++, "TreeRoot", "", false);
}

FileRepository::~FileRepository()
{
    delete mRoot;
}

FileContext*FileRepository::fileContext(int id, FileSystemContext* startNode)
{
    return static_cast<FileContext*>(context(id, startNode));
}

FileSystemContext* FileRepository::context(int id, FileSystemContext* startNode)
{
    if (!startNode)
        startNode = mRoot;
    if (startNode->id() == id)
        return startNode;
    for (int i = 0; i < startNode->children().size(); ++i) {
        FileSystemContext* entry = context(id, startNode->child(i));
        if (entry) return entry;
    }
    return nullptr;
}

QModelIndex FileRepository::index(FileSystemContext *entry)
{
     if (!entry)
         return QModelIndex();
     if (!entry->parent())
         return createIndex(0, 0, entry);
     for (int i = 0; i < entry->parentEntry()->children().size(); ++i) {
         if (entry->parentEntry()->child(i) == entry) {
             return createIndex(i, 0, entry);
         }
     }
     return QModelIndex();
}

QModelIndex FileRepository::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    return createIndex(row, column, node(parent)->child(row));
}

QModelIndex FileRepository::parent(const QModelIndex& child) const
{
    FileSystemContext* eChild = node(child);
    if (eChild == mTreeRoot || eChild == mRoot)
        return QModelIndex();
    FileSystemContext* eParent = eChild->parentEntry();
    if (eParent == mTreeRoot || eParent == mRoot)
        return createIndex(0, child.column(), eParent);
    int row = eParent->children().indexOf(eChild);
    if (row < 0) {
        qDebug() << "could not find child in parent";
        throw std::runtime_error("could not find child in parent");
    }
    return createIndex(row, child.column(), eParent);
}

int FileRepository::rowCount(const QModelIndex& parent) const
{
    FileSystemContext* entry = node(parent);
    if (!entry) return 0;
    return entry->children().count();
}

int FileRepository::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant FileRepository::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    switch (role) {

    case Qt::DisplayRole:
        return node(index)->name();
        break;

    case Qt::FontRole:
        if (node(index)->active()) {
            QFont f;
            f.setBold(true);
            return f;
        }
        break;

    case Qt::ForegroundRole:
        if (node(index)->active())
            return QColor(Qt::blue);
        break;

    case Qt::ToolTipRole:
        return node(index)->location();
        break;

    default:
        break;
    }
    return QVariant();
}

QModelIndex FileRepository::addGroup(QString name, QString location, bool isGist, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = rootModelIndex();
    int offset = node(parentIndex)->peekIndex(name);
    beginInsertRows(parentIndex, offset, offset);
    FileGroupContext* fgContext = new FileGroupContext(group(parentIndex), mNextId++, name, location, isGist);
    endInsertRows();
    connect(fgContext, &FileGroupContext::nameChanged, this, &FileRepository::nodeNameChanged);
    return index(offset, 0, parentIndex);
}

QModelIndex FileRepository::addFile(QString name, QString location, bool isGist, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = rootModelIndex();
    int offset = node(parentIndex)->peekIndex(name);
    beginInsertRows(parentIndex, offset, offset);
    FileContext* fContext = new FileContext(group(parentIndex), mNextId++, name, location, isGist);
    endInsertRows();
    connect(fContext, &FileContext::nameChanged, this, &FileRepository::nodeNameChanged);
    return index(offset, 0, parentIndex);
}

QModelIndex FileRepository::rootTreeModelIndex()
{
    return createIndex(0, 0, mTreeRoot);
}

QModelIndex FileRepository::rootModelIndex()
{
    return createIndex(0, 0, mRoot);
}

QModelIndex FileRepository::find(const QString &filePath, QModelIndex parent)
{
    FileSystemContext* par = node(parent);
    for (int i = 0; i < par->children().count(); ++i) {
        FileSystemContext* child = par->child(i);
        if (QFileInfo(child->location()) == QFileInfo(filePath)) {
            return createIndex(i, 0, child);
        }
    }
    return QModelIndex();
}

void FileRepository::nodeNameChanged(int id, const QString& newName)
{
    // TODO(JM) FIXME need to retrieve QModelIndex for ctx
    FileSystemContext* nd = context(id);
    if (!nd) return;

    QModelIndex ndIndex = index(nd);
    dataChanged(ndIndex, ndIndex);
}

FileSystemContext*FileRepository::node(const QModelIndex& index) const
{
    return static_cast<FileSystemContext*>(index.internalPointer());
}

FileGroupContext*FileRepository::group(const QModelIndex& index) const
{
    return static_cast<FileGroupContext*>(index.internalPointer());
}

void FileRepository::changeName(QModelIndex index, QString newName)
{
    node(index)->setName(newName);
    dataChanged(index, index);
}

} // namespace ide
} // namespace gams
