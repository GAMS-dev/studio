#ifndef FILEREPOSITORY_H
#define FILEREPOSITORY_H

#include <QtWidgets>
#include "filegroupcontext.h"
#include "filecontext.h"

namespace gams {
namespace ide {

class FileRepository : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit FileRepository(QObject *parent = nullptr);
    ~FileRepository();

    FileSystemContext* context(int id, FileSystemContext* startNode = nullptr);
    FileContext* fileContext(int id, FileSystemContext* startNode = nullptr);
    FileGroupContext* groupContext(int id, FileSystemContext* startNode = nullptr);
    QModelIndex index(FileSystemContext* entry);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QModelIndex addGroup(QString name, QString location, bool isGist, QModelIndex parentIndex = QModelIndex());
    QModelIndex addFile(QString name, QString location, bool isGist, QModelIndex parentIndex = QModelIndex());
    QModelIndex rootTreeModelIndex();
    QModelIndex rootModelIndex();
    QModelIndex findPath(const QString& filePath, QModelIndex parent);

public slots:
    void nodeNameChanged(int id, const QString &newName);
    void updatePathNode(int id, QDir dir);

private:
    FileSystemContext* node(const QModelIndex& index) const;
    FileGroupContext* group(const QModelIndex& index) const;
    void changeName(QModelIndex index, QString newName);
    QModelIndex findEntry(QString name, QString location, QModelIndex parentIndex);

private:
    int mNextId;
    FileGroupContext* mRoot;
    FileGroupContext* mTreeRoot;
};

} // namespace ide
} // namespace gams

#endif // FILEREPOSITORY_H
