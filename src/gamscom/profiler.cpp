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
#include "profiler.h"
#include "continuouslinedata.h"
#include "editors/editorhelper.h"

namespace gams {
namespace studio {
namespace gamscom {

const QStringList Profiler::CTimeUnits = {"sec","mm:ss","hh:mm","day"};
const QStringList Profiler::CMemoryUnits  = {"byte","KB","MB","GB","TB"};
const QStringList Profiler::CAggStatements = {"for", "loop", "repeat", "until", "while"};
const int CMaxEntries = 10;


Profiler::Profiler()
{}

Profiler::~Profiler()
{
    clear();
}

void Profiler::clear()
{
    mMainFile.clear();
    mProfileData.clear();
    mFileLine2Cln.clear();
    mMaxSingleSteps.clear();
    mMaxCompountTime = 0.;
    mMaxMemory = 0ull;
    mMaxRows = 0ull;
    mCurrentTimeUnit = 0;
    mCurrentMemoryUnit = 0;
    mSumTimeInSec = 0.;
    mSumRows = 0ull;
    mSumSteps = 0ull;
}

bool Profiler::isEmpty()
{
    return mProfileData.isEmpty();
}

void Profiler::setContinuousLineData(ContinuousLineData *contLines)
{
    mContLines = contLines;
}


void Profiler::add(int contLine, const QString &statement, qreal timeSec, size_t memory, size_t rows)
{
    if (!mProfileData.contains(contLine)) {
        QMap<QString, ProfileData> map;
        mProfileData.insert(contLine, map);
    }
    QMap<QString, ProfileData> &map = mProfileData[contLine];

    // update time
    size_t lastMemDelta = 0;
    if (!map.contains(statement)) {
        map.insert(statement, ProfileData(statement, timeSec, memory, rows));
    } else {
        map[statement].add(timeSec, memory, rows);
    }
    lastMemDelta = map[statement].lastMemoryDelta;
    if (!map.contains("")) {
        map.insert("", ProfileData("", timeSec, memory, rows));
    } else {
        map[""].add(timeSec, lastMemDelta, rows);
    }

    // update max values
    if (!CAggStatements.contains(statement.trimmed(), Qt::CaseInsensitive)) {
        mSumTimeInSec += timeSec;
        mSumRows += rows;
        ++mSumSteps;
        const ProfileData &curPD = map.value("");
        for (int i = 0; i < mMaxSingleTime.size(); ++i)
            if (mMaxSingleTime.at(i).first == contLine) mMaxSingleTime.remove(i--);
        for (int i = 0; i < mMaxSingleSteps.size(); ++i)
            if (mMaxSingleSteps.at(i).first == contLine) mMaxSingleSteps.remove(i);
        int iTime = CMaxEntries;
        int iSteps = CMaxEntries;
        for (int i = 0; i < CMaxEntries; ++i) {
            if (i >= mMaxSingleTime.size()) {
                if (iTime == CMaxEntries) iTime = i;
            } else if (iTime == CMaxEntries && mMaxSingleTime.at(i).second.timeInSec < curPD.timeInSec)
                iTime = i;
            if (i >= mMaxSingleSteps.size()) {
                if (iSteps == CMaxEntries) iSteps = i;
            } else if (iSteps == CMaxEntries && mMaxSingleSteps.at(i).second.steps < curPD.steps)
                iSteps = i;
        }
        if (iTime < CMaxEntries) {
            mMaxSingleTime.insert(iTime, QPair<int, ProfileData>(contLine, curPD));
            if (mMaxSingleTime.size() > CMaxEntries) mMaxSingleTime.removeLast();
        }
        if (iSteps < CMaxEntries) {
            mMaxSingleSteps.insert(iSteps, QPair<int, ProfileData>(contLine, curPD));
            if (mMaxSingleSteps.size() > CMaxEntries) mMaxSingleSteps.removeLast();
        }
        if (curPD.timeInSecMax > mMaxCompountTime) mMaxCompountTime = curPD.timeInSecMax;
        if (curPD.steps > mMaxSteps) mMaxSteps = curPD.steps;
    }
    if (timeSec > mMaxCompountTime) mMaxCompountTime = timeSec;
    if (memory > mMaxMemory) mMaxMemory = memory;
    if (rows > mMaxRows) mMaxRows = rows;

    // Update includeLines
    QMap<QString, QMap<int, QPair<int, int>>>::ConstIterator fit = mFileLine2Cln.constBegin();
    while (fit != mFileLine2Cln.constEnd()) {
        QMap<int, QPair<int, int>>::ConstIterator cit = fit->constBegin();
        while (cit != fit->constEnd()) {
            if (contLine > cit->first && contLine < cit->second) {
                if (!mProfileData.contains(cit->first)) {
                    QMap<QString, ProfileData> map;
                    mProfileData.insert(cit->first, map);
                }
                QMap<QString, ProfileData> &map = mProfileData[cit->first];
                if (!map.contains("File SUM"))
                    map.insert("File SUM", ProfileData("", timeSec, memory, rows));
                else
                    map["File SUM"].add(timeSec, memory, rows);
                if (!map.contains(""))
                    map.insert("", ProfileData("", timeSec, memory, rows));
                else
                    map[""].add(timeSec, memory, rows);
                ProfileData &pd = map[""];
                if (pd.timeInSecMax > mMaxCompountTime) mMaxCompountTime = map[""].timeInSecMax;
                if (pd.memoryMax > mMaxMemory) mMaxMemory = map[""].memoryMax;
                if (pd.rowsMax > mMaxRows) mMaxRows = map[""].rowsMax;
                if (pd.steps > mMaxSteps) mMaxSteps = map[""].steps;
            }
            ++cit;
        }
        ++fit;
    }
}

void Profiler::getSums(qreal &timeSec, size_t &rows, size_t &steps)
{
    timeSec = mSumTimeInSec;
    rows = mSumRows;
    steps = mSumSteps;
}

int Profiler::continuousLine(QString filename, int localLine)
{
    int res = -1;
    if (mContLines) res = mContLines->continuousLine(filename, localLine);
    if (res < 0) res = mFileLine2Cln.value(filename).value(localLine, {-1,-1}).first;
    if (res < 0 && localLine == 1 && filename.compare(mMainFile) == 0) res = 1;
    return res;
}

void Profiler::getUnits(QStringList &timeUnits, QStringList &memUnits)
{
    timeUnits = CTimeUnits;
    memUnits  = CMemoryUnits;
}

void Profiler::getCurrentUnits(int &timeUnit, int &memUnit)
{
    timeUnit = mCurrentTimeUnit;
    memUnit = mCurrentMemoryUnit;
}

void Profiler::setCurrentUnits(int timeUnit, int memUnit)
{
    // index == -1 selects the optimal unit for the max-value
    if (timeUnit < 0)
        timeUnit = CTimeUnits.indexOf(EditorHelper::timeUnit(mMaxCompountTime), 0);
    mCurrentTimeUnit = qBound(0, timeUnit, CTimeUnits.size());
    if (memUnit < 0)
        memUnit = CMemoryUnits.indexOf(EditorHelper::memoryUnit(mMaxMemory), 0);
    mCurrentMemoryUnit = qBound(0, memUnit, CMemoryUnits.size());
}

void Profiler::getProfile(int contLine, qreal &timeSec, size_t &memory, size_t &rows, size_t &steps)
{
    if (mProfileData.contains(contLine)) {
        const QMap<QString, ProfileData> &map = mProfileData[contLine];
        if (map.contains("")) {
            const ProfileData &prf = map[""];
            timeSec = prf.timeInSec;
            memory = prf.memoryMax;
            rows = prf.rowsMax;
            steps = prf.steps;
        }
    }
}

void Profiler::getProfileText(int contLine, QStringList &profileData)
{
    if (mProfileData.contains(contLine)) {
        QMap<QString, ProfileData>::ConstIterator it = mProfileData[contLine].constBegin();
        QString sum;
        int rows = 0;
        while(it != mProfileData[contLine].constEnd()) {

            QString row;
            if (it.key().isEmpty())
                sum = QString("<tr><td align='right';>SUM: </td><td align='right';> %1 </td><td>(%2%) </td>"
                              "<td align='right';> %3 </td><td></td><td align='right';> %4 ↻</td><td>(%5%) </td></tr>")
                          .arg(EditorHelper::formatTime(it.value().timeInSec))
                          .arg(qreal(it.value().timeInSec)*100 / mSumTimeInSec,0,'f',1)
                          .arg(EditorHelper::formatMemory(it.value().memoryMax))
                          .arg(it.value().steps)
                          .arg(qreal(it.value().steps)*100 / qreal(mSumSteps),0,'f',1);
            else if (it.key().compare("File SUM") == 0) {
                row = QString("<tr><td align='right';>File SUM: </td><td align='right';> %1 </td><td>(%2%) </td>"
                              "<td align='right';> %3 </td><td></td><td align='right';> %4 ↻</td><td>(%5%) </td></tr>")
                          .arg(EditorHelper::formatTime(it.value().timeInSec))
                          .arg(qreal(it.value().timeInSec)*100 / mSumTimeInSec,0,'f',1)
                          .arg(EditorHelper::formatMemory(it.value().memoryMax))
                          .arg(it.value().steps)
                          .arg(qreal(it.value().steps)*100 / qreal(mSumSteps),0,'f',1);
                profileData.append(row);
            } else {
                QString sTime = it.value().timeInSecMin == it.value().timeInSecMax
                                    ? EditorHelper::formatTime(it.value().timeInSecMin)
                                    : QString("%1 - %2").arg(EditorHelper::formatTime(it.value().timeInSecMin),
                                                             EditorHelper::formatTime(it.value().timeInSecMax));
                QString sMem = it.value().memoryMin == it.value().memoryMax
                                    ? EditorHelper::formatMemory(it.value().memoryMin)
                                    : QString("%1 - %2").arg(EditorHelper::formatMemory(it.value().memoryMin),
                                                             EditorHelper::formatMemory(it.value().memoryMax));
                QString sRows;
                if (it.value().rowsMax > 0)
                    sRows = "[" + (it.value().rowsMin == it.value().rowsMax
                                    ? QString::number(it.value().rowsMin)
                                    : QString("%1 - %2").arg(it.value().rowsMin).arg(it.value().rowsMax)) + "]";

                row = QString("<tr><td>%1 </td><td align='right'> %2 </td><td align='right'>(%3%) </td>"
                              "<td align='right'> %4 </td><td align='right'> %7 </td>"
                              "<td align='right'> %5 ↻</td><td align='right'>(%6%) </td></tr>")
                          .arg(it.key(), sTime).arg(qreal(it.value().timeInSec)*100 / mSumTimeInSec,0,'f',1)
                          .arg(sMem).arg(it.value().steps).arg(qreal(it.value().steps)*100 / qreal(mSumSteps),0,'f',1)
                          .arg(sRows);
                profileData.append(row);
                ++rows;
            }
            ++it;
        }
        if (rows > 1)
            profileData.append(sum);
        profileData.prepend("<body><table style='white-space:nowrap;'>");
        profileData << "</table></body>";
    }
}

void Profiler::addIncludes(const QList<IncludeLine *> lines)
{
    for (const IncludeLine *line : lines) {
        if (!mFileLine2Cln.contains(line->parentFile)) {
            QMap<int, QPair<int, int>> val;
            mFileLine2Cln.insert(line->parentFile, val);
        }
        QMap<int, QPair<int, int>> &val = mFileLine2Cln[line->parentFile];
        val.insert(line->line, QPair<int,int>(line->contLine, line->outerContLine));
    }
}

QList<QPair<int, qreal> > Profiler::maxTime() const
{
    QList<QPair<int, qreal> > res;
    res.reserve(mMaxSingleTime.size());
    for (int i = 0; i < mMaxSingleTime.size(); ++i) {
        res << QPair<int, qreal>(mMaxSingleTime.at(i).first, mMaxSingleTime.at(i).second.timeInSec);
    }
    return res;
}

QList<QPair<int, int> > Profiler::maxSteps() const
{
    QList<QPair<int, int> > res;
    res.reserve(mMaxSingleSteps.size());
    for (int i = 0; i < mMaxSingleSteps.size(); ++i) {
        res << QPair<int, qreal>(mMaxSingleSteps.at(i).first, mMaxSingleSteps.at(i).second.steps);
    }
    return res;
}

void Profiler::getMaxCompoundValues(qreal &timeSec, size_t &memory, size_t &rows, size_t &steps)
{
    timeSec = mMaxCompountTime;
    memory = mMaxMemory;
    rows = mMaxRows;
    steps = mMaxSteps;
}

} // namespace gamscom
} // namespace studio
} // namespace gams
