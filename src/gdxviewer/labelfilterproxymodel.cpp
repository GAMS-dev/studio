#include "labelfilterproxymodel.h"

namespace gams {
namespace studio {
namespace gdxviewer {

LabelFilterProxyModel::LabelFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{}

bool LabelFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    int leftCheckState = sourceModel()->data(source_left, Qt::CheckStateRole).toInt();
    int rightCheckState = sourceModel()->data(source_right, Qt::CheckStateRole).toInt();

    if (leftCheckState != rightCheckState)
        return leftCheckState < rightCheckState;
    return source_left.row() > source_right.row();
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
