#ifndef GAMS_STUDIO_GDXVIEWER_TABLEVIEWDOMAINMODEL_H
#define GAMS_STUDIO_GDXVIEWER_TABLEVIEWDOMAINMODEL_H

#include "tableviewmodel.h"

#include <QAbstractTableModel>

namespace gams {
namespace studio {
namespace gdxviewer {

class TableViewDomainModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TableViewDomainModel(TableViewModel* tvModel, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    TableViewModel *tvModel() const;



private:
    TableViewModel* mTvModel;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_TABLEVIEWDOMAINMODEL_H
