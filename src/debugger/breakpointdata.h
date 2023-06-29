#ifndef GAMS_STUDIO_DEBUGGER_BREAKPOINTDATA_H
#define GAMS_STUDIO_DEBUGGER_BREAKPOINTDATA_H

#include <QList>
#include <QMap>
#include <QList>
#include <QPair>

namespace gams {
namespace studio {
namespace debugger {

typedef QMap<int, int> SortedSet; // QMap (instead of QList) handles sort and avoids double entries

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
    void adjustBreakpoint(const QString &filename, int &fileLine, bool skipExist = true);
    int addBreakpoint(const QString &filename, int fileLine);
    void delBreakpoint(const QString &filename, int fileLine);
    void delBreakpoints();
    bool isBreakpoint(const QString &filename, int fileLine) const;
    QStringList bpFiles();
    QList<int> bpFileLines(const QString &filename) const;
    QList<int> bpContinuousLines() const;

private:
    QMap<int, QString> mLastCln4File;
    QMap<int, int> mCln2Line;
    QMap<QString, QMap<int, int> > mFileLine2Cln;

    QMap<QString, SortedSet> mActiveBp;
};

} // namespace debugger
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_DEBUGGER_BREAKPOINTDATA_H
