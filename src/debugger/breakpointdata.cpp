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
    mStartCln4File.clear();
    mCln2Line.clear();
    mFileLine2Cln.clear();
}

bool BreakpointData::addLinesMap(const QString &filename, const QList<int> &fileLines, const QList<int> &contLines)
{
    if (fileLines.isEmpty() || fileLines.size() != contLines.size())
        return false;
    if (mStartCln4File.isEmpty() || mStartCln4File.last() != filename)
        mStartCln4File.insert(contLines.last(), filename);

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
    return mStartCln4File.value(contLine);
}

int BreakpointData::fileLine(int contLine) const
{
    return mCln2Line.value(contLine, -1);
}

void BreakpointData::adjustBreakpoint(const QString &filename, int &fileLine)
{
    const QMap<int, int> map = mFileLine2Cln.value(filename);
    if (map.isEmpty()) return;

    const auto iter = map.lowerBound(fileLine);
    fileLine = (iter == map.constEnd()) ? map.lastKey() : fileLine = iter.key();
}

int BreakpointData::addBreakpoint(const QString &filename, int fileLine)
{
    const QMap<int, int> map = mFileLine2Cln.value(filename);
    int resLine = fileLine;
    if (!map.isEmpty()) {
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
