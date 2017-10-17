#ifndef GDXSYMBOL_H
#define GDXSYMBOL_H

#include <QString>

namespace gams {
namespace studio {
namespace gdxviewer {

class GDXSymbol
{
public:
    GDXSymbol(int nr, QString name, int dimension, int type, int subType, int recordCount, QString explText);

    int nr() const;
    QString name() const;
    int dim() const;
    int type() const;
    int subType() const;
    int recordCount() const;
    QString explText() const;

private:
    int mNr;
    QString mName;
    int mDim;
    int mType;
    int mSubType;
    int mRecordCount;
    QString mExplText;

};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GDXSYMBOL_H
