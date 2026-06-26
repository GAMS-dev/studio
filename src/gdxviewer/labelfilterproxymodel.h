#ifndef LABELFILTERPROXYMODEL_H
#define LABELFILTERPROXYMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>

namespace gams {
namespace studio {
namespace gdxviewer {

class LabelFilterProxyModel : public QSortFilterProxyModel
{
public:
    explicit LabelFilterProxyModel(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // LABELFILTERPROXYMODEL_H
