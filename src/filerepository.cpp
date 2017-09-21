#include "filerepository.h"
#include "exception.h"

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

FileSystemContext* FileRepository::context(int fileId, FileSystemContext* startNode)
{
    if (!startNode)
        throw FATAL() << "missing startNode";
    if (startNode->id() == fileId)
        return startNode;
    for (int i = 0; i < startNode->childCount(); ++i) {
        FileSystemContext* iChild = startNode->childEntry(i);
        if (!iChild)
            throw FATAL() << "child must not be null";
        FileSystemContext* entry = context(fileId, iChild);
        if (entry) return entry;
    }
    return nullptr;
}

FileContext* FileRepository::fileContext(int fileId, FileSystemContext* startNode)
{
    return static_cast<FileContext*>(context(fileId, (startNode ? startNode : mRoot)));
}

FileGroupContext*FileRepository::groupContext(int fileId, FileSystemContext* startNode)
{
    return static_cast<FileGroupContext*>(context(fileId, (startNode ? startNode : mRoot)));
}

QModelIndex FileRepository::index(FileSystemContext *entry)
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

QModelIndex FileRepository::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    return createIndex(row, column, node(parent)->childEntry(row));
}

QModelIndex FileRepository::parent(const QModelIndex& child) const
{
    FileSystemContext* eChild = node(child);
    if (eChild == mTreeRoot || eChild == mRoot)
        return QModelIndex();
    FileGroupContext* eParent = eChild->parentEntry();
    if (eParent == mTreeRoot || eParent == mRoot)
        return createIndex(0, child.column(), eParent);
    int row = eParent->indexOf(eChild);
    if (row < 0)
        throw FATAL() << "could not find child in parent";
    return createIndex(row, child.column(), eParent);
}

int FileRepository::rowCount(const QModelIndex& parent) const
{
    FileSystemContext* entry = node(parent);
    if (!entry) return 0;
    return entry->childCount();
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
        if (node(index)->flags().testFlag(FileSystemContext::cfActive)) {
            QFont f;
            f.setBold(true);
            return f;
        }
        break;

    case Qt::ForegroundRole: {
        FileSystemContext::ContextFlags flags = node(index)->flags();
        if (flags.testFlag(FileSystemContext::cfMissing))
            return QColor(Qt::red);
        if (flags.testFlag(FileSystemContext::cfActive)) {
            return flags.testFlag(FileSystemContext::cfGroup) ? QColor(Qt::black)
                                                              : QColor(Qt::blue);
        }
        break;
    }

    case Qt::ToolTipRole:
        return node(index)->location();
        break;

    default:
        break;
    }
    return QVariant();
}

QModelIndex FileRepository::findEntry(QString name, QString location, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = rootModelIndex();
    FileGroupContext *par = group(parentIndex);
    if (!par)
        throw FATAL() << "Can't get parent object";

    bool exact;
    int offset = par->peekIndex(name, &exact);
    if (exact) {
        FileSystemContext *fc = par->childEntry(offset);
        if (fc->location().compare(location, Qt::CaseInsensitive) == 0) {
            return index(offset, 0, parentIndex);
        }
    }
    return QModelIndex();
}

QModelIndex FileRepository::addGroup(QString name, QString location, bool isGist, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = rootModelIndex();
    FileGroupContext *par = group(parentIndex);
    if (!par)
        throw FATAL() << "Can't get parent object";

    bool exact;
    int offset = par->peekIndex(name, &exact);
    if (exact) {
        FileSystemContext *fc = par->childEntry(offset);
        fc->setLocation(location);
        qDebug() << "found dir " << name << " for " << location << " at pos=" << offset;
        return index(offset, 0, parentIndex);
    }

    beginInsertRows(parentIndex, offset, offset);
    FileGroupContext* fgContext = new FileGroupContext(group(parentIndex), mNextId++, name, location, isGist);
    if (isGist) {
        QFileSystemWatcher *watcher = new QFileSystemWatcher(fgContext);
        connect(fgContext, &FileGroupContext::contentChanged, this, &FileRepository::updatePathNode);
        connect(watcher, &QFileSystemWatcher::directoryChanged, fgContext, &FileGroupContext::directoryChanged);
        watcher->addPath(location);
    }
    endInsertRows();
    connect(fgContext, &FileGroupContext::nameChanged, this, &FileRepository::nodeNameChanged);
    qDebug() << "added dir " << name << " for " << location << " at pos=" << offset;
    updatePathNode(fgContext->id(), fgContext->location());
    return index(offset, 0, parentIndex);
}

