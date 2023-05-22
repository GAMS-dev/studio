#ifndef GAMS_STUDIO_GDXVIEWER_NUMERICALFORMATCONTROLLER_H
#define GAMS_STUDIO_GDXVIEWER_NUMERICALFORMATCONTROLLER_H

#include <QCheckBox>
#include <QComboBox>
#include <QObject>
#include <QSpinBox>

namespace gams {
namespace studio {
namespace gdxviewer {

class NumericalFormatController
{
public:
    NumericalFormatController();
    static void initFormatComboBox(QComboBox* cb);
    static void initPrecisionSpinBox(QSpinBox *sb);

public:
    static bool update(QComboBox* cbFormat, QSpinBox *sbPrecision, QCheckBox *cbSqZeroes, bool restoreSqZeroes);
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_NUMERICALFORMATCONTROLLER_H
