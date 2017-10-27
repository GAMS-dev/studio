#ifndef FILETREEMODEL_H
#define FILETREEMODEL_H

#include <QtWidgets>
#include "filegroupcontext.h"

namespace gams {
namespace studio {

class FileRepository;

class FileTreeModel : public QAbstractItemModel
{
public:
    explicit FileTreeModel(FileRepository *parent, FileGroupContext* root);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QModelIndex index(FileSystemContext* entry);
    QModelIndex rootModelIndex() const;
    FileGroupContext* rootContext() const;

protected:
    friend class FileRepository;

    bool insertChild(int row, FileGroupContext* parent, FileSystemContext* child);
    bool removeChild(FileSystemContext* child);

    bool isCurrent(const QModelIndex& index) const;
    void setCurrent(const QModelIndex& index);

private:
    FileRepository *mFileRepo;
    FileGroupContext* mRoot = nullptr;
    QModelIndex mCurrent;

};

} // namespace studio
} // namespace gams

#endif // FILETREEMODEL_H
