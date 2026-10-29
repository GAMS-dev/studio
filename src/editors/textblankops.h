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
#pragma once

#include <QFlags>
#include <QTextBlock>
#include <QTextCursor>
#include <functional>

namespace gams {
namespace studio {

class TextBlankOps
{
public:
    enum Flag {
        None         = 0x0,
        TrimLeading  = 0x1,
        TrimTrailing = 0x2,
        Trim         = TrimLeading | TrimTrailing,
        EolToSpace   = 0x4,
        TabToSpace   = 0x8,
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    explicit TextBlankOps(const QTextCursor &cursor, int tabWidth = 4);

    void apply(Flags flags);

private:
    void trim(bool leading, bool trailing);
    void eolToSpace();
    void tabToSpace();
    void forEachLine(const std::function<void(QTextCursor &, const QTextBlock &)> &fn);

    QTextCursor mCursor;
    int mTabWidth;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TextBlankOps::Flags)
}
}
