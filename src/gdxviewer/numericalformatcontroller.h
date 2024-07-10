/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
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
    static void initFormatComboBox(QComboBox* cb);
    static void initPrecisionSpinBox(QSpinBox *sb);

public:
    static bool update(QComboBox* cbFormat, QSpinBox *sbPrecision, QCheckBox *cbSqZeroes, bool restoreSqZeroes);
private:
    NumericalFormatController();
    static const QString svFull;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_NUMERICALFORMATCONTROLLER_H
