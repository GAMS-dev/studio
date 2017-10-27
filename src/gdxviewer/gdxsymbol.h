#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H

#include <QAbstractTableModel>
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
    explicit GdxSymbol(gdxHandle_t gdx, QStringList* uel2Label, QStringList* strPool, int nr, QString name, int dimension, int type, int subtype, int recordCount, QString explText, QObject *parent = 0);
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

private:
    void loadData();

    int mNr;
    QString mName;
    int mDim;
    int mType;
    int mSubType;
    int mRecordCount;
    QString mExplText;

    gdxHandle_t mGdx;

    int* mKeys;
    double* mValues;

    QStringList* mUel2Label;
    QStringList* mStrPool;

};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLDATATABLEMODEL_H
