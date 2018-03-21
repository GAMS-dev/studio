#ifndef GAMS_STUDIO_LXIVIEWER_LXITREEMODEL_H
#define GAMS_STUDIO_LXIVIEWER_LXITREEMODEL_H

#include <QAbstractItemModel>
#include <QVector>
#include "lxitreeitem.h"

namespace gams {
namespace studio {
namespace lxiviewer {

class LxiTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit LxiTreeModel(LxiTreeItem *root, QVector<int> lineNrs, QVector<LxiTreeItem*> treeItems, QObject *parent = nullptr);
    ~LxiTreeModel();

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVector<int> lineNrs() const;

    QVector<LxiTreeItem *> treeItems() const;

private:
    LxiTreeItem* mRootItem = nullptr;

    QVector<int> mLineNrs;
    QVector<LxiTreeItem*> mTreeItems;

};

} // namespace lxiviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_LXIVIEWER_LXITREEMODEL_H
