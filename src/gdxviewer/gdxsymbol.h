#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H

#include <QAbstractTableModel>
#include <QMutex>
#include "gdxsymbol.h"
#include <memory>
#include "gdxcc.h"
#include <QString>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbol : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GdxSymbol(gdxHandle_t gdx, QMutex* gdxMutex, QStringList* uel2Label, QStringList* strPool, int nr, QString name, int dimension, int type, int subtype, int recordCount, QString explText, int* sortIndex, QObject *parent = 0);
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

private:
    int mNr;
    QString mName;
    int mDim;
    int mType;
    int mSubType;
    int mRecordCount;
    QString mExplText;

    gdxHandle_t mGdx;

    bool mIsLoaded = false;
    int mLoadedRecCount = 0;

    bool stopLoading = false;

    int* mKeys = nullptr;
    double* mValues = nullptr;

    QStringList* mUel2Label;
    QStringList* mStrPool;

    QMutex* mGdxMutex;

    QStringList mDomains;

    bool mSqueezeDefaults = false;

    bool mDefaultColumn[GMS_VAL_MAX] {false};

    void calcDefaultColumns();

    int* mSortMap = nullptr;
    int* mLabelCompIdx;

};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
