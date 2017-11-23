#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H

#include <QAbstractTableModel>
#include <QMutex>
#include "gdxsymboltable.h"
#include <memory>
#include "gdxcc.h"
#include <QString>
#include <QSet>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolTable;


class GdxSymbol : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GdxSymbol(gdxHandle_t gdx, QMutex* gdxMutex, int nr, QString name, int dimension, int type, int subtype, int recordCount, QString explText, GdxSymbolTable* gdxSymbolTable, QObject *parent = 0);
    ~GdxSymbol();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int nr() const;

    QString name() const;

    int dim() const;

    int type() const;

    int recordCount() const;

    QString explText() const;

    bool isLoaded() const;
    void loadData();
    void stopLoadingData();

    bool squeezeDefaults() const;
    void setSqueezeDefaults(bool squeezeDefaults);

    bool isAllDefault(int valColIdx);

    int subType() const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    void filterRows();
    int sortColumn() const;

    Qt::SortOrder sortOrder() const;

    void resetSorting();

    QList<QMap<int, bool> *> filterUels() const;

    GdxSymbolTable *gdxSymbolTable() const;

private:
    int mNr;
    QString mName;
    int mDim;
    int mType;
    int mSubType;
    int mRecordCount;
    QString mExplText;

    GdxSymbolTable* mGdxSymbolTable;

    gdxHandle_t mGdx;

    bool mIsLoaded = false;
    int mLoadedRecCount = 0;
    int mFilterRecCount = 0;

    bool stopLoading = false;

    int* mKeys = nullptr;
    double* mValues = nullptr;

    QMutex* mGdxMutex;

    QStringList mDomains;

    bool mSqueezeDefaults = false;

    bool mDefaultColumn[GMS_VAL_MAX] {false};

    void calcDefaultColumns();
    void calcUelsInColumn();

    QList<QMap<int, bool>*> mFilterUels;

    int* mRecSortIdx = nullptr;
    int* mRecFilterIdx = nullptr;

    int mSortColumn = -1;
    Qt::SortOrder mSortOrder;


};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
