#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QtWidgets>
#include "treeentry.h"

namespace gams {
namespace ide {

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeModel(QObject *parent = nullptr);
    ~TreeModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QModelIndex addEntry(QString name, QString identString, bool isGist, QModelIndex parentIndex = QModelIndex());
    QModelIndex rootModelIndex();
    QModelIndex find(const QString& identString, QModelIndex parent);

public slots:
    void entryNameChanged(const QString &identString, const QString &newName);

private:
    TreeEntry* entry(const QModelIndex& index) const;
    void changeName(QModelIndex index, QString newName);

private:
    TreeEntry* mRoot;
};

} // namespace ide
} // namespace gams

#endif // TREEMODEL_H
