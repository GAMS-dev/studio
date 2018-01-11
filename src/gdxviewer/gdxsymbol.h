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

    bool *filterActive() const;

private:
    int mNr;
    int mDim;
    int mType;
    int mSubType;
    int mRecordCount;
    QString mExplText;
    QString mName;

    int* mMinUel = nullptr;
    int* mMaxUel = nullptr;

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

    bool mDefaultColumn[GMS_VAL_MAX] {false};

    void calcDefaultColumns();
    void calcUelsInColumn();
    void loadMetaData();
    void loadDomains();

    QVector<QVector<int>*> mUelsInColumn;
    QVector<bool*> mShowUelInColumn;
    bool* mFilterActive = nullptr;

    int* mRecSortIdx = nullptr;
    int* mRecFilterIdx = nullptr;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
