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

const QStringList Profiler::CTimeUnits = {"sec","min","h","day"};
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
    mProfileData.clear();
    mFileLine2Cln.clear();
    mMaxSingleLoops.clear();
    mMaxSingleLoops.clear();
    mMaxCompountTime = 0.;
    mMaxMemory = 0ull;
    mCurrentTimeUnit = 0;
    mCurrentMemoryUnit = 0;
    mSumTimeInSec = 0.;
    mSumLoop = 0ull;
}

bool Profiler::isEmpty()
{
    return mProfileData.isEmpty();
}

void Profiler::setContinuousLineData(ContinuousLineData *contLines)
{
    mContLines = contLines;
}


void Profiler::add(int contLine, const QString &statement, qreal timeSec, size_t memory)
{
    if (!mProfileData.contains(contLine)) {
        QMap<QString, ProfileData> map;
        mProfileData.insert(contLine, map);
    }
    QMap<QString, ProfileData> &map = mProfileData[contLine];

    // update time
    size_t lastMemDelta = 0;
    if (!map.contains(statement)) {
        map.insert(statement, ProfileData(statement, timeSec, memory));
    } else {
        map[statement].add(timeSec, memory);
    }
    lastMemDelta = map[statement].lastMemoryDelta;
    if (!map.contains("")) {
        map.insert("", ProfileData("", timeSec, memory));
    } else {
        map[""].add(timeSec, lastMemDelta);
    }

    // update max values
    if (!CAggStatements.contains(statement.trimmed(), Qt::CaseInsensitive)) {
        mSumTimeInSec += timeSec;
        ++mSumLoop;
        const ProfileData &curPD = map.value("");
        for (int i = 0; i < mMaxSingleTime.size(); ++i)
            if (mMaxSingleTime.at(i).first == contLine) mMaxSingleTime.remove(i--);
        for (int i = 0; i < mMaxSingleLoops.size(); ++i)
            if (mMaxSingleLoops.at(i).first == contLine) mMaxSingleLoops.remove(i);
        int iTime = CMaxEntries;
        int iLoop = CMaxEntries;
        for (int i = 0; i < CMaxEntries; ++i) {
            if (i >= mMaxSingleTime.size()) {
                if (iTime == CMaxEntries) iTime = i;
            } else if (iTime == CMaxEntries && mMaxSingleTime.at(i).second.timeInSec < curPD.timeInSec)
                iTime = i;
            if (i >= mMaxSingleLoops.size()) {
                if (iLoop == CMaxEntries) iLoop = i;
            } else if (iLoop == CMaxEntries && mMaxSingleLoops.at(i).second.loops < curPD.loops)
                iLoop = i;
        }
        if (iTime < CMaxEntries) {
            mMaxSingleTime.insert(iTime, QPair<int, ProfileData>(contLine, curPD));
            if (mMaxSingleTime.size() > CMaxEntries) mMaxSingleTime.removeLast();
        }
        if (iLoop < CMaxEntries) {
            mMaxSingleLoops.insert(iLoop, QPair<int, ProfileData>(contLine, curPD));
            if (mMaxSingleLoops.size() > CMaxEntries) mMaxSingleLoops.removeLast();
        }
        if (curPD.timeInSecMax > mMaxCompountTime) mMaxCompountTime = curPD.timeInSecMax;
        if (curPD.loops > mMaxLoops) mMaxLoops = curPD.loops;
    }
    if (timeSec > mMaxCompountTime) mMaxCompountTime = timeSec;
    if (memory > mMaxMemory) mMaxMemory = memory;

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
                    map.insert("File SUM", ProfileData("", timeSec, memory));
                else
                    map["File SUM"].add(timeSec, memory);
                if (!map.contains(""))
                    map.insert("", ProfileData("", timeSec, memory));
                else
                    map[""].add(timeSec, memory);
                ProfileData &pd = map[""];
                if (pd.timeInSecMax > mMaxCompountTime) mMaxCompountTime = map[""].timeInSecMax;
                if (pd.memoryMax > mMaxMemory) mMaxMemory = map[""].memoryMax;
                if (pd.loops > mMaxLoops) mMaxLoops = map[""].loops;
            }
            ++cit;
        }
        ++fit;
    }
}

void Profiler::getSums(qreal &timeSec, size_t &loops)
{
    timeSec = mSumTimeInSec;
    loops = mSumLoop;
}

int Profiler::continuousLine(QString filename, int localLine)
{
    int res = -1;
    if (mContLines) res = mContLines->continuousLine(filename, localLine);
    if (res < 0) res = mFileLine2Cln.value(filename).value(localLine, {-1,-1}).first;
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

void Profiler::getProfile(int contLine, qreal &timeSec, size_t &memory, size_t &loops)
{
    if (mProfileData.contains(contLine)) {
        const QMap<QString, ProfileData> &map = mProfileData[contLine];
        if (map.contains("")) {
            const ProfileData &prf = map[""];
            timeSec = prf.timeInSec;
            memory = prf.memoryMax;
            loops = prf.loops;
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
                              "<td align='right';> %3 </td><td align='right';> %4 ↻</td><td>(%5%) </td></tr>")
                          .arg(EditorHelper::formatTime(it.value().timeInSec))
                          .arg(qreal(it.value().timeInSec)*100 / mSumTimeInSec,0,'f',1)
                          .arg(EditorHelper::formatMemory(it.value().memoryMax))
                          .arg(it.value().loops)
                          .arg(qreal(it.value().loops)*100 / qreal(mSumLoop),0,'f',1);
            else if (it.key().compare("File SUM") == 0) {
                row = QString("<tr><td align='right';>File SUM: </td><td align='right';> %1 </td><td>(%2%) </td>"
                              "<td align='right';> %3 </td><td align='right';> %4 ↻</td><td>(%5%) </td></tr>")
                          .arg(EditorHelper::formatTime(it.value().timeInSec))
                          .arg(qreal(it.value().timeInSec)*100 / mSumTimeInSec,0,'f',1)
                          .arg(EditorHelper::formatMemory(it.value().memoryMax))
                          .arg(it.value().loops)
                          .arg(qreal(it.value().loops)*100 / qreal(mSumLoop),0,'f',1);
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
                row = QString("<tr><td>%1 </td><td align='right'> %2 </td><td align='right'>(%3%) </td>"
                              "<td align='right'> %4 </td><td align='right'> %5 ↻</td><td align='right'>(%6%) </td></tr>")
                          .arg(it.key(), sTime).arg(qreal(it.value().timeInSec)*100 / mSumTimeInSec,0,'f',1)
                          .arg(sMem).arg(it.value().loops).arg(qreal(it.value().loops)*100 / qreal(mSumLoop),0,'f',1);
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
    for (int i = 0; i < mMaxSingleTime.size(); ++i) {
        res << QPair<int, qreal>(mMaxSingleTime.at(i).first, mMaxSingleTime.at(i).second.timeInSec);
    }
    return res;
}

QList<QPair<int, int> > Profiler::maxLoops() const
{
    QList<QPair<int, int> > res;
    for (int i = 0; i < mMaxSingleLoops.size(); ++i) {
        res << QPair<int, qreal>(mMaxSingleLoops.at(i).first, mMaxSingleLoops.at(i).second.loops);
    }
    return res;
}

void Profiler::getMaxCompoundValues(qreal &timeSec, size_t &memory, size_t &loops)
{
    timeSec = mMaxCompountTime;
    memory = mMaxMemory;
    loops = mMaxLoops;
}

} // namespace gamscom
} // namespace studio
} // namespace gams
