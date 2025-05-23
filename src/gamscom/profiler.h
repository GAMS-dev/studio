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
#ifndef PROFILER_H
#define PROFILER_H

#include <QObject>
#include <QList>
#include <QMap>

namespace gams {
namespace studio {
namespace gamscom {

class ContinuousLineData;

struct IncludeLine {
    IncludeLine(const QString &_file, int _line, int _contLine) : parentFile(_file), line(_line), contLine(_contLine) { }
    IncludeLine(const QString &_file) : parentFile(_file), line(1), contLine(1), outerContLine(1) { }
    QString toString() const {
        return QString("%1:%2 [%3,%4] INC %5").arg(parentFile).arg(line).arg(contLine).arg(outerContLine).arg(childFile);
    }
    QString parentFile;
    int line;
    int contLine;
    int outerContLine = std::numeric_limits<int>::max();
    QString childFile;
};

struct ProfileData {
    ProfileData() {}
    ProfileData(const QString &s, qreal time, size_t mem, size_t rows): statement(s)
        , timeInSec(time), timeInSecMin(time), timeInSecMax(time)
        , memoryMin(mem), memoryMax(mem), lastMemoryDelta(mem), rowsMin(rows), rowsMax(rows), steps(1) {}
    void add(qreal time, size_t mem, size_t rows) {
        if (timeInSecMin > time) timeInSecMin = time;
        if (timeInSecMax < time) timeInSecMax = time;
        timeInSec += time;
        if (memoryMin > mem) memoryMin = mem;
        if (memoryMax < mem) {
            lastMemoryDelta = mem - memoryMax;
            memoryMax = mem;
        }
        if (rowsMin > rows) rowsMin = rows;
        if (rowsMax < rows) rowsMax = rows;
        ++steps;
    }
    QString statement;
    qreal timeInSec = 0.;
    qreal timeInSecMin = 0.;
    qreal timeInSecMax = 0.;
    size_t memoryMin = 0ull;
    size_t memoryMax = 0ull;
    size_t lastMemoryDelta = 0ull;
    size_t rowsMin = 0ull;
    size_t rowsMax = 0ull;
    size_t steps = 0ull;
};

class Profiler : public QObject
{
    Q_OBJECT
public:
    Profiler();
    ~Profiler();
    void setMainFile(const QString &filepath) { mMainFile = filepath; }
    void clear();
    bool isEmpty();
    void setContinuousLineData(ContinuousLineData *contLines);
    void add(int contLine, const QString &statement, qreal timeSec, size_t memory, size_t rows);
    void getSums(qreal &timeSec, size_t &rows, size_t &steps);
    int continuousLine(QString filename, int localLine);

    void getUnits(QStringList &timeUnits, QStringList &memUnits);
    void getCurrentUnits(int &timeUnit, int &memUnit);
    void setCurrentUnits(int timeUnit = -1, int memUnit = -1);
    void addIncludes(const QList<IncludeLine *> lines);

    void getProfile(int contLine, qreal &timeSec, size_t &memory, size_t &rows, size_t &steps);
    void getProfileText(int contLine, QStringList &profileData);

    void getMaxCompoundValues(qreal &timeSec, size_t &memory, size_t &rows, size_t &steps);
    QList<QPair<int, qreal> > maxTime() const;
    QList<QPair<int, int> > maxSteps() const;

    static const QStringList CTimeUnits;
    static const QStringList CMemoryUnits;
    static const QStringList CAggStatements;

private:
    ContinuousLineData *mContLines = nullptr;

    QMap<int, QMap<QString, ProfileData>> mProfileData;
    QMap<QString, QMap<int, QPair<int, int>> > mFileLine2Cln; // filename, localLine [contLine, contLineEnd]
    QList<QPair<int, ProfileData>> mMaxSingleTime;
    QList<QPair<int, ProfileData>> mMaxSingleSteps;
    QString mMainFile;
    qreal mMaxCompountTime = 0.;
    size_t mMaxMemory = 0ull;
    size_t mMaxRows = 0ull;
    size_t mMaxSteps = 0ull;
    int mCurrentTimeUnit = 0;
    int mCurrentMemoryUnit = 0;
    qreal mSumTimeInSec = 0.;
    size_t mSumRows = 0ull;
    size_t mSumSteps = 0ull;
};

} // namespace gamscom
} // namespace studio
} // namespace gams

#endif // PROFILER_H
