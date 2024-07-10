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
#ifndef GAMS_STUDIO_NUMERICS_DOUBLEFORMATTER_H
#define GAMS_STUDIO_NUMERICS_DOUBLEFORMATTER_H

#include <QString>

namespace gams {
namespace studio {
namespace numerics {

class DoubleFormatter
{
public:
    enum Format {
        g = 0,
        f = 1,
        e = 2
    };
    static const int gFormatFull;
    static QString format(double v, Format format, int precision, int squeeze, QChar decSep = '.');

private:
    DoubleFormatter() {};
};

} // namespace numerics
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NUMERICS_DOUBLEFORMATTER_H
