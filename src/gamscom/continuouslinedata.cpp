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
#include "continuouslinedata.h"

namespace gams {
namespace studio {
namespace gamscom {

ContinuousLineData::ContinuousLineData()
{ }

ContinuousLineData::~ContinuousLineData()
{ }

void ContinuousLineData::clearLinesMap()
{
    mLastCln4File.clear();
    mCln2Line.clear();
    mFileLine2Cln.clear();
}

bool ContinuousLineData::addLinesMap(const QString &filename, const QList<int> &fileLines, const QList<int> &contLines)
{
    if (fileLines.isEmpty() || fileLines.size() != contLines.size())
        return false;
    if (!mLastCln4File.isEmpty() && mLastCln4File.last() == filename)
        mLastCln4File.remove(mLastCln4File.lastKey());
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

bool ContinuousLineData::hasLinesMap()
{
    return !mFileLine2Cln.isEmpty();
}

int ContinuousLineData::continuousLine(const QString &filename, int fileLine) const
{
    return mFileLine2Cln.value(filename).value(fileLine, -1);
}

QString ContinuousLineData::filename(int contLine) const
{
    if (contLine < 0 || mLastCln4File.isEmpty()) return QString();
    const auto iter = mLastCln4File.lowerBound(contLine);
    if (iter == mLastCln4File.constEnd())
        return mLastCln4File.last();
    return iter.value();
}

int ContinuousLineData::fileLine(int contLine) const
{
    return mCln2Line.value(contLine, -1);
}

QStringList ContinuousLineData::bpFiles()
{
    return mActiveBp.keys();
}

void ContinuousLineData::adjustBreakpoints()
{
    QMap<QString, SortedIntMap> newBp;
    QMap<QString, SortedIntMap> newAimBp;
    for (auto iFile = mActiveBp.constBegin() ; iFile != mActiveBp.constEnd() ; ++iFile) {
        SortedIntMap bps;
        SortedIntMap aimBps;
        for (auto iLine = iFile.value().constBegin() ; iLine != iFile.value().constEnd() ; ++iLine) {
            int line = iLine.key();
            adjustBreakpoint(iFile.key(), line);
            bps.insert(line, line);
            aimBps.insert(iLine.key(), line);
        }
        newBp.insert(iFile.key(), bps);
        newAimBp.insert(iFile.key(), aimBps);
    }
    mActiveBp = newBp;
    mAimedBp = newAimBp;
}

void ContinuousLineData::adjustBreakpoint(const QString &filename, int &fileLine)
{
    const QMap<int, int> map = mFileLine2Cln.value(filename);
    if (map.isEmpty()) {
        fileLine = -1;
        return;
    }

    const auto iter = map.lowerBound(fileLine);
    fileLine = (iter == map.constEnd()) ? map.lastKey() : iter.key();
}

int ContinuousLineData::addBreakpoint(const QString &filename, int fileLine, bool isRunning)
{
    const QMap<int, int> map = mFileLine2Cln.value(filename);
    int resLine = fileLine;
    if (!map.isEmpty() && isRunning) {
        const auto iter = map.lowerBound(fileLine);
        resLine = (iter == map.constEnd()) ? map.lastKey() : iter.key();
    }
    SortedIntMap lines = mActiveBp.value(filename);
    lines.insert(resLine, resLine);
    mActiveBp.insert(filename, lines);

    lines = mAimedBp.value(filename);
    lines.insert(fileLine, resLine);
    mAimedBp.insert(filename, lines);
    return resLine;
}

void ContinuousLineData::delBreakpoint(const QString &filename, int fileLine)
{
    SortedIntMap lines = mActiveBp.value(filename);
    lines.remove(fileLine);
    if (lines.isEmpty())
        mActiveBp.remove(filename);
    else
        mActiveBp.insert(filename, lines);

    lines = mAimedBp.value(filename);
    for (auto iter = lines.begin(); iter != lines.end(); ) {
        if (iter.key() == fileLine || iter.value() == fileLine)
            iter = lines.erase(SortedIntMap::const_iterator(iter)); // clazy complained about mixed (const)iterators
        else
            ++iter;
    }
    if (lines.isEmpty())
        mAimedBp.remove(filename);
    else
        mAimedBp.insert(filename, lines);
}

void ContinuousLineData::delBreakpoints()
{
    mActiveBp.clear();
    mAimedBp.clear();
}

void ContinuousLineData::resetAimedBreakpoints()
{
    mAimedBp = mActiveBp;
}

SortedIntMap ContinuousLineData::bpFileLines(const QString &filename) const
{
    return mActiveBp.value(filename);
}

SortedIntMap ContinuousLineData::bpAimedFileLines(const QString &filename) const
{
    return mAimedBp.value(filename);
}

QList<int> ContinuousLineData::bpContinuousLines() const
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
