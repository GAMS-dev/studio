#ifndef GAMS_STUDIO_GDXVIEWER_TABLEVIEWMODEL_H
#define GAMS_STUDIO_GDXVIEWER_TABLEVIEWMODEL_H

#include <QAbstractTableModel>
#include "gdxsymbol.h"
#include "gdxsymboltable.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class TableViewModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TableViewModel(GdxSymbol* sym, GdxSymbolTable* gdxSymbolTable, QObject *parent = nullptr);

    ~TableViewModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVector<bool> defaultColumnTableView() const;

    QVector<int> tvDimOrder() const;

    QVector<int> *getTvSectionWidth() const;

    int dim();

    int tvColDim() const;

    int type();

    void setTableView(bool tableView, int colDim = -1, QVector<int> tvDims = QVector<int>());

    bool isAllDefault(int valColIdx);

    bool needDummyRow() const;

    bool needDummyColumn() const;

    void reset();

private:
    void calcDefaultColumnsTableView();
    void initTableView(int nrColDim, QVector<int> dimOrder);

    GdxSymbol* mSym;
    GdxSymbolTable* mGdxSymbolTable;

    int mTvColDim;
    QVector<int> mTvDimOrder;
    QVector<QVector<uint>> mTvRowHeaders;
    QVector<QVector<uint>> mTvColHeaders;
    QHash<QVector<uint>, int> mTvKeysToValIdx;

    QVector<bool> mDefaultColumnTableView;

    QVector<int>* tvSectionWidth = nullptr;
    QMap<QString, int>* tvLabelWidth = nullptr;

    bool mNeedDummyRow = false;
    bool mNeedDummyColumn = false;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_TABLEVIEWMODEL_H
