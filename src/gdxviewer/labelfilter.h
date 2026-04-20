/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#ifndef LABELFILTER_H
#define LABELFILTER_H

#include <QObject>

namespace gams {
namespace studio {
namespace gdxviewer {

class LabelFilter
{
public:
    LabelFilter(std::unique_ptr<bool []> showUelInColumn);

    bool isActive() const;
    void setActive(bool newActive);

    inline bool isUelShown(size_t uel) const
    {
        return mShowUel[uel];
    }

    inline void setUelShown(size_t uel, bool shown)
    {
        mShowUel[uel] = shown;
    }

private:
    bool active = false;
    std::unique_ptr<bool []> mShowUel;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // LABELFILTER_H
