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
#ifndef EDITORHELPER_H
#define EDITORHELPER_H

#include <QString>

namespace gams {
namespace studio {

class EditorHelper
{
public:
    static void nextWord(int offset, int &pos, const QString &text);
    static void prevWord(int offset, int &pos, const QString &text);
    static QString formatTime(qreal timeSec, bool compact = false);
    static QString formatTime2(qreal timeSec, int &unitIndex, bool compact = false);
    static QString timeUnit(qreal timeSec);
    static QString formatMemory(size_t bytes, bool compact = false);
    static QString formatMemory2(size_t bytes, int &unitIndex, bool compact = false);
    static QString memoryUnit(size_t bytes);
    static QString formatSteps(size_t steps, int digits);
};

}
}

#endif // EDITORHELPER_H
