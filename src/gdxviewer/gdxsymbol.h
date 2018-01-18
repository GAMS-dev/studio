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
    explicit GdxSymbol(gdxHandle_t gdx, QMutex* gdxMutex, int nr, GdxSymbolTable* gdxSymbolTable, QObject *parent = 0);
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

    bool isAllDefault(int valColIdx);

    int subType() const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void filterRows();
    int sortColumn() const;

    Qt::SortOrder sortOrder() const;

    void resetSortFilter();

    GdxSymbolTable *gdxSymbolTable() const;

    QVector<QVector<int> *> uelsInColumn() const;

    QVector<bool *> showUelInColumn() const;

    void setShowUelInColumn(const QVector<bool *> &showUelInColumn);

    QVector<bool> filterActive() const;
    void setFilterActive(const QVector<bool> &filterActive);

signals:
    void loadFinished();

private:
    gdxHandle_t mGdx;
    int mNr;
    QMutex* mGdxMutex;
    int mDim;
    int mType;
    int mSubType;
    int mRecordCount;
    QString mExplText;
    QString mName;

    QVector<int> mMinUel;
    QVector<int> mMaxUel;

    GdxSymbolTable* mGdxSymbolTable;



    bool mIsLoaded = false;
    int mLoadedRecCount = 0;
    int mFilterRecCount = 0;

    bool stopLoading = false;

    QVector<int> mKeys;
    QVector<double> mValues;

    QStringList mDomains;

    bool mDefaultColumn[GMS_VAL_MAX] {false};

    void calcDefaultColumns();
    void calcUelsInColumn();
    void loadMetaData();
    void loadDomains();

    QVector<QVector<int>*> mUelsInColumn;
    QVector<bool*> mShowUelInColumn;
    QVector<bool> mFilterActive;

    QVector<int> mRecSortIdx;
    QVector<int> mRecFilterIdx;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
