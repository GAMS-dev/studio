/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
 */
#include "breakpointdata.h"

namespace gams {
namespace studio {
namespace debugger {

BreakpointData::BreakpointData()
{ }

BreakpointData::~BreakpointData()
{ }

void BreakpointData::clearLinesMap()
{
    mLastCln4File.clear();
    mCln2Line.clear();
    mFileLine2Cln.clear();
}

bool BreakpointData::addLinesMap(const QString &filename, const QList<int> &fileLines, const QList<int> &contLines)
{
    if (fileLines.isEmpty() || fileLines.size() != contLines.size())
        return false;
    if (mLastCln4File.isEmpty() || mLastCln4File.last() != filename)
        mLastCln4File.insert(contLines.last(), filename);

    QMap<int, int> revMap = mFileLine2Cln.value(filename);
    for (int i = 0; i < fileLines.size(); ++i) {
        mCln2Line.insert(contLines.at(i), fileLines.at(i));
        revMap.insert(fileLines.at(i), contLines.at(i));
    }
    mFileLine2Cln.insert(filename, revMap);
    return true;
}

bool BreakpointData::hasLinesMap()
{
    return !mFileLine2Cln.isEmpty();
}

int BreakpointData::continuousLine(const QString &filename, int fileLine) const
{
    return mFileLine2Cln.value(filename).value(fileLine, -1);
}

QString BreakpointData::filename(int contLine) const
{
    if (contLine < 0 || mLastCln4File.isEmpty()) return QString();
    const auto iter = mLastCln4File.lowerBound(contLine);
    if (iter == mLastCln4File.constEnd())
        return mLastCln4File.last();
    return iter.value();
}

int BreakpointData::fileLine(int contLine) const
{
    return mCln2Line.value(contLine, -1);
}

QStringList BreakpointData::bpFiles()
{
    return mActiveBp.keys();
}

void BreakpointData::adjustBreakpoints()
{
    QMap<QString, SortedSet> newBp;
    for (auto iFile = mActiveBp.constBegin() ; iFile != mActiveBp.constEnd() ; ++iFile) {
        SortedSet bps;
        for (auto iLine = iFile.value().constBegin() ; iLine != iFile.value().constEnd() ; ++iLine) {
            int line = iLine.key();
            adjustBreakpoint(iFile.key(), line, false);
            bps.insert(line, 0);
        }
        newBp.insert(iFile.key(), bps);
    }
    mActiveBp = newBp;
}

void BreakpointData::adjustBreakpoint(const QString &filename, int &fileLine, bool skipExist)
{
    if (skipExist && mActiveBp.value(filename).contains(fileLine))
        return;

    const QMap<int, int> map = mFileLine2Cln.value(filename);
    if (map.isEmpty()) return;

    const auto iter = map.lowerBound(fileLine);
    fileLine = (iter == map.constEnd()) ? map.lastKey() : fileLine = iter.key();
}

int BreakpointData::addBreakpoint(const QString &filename, int fileLine, bool isRunning)
{
    const QMap<int, int> map = mFileLine2Cln.value(filename);
    int resLine = fileLine;
    if (!map.isEmpty() && isRunning) {
        const auto iter = map.lowerBound(fileLine);
        resLine = (iter == map.constEnd()) ? map.lastKey() : iter.key();
    }
    SortedSet lines = mActiveBp.value(filename);
    lines.insert(resLine, 0);
    mActiveBp.insert(filename, lines);
    return resLine;
}

void BreakpointData::delBreakpoint(const QString &filename, int fileLine)
{
    SortedSet lines = mActiveBp.value(filename);
    lines.remove(fileLine);
    if (lines.isEmpty())
        mActiveBp.remove(filename);
    else
        mActiveBp.insert(filename, lines);
}

void BreakpointData::delBreakpoints()
{
    mActiveBp.clear();
}

bool BreakpointData::isBreakpoint(const QString &filename, int fileLine) const
{
    return mActiveBp.value(filename).contains(fileLine);
}

QList<int> BreakpointData::bpFileLines(const QString &filename) const
{
    return mActiveBp.value(filename).keys();
}

QList<int> BreakpointData::bpContinuousLines() const
{
    QList<int> res;
    auto itFile = mActiveBp.constBegin();
    while (itFile != mActiveBp.constEnd()) {
        auto itLine = itFile.value().constBegin();
        while (itLine != itFile.value().constEnd()) {
            int contLine = continuousLine(itFile.key(), itLine.key());
            if (contLine >= 0)
                res << contLine;
            ++itLine;
        }
        ++itFile;
    }
    return res;
}

} // namespace debugger
} // namespace studio
} // namespace gams
