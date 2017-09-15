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

    FileContext* fileContext(int id, FileSystemContext* startNode = nullptr);
    FileSystemContext* context(int id, FileSystemContext* startNode = nullptr);
    virtual QModelIndex index(FileSystemContext* entry);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QModelIndex addGroup(QString name, QString location, bool isGist, QModelIndex parentIndex = QModelIndex());
    QModelIndex addFile(QString name, QString location, bool isGist, QModelIndex parentIndex = QModelIndex());
    QModelIndex rootTreeModelIndex();
    QModelIndex rootModelIndex();
    QModelIndex find(const QString& filePath, QModelIndex parent);

public slots:
    void nodeNameChanged(int id, const QString &newName);

private:
    FileSystemContext* node(const QModelIndex& index) const;
    FileGroupContext* group(const QModelIndex& index) const;
    void changeName(QModelIndex index, QString newName);

private:
    int mNextId;
    FileGroupContext* mRoot;
    FileGroupContext* mTreeRoot;
};

} // namespace ide
} // namespace gams

#endif // FILEREPOSITORY_H
