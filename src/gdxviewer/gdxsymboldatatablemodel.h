#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H

#include <QAbstractTableModel>
#include "gdxsymbol.h"
#include <memory>

namespace gams {
namespace studio {
namespace gdxviewer {

class GDXSymbolDataTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GDXSymbolDataTableModel(std::shared_ptr<GDXSymbol> gdxSymbol, QStringList* uel2Label, QObject *parent = 0);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    std::shared_ptr<GDXSymbol> mGdxSymbol; //TODO(CW): use shared pointer?
    QStringList* mUel2Label;

};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
