#ifndef GDXSYMBOL_H
#define GDXSYMBOL_H

#include <QString>
#include "gdxcc.h"

namespace gams {
namespace studio {
namespace gdxviewer {

class GDXSymbol
{
public:
    GDXSymbol(int nr, QString name, int dimension, int type, int subType, int recordCount, QString explText, gdxHandle_t gdx);
    ~GDXSymbol();

    int nr() const;
    QString name() const;
    int dim() const;
    int type() const;
    int subType() const;
    int recordCount() const;
    QString explText() const;
    void loadData();

    int key(int rowIdx, int colIdx) const;
    double value(int rowIdx, int colIdx) const;

private:
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
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GDXSYMBOL_H
