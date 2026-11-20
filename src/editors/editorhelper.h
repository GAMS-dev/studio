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
#ifndef EDITORHELPER_H
#define EDITORHELPER_H

#include <QString>
#include <QStringView>
#include <QList>
#include <QPoint>

namespace gams {
namespace studio {

class EditorHelper
{
public:
    // --- TextView: horizontal scrolling by cutting the lines (monospace font) and smooth wheel scrolling

    /// \brief Splits text into lines at '\n' and removes a trailing '\r' from each line.
    /// \param lines receives the lines, referencing the data of text.
    /// \param lineLengths if set, receives the length of each line.
    /// \return the length of the longest line.
    static int splitLines(QStringView text, QList<QStringView> &lines, QList<int> *lineLengths = nullptr);

    /// \brief Cuts the part starting at fromColumn with at most columnCount characters out of each line.
    /// \return the parts joined by '\n'. A line ending left of fromColumn results in an empty line.
    static QString cutLines(const QList<QStringView> &lines, int fromColumn, int columnCount);

    /// \brief Converts a full-line column to the column in a document line that starts at docColumn.
    /// \return the document column, limited to [0, docLineLength].
    static int toDocColumn(int column, int docColumn, int docLineLength);

    /// \brief Checks if the visible columns are contained in the document.
    /// \param columnOffset the first visible column.
    /// \param visibleColumns the number of visible columns.
    /// \param docColumn the first column of the document.
    /// \param docColumnCount the number of columns of the document.
    /// \return true if the document covers all visible columns, otherwise it needs to be rebuilt.
    static bool isInColumnBuffer(int columnOffset, int visibleColumns, int docColumn, int docColumnCount);

    /// \brief Scrolls a position given in units (like columns or lines) by pixels.
    /// \param position the current position in units.
    /// \param remain the pixels scrolled beyond the position (partial unit), updated to the new value.
    /// \param dPixels the pixels to scroll.
    /// \param unitSize the size of a unit in pixels.
    /// \param maxPosition the maximum position.
    /// \return the new position, limited to [0, maxPosition]. When limited, remain is set to 0.
    static int scrollByPixels(int position, qreal &remain, qreal dPixels, qreal unitSize, int maxPosition);

    /// \brief Determines the axis a wheel event scrolls. A touchpad gesture (including its momentum) keeps the axis
    /// it started with to avoid jitter by the movement on the cross axis. A mouse wheel decides on each event.
    /// \param lockedAxis the state between the events of a gesture, start with 0.
    /// \param phase the scroll phase of the wheel event.
    /// \param pixels the scroll delta of the wheel event.
    /// \return Qt::Horizontal, Qt::Vertical, or 0 if not decided yet.
    static int wheelAxis(int &lockedAxis, Qt::ScrollPhase phase, const QPoint &pixels);

    // --- Word navigation

    /// \brief Moves pos to the start of the next word in text.
    /// \param offset the offset added to pos to get the index in text.
    /// \param pos the position, updated to the start of the next word. It is incremented by one for an empty text or
    /// at the end of text (e.g. to continue in the next line).
    static void nextWord(int offset, int &pos, const QString &text);

    /// \brief Moves pos to the start of the previous word in text.
    /// \param offset the offset added to pos to get the index in text.
    /// \param pos the position, updated to the start of the previous word. It is decremented by one if it is behind
    /// the end of text.
    static void prevWord(int offset, int &pos, const QString &text);

    // --- Formatting of time, memory, and steps (e.g. for the profiler)

    /// \brief Formats a time including the unit (s, m, h). The precision depends on the size of the time.
    /// \param compact if true, uses a short format (e.g. "1:05 m"), otherwise includes milliseconds (e.g. "1:05.123 m").
    /// \return the formatted time.
    static QString formatTime(qreal timeSec, bool compact = false);

    /// \brief Formats a time without the unit, to show several times in the same unit.
    /// \param unitIndex the unit: 0 = seconds, 1 = minutes, 2 = hours, 3 = days. If negative, it is set to the unit
    /// matching timeSec.
    /// \param compact if true, uses a short format, otherwise includes milliseconds.
    /// \return the formatted time, or "wrong unit" for an invalid unitIndex.
    static QString formatTime2(qreal timeSec, int &unitIndex, bool compact = false);

    /// \brief Determines the unit matching the time.
    /// \return "sec", "min", "hour", or "days".
    static QString timeUnit(qreal timeSec);

    /// \brief Formats a memory size including the unit (B, KB, MB, GB, TB), right justified.
    /// \param compact if true, uses up to two decimals and a width of 8, otherwise three decimals and a width of 10.
    /// \return the formatted memory size.
    static QString formatMemory(size_t bytes, bool compact = false);

    /// \brief Formats a memory size without the unit, to show several sizes in the same unit.
    /// \param unitIndex the unit: 0 = B, 1 = KB, 2 = MB, 3 = GB, 4 = TB. If negative, it is set to the unit matching
    /// bytes.
    /// \param compact if true, limits the number to five characters.
    /// \return the formatted memory size, or an empty string for an invalid unitIndex.
    static QString formatMemory2(size_t bytes, int &unitIndex, bool compact = false);

    /// \brief Determines the unit matching the memory size.
    /// \return "B", "KB", "MB", "GB", or "TB".
    static QString memoryUnit(size_t bytes);

    /// \brief Formats a number of steps, right justified.
    /// \param digits the width is digits + 2.
    /// \return the formatted number of steps.
    static QString formatSteps(size_t steps, int digits);
};

}
}

#endif // EDITORHELPER_H
