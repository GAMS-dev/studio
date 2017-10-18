#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H

#include <QAbstractItemModel>
#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GdxSymbolTableModel(QList<GDXSymbol> data, QObject *parent = 0);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QStringList mHeaderText;
    QList<GDXSymbol> mData;

};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLTABLEMODEL_H