QModelIndex FileRepository::addFile(QString name, QString location, bool isGist, QModelIndex parentIndex)
{
    if (!parentIndex.isValid())
        parentIndex = rootModelIndex();
    FileGroupContext *par = group(parentIndex);
    if (!par)
        throw FATAL() << "Can't get parent object";

    bool exact;
    int offset = par->peekIndex(name, &exact);
    if (exact) {
        FileSystemContext *fc = par->childEntry(offset);
        fc->setLocation(location);
        qDebug() << "found file " << name << " for " << location << " at pos=" << offset;
        return index(offset, 0, parentIndex);
    }
    beginInsertRows(parentIndex, offset, offset);
    FileContext* fContext = new FileContext(group(parentIndex), mNextId++, name, location, isGist);
    endInsertRows();
    connect(fContext, &FileContext::nameChanged, this, &FileRepository::nodeNameChanged);
    qDebug() << "added file " << name << " for " << location << " at pos=" << offset;
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

QModelIndex FileRepository::findPath(const QString &filePath, QModelIndex parent)
{
    FileSystemContext* par = node(parent);
    for (int i = 0; i < par->childCount(); ++i) {
        FileSystemContext* child = par->childEntry(i);
        if (QFileInfo(child->location()) == QFileInfo(filePath)) {
            return createIndex(i, 0, child);
        }
    }
    return QModelIndex();
}

void FileRepository::close(int fileId)
{
    FileContext *fc = fileContext(fileId);
    fc->setDocument(nullptr);
    QModelIndex fci = index(fc);
    dataChanged(fci, fci);
    emit fileClosed(fileId);
}

void FileRepository::nodeNameChanged(int fileId, const QString& newName)
{
    Q_UNUSED(newName);
    FileSystemContext* nd = context(fileId, mRoot);
    if (!nd) return;

    QModelIndex ndIndex = index(nd);
    dataChanged(ndIndex, ndIndex);
}

typedef QPair<int, FileSystemContext*> IndexedFSContext;

void FileRepository::updatePathNode(int fileId, QDir dir)
{
    FileGroupContext *gc = groupContext(fileId, mRoot);
    if (!gc)
        throw QException();
    if (dir.exists()) {
        QDir::Filters dirFilter = QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot;
        QStringList fileList = dir.entryList(dirFilter, QDir::DirsLast);
        qSort(fileList);
        // remove known entries from fileList and remember vanished entries
        QList<IndexedFSContext> vanishedEntries;
        for (int i = 0; i < gc->childCount(); ++i) {
            FileSystemContext *entry = gc->childEntry(i);
            QRegExp rx(entry->location());
            int pos = fileList.indexOf(rx);
            if (pos >= 0) {
                fileList.removeAt(pos);
                if (qobject_cast<FileGroupContext*>(entry)) {
                    updatePathNode(entry->id(), entry->location());
                }
            } else {
                // prepare indicees in reverse order (highest index first)
                vanishedEntries.insert(0, IndexedFSContext(i, entry));
            }
        }
        // check for vanished files and directories
        for (IndexedFSContext childIndex: vanishedEntries) {
            FileSystemContext* entry = childIndex.second;
            if (entry->flags().testFlag(FileSystemContext::cfGroup)) {
                updatePathNode(entry->id(), entry->location());
            }
            if (entry->flags().testFlag(FileSystemContext::cfActive)) {
                // mark active files as missing (directories recursively)
                entry->setFlag(FileSystemContext::cfMissing);
            } else {
                // inactive files can be removed (directories recursively)
                entry->setParent(nullptr);
                delete entry;
            }
        }
        // add newly appeared files and directories
        for (QString fsEntry: fileList) {
            QFileInfo fi(dir, fsEntry);
            if (fi.isDir()) {
                QModelIndex grInd = addGroup(fsEntry, fi.canonicalFilePath(), false, index(gc));
                FileGroupContext *fg = group(grInd);
                updatePathNode(fg->id(), fg->location());
            } else {
                addFile(fsEntry, fi.canonicalFilePath(), false, index(gc));
            }
        }
    }
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
