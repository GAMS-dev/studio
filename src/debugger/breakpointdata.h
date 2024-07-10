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
#ifndef GAMS_STUDIO_DEBUGGER_BREAKPOINTDATA_H
#define GAMS_STUDIO_DEBUGGER_BREAKPOINTDATA_H

#include "common.h"

#include <QList>
#include <QMap>
#include <QList>
#include <QPair>

namespace gams {
namespace studio {
namespace debugger {

class BreakpointData
{
public:
    explicit BreakpointData();
    virtual ~BreakpointData();

    void clearLinesMap();
    bool addLinesMap(const QString &filename, const QList<int> &fileLines, const QList<int> &contLines);

    bool hasLinesMap();
    int continuousLine(const QString &filename, int fileLine) const;
    QString filename(int contLine) const;
    int fileLine(int contLine) const;

    void adjustBreakpoints();
    int addBreakpoint(const QString &filename, int fileLine, bool isRunning = false);
    void delBreakpoint(const QString &filename, int fileLine);
    void delBreakpoints();
    void resetAimedBreakpoints();
    QStringList bpFiles();
    SortedIntMap bpFileLines(const QString &filename) const;
    SortedIntMap bpAimedFileLines(const QString &filename) const;
    QList<int> bpContinuousLines() const;

private:
    void adjustBreakpoint(const QString &filename, int &fileLine);

private:
    QStringList mFiles;
    QMap<int, QString> mLastCln4File;
    QMap<int, int> mCln2Line;
    QMap<QString, QMap<int, int> > mFileLine2Cln;

    QMap<QString, SortedIntMap> mActiveBp;
    QMap<QString, SortedIntMap> mAimedBp;
};

} // namespace debugger
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_DEBUGGER_BREAKPOINTDATA_H
