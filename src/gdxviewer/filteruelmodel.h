#ifndef GAMS_STUDIO_GDXVIEWER_FILTERUELMODEL_H
#define GAMS_STUDIO_GDXVIEWER_FILTERUELMODEL_H

#include <QAbstractListModel>
#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class FilterUelModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FilterUelModel(GdxSymbol* symbol, int column, QObject *parent = 0);
    ~FilterUelModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    int *uels() const;

    bool *checked() const;

private:
    GdxSymbol* mSymbol;
    int* mUels;
    bool* mChecked;
    int mColumn;
    QMap<int, bool>* mfilterUels;

};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_FILTERUELMODEL_H
