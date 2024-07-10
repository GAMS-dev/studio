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
#include "doubleformatter.h"
#include "doubleFormat.h"

namespace gams {
namespace studio {
namespace numerics {

const int DoubleFormatter::gFormatFull = 0;

QString DoubleFormatter::format(double v, DoubleFormatter::Format format, int precision, int squeeze, QChar decSep)
{
    char outBuf[32];
    int outLen;
    char* p = nullptr;
    if (format == Format::g)
        p = x2gfmt(v, precision, squeeze, outBuf, &outLen, decSep.toLatin1());
    else if (format == Format::f)
        p = x2fixed(v, precision, squeeze, outBuf, &outLen, decSep.toLatin1());
    else if (format == Format::e)
        p = x2efmt(v, precision, squeeze, outBuf, &outLen, decSep.toLatin1());
    if (!p)
        return "FORMAT_ERROR";
    else
        return QString(p);
}

} // namespace numerics
} // namespace studio
} // namespace gams
