/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "editorhelper.h"

namespace gams {
namespace studio {

void EditorHelper::nextWord(int offset,int &pos, const QString &text)
{
    if (text.length() == 0) {
        // short line
        pos++;
        return;
    } else if (pos+offset < text.length()) {
        // navigating in a line
        if (pos + offset > text.length()) return;
        bool stop = false;

        while (++pos + offset < text.length()) {
            // stop if last char is not letter or number
            stop = (pos + offset > 0) && !text.at(pos + offset - 1).isLetterOrNumber();
            if (text.at(pos + offset).isLetterOrNumber() && stop) return;
        }
    } else {
        // reached end of line
        pos++;
    }
}

void EditorHelper::prevWord(int offset, int &pos, const QString &text)
{
    if (pos + offset > text.length()) {
        pos--;
        return;
    }
    if (pos + offset > 0) {
        --pos;
        if (pos+offset == 0) return;
        bool last = false;

        while (--pos + offset > 0) {
            last = !text.at(pos + offset + 1).isLetterOrNumber();
            if (!text.at(pos+offset).isLetterOrNumber() && !last) {
                ++pos;
                return;
            }
        }
    }
}

}
}
