/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef MEMORYMAPPER_H
#define MEMORYMAPPER_H

#include "abstracttextmapper.h"
#include "logparser.h"
#include <QMutex>
#include <QTime>
#include <QContiguousCache>

namespace gams {
namespace studio {

class DynamicFile;
//struct LogParser_MarksBlockState;
//class LogParser;

class MemoryMapper : public AbstractTextMapper
{
    Q_OBJECT
private:
    struct Unit {
        Unit(Chunk *chunk = nullptr, QString text = QString())
            : firstChunk(chunk), foldText(text) {}
        Chunk *firstChunk;
        QString foldText;
        int chunkCount = 0;
        bool folded = false;
    };
    struct LineRef {
        Chunk *chunk = nullptr;
        int relLine = 0;
    };

public:
    explicit MemoryMapper(QObject *parent = nullptr);
    AbstractTextMapper::Kind kind() override { return AbstractTextMapper::memoryMapper; }

    void setLogParser(LogParser *parser);
    void setLogFile(DynamicFile *logFile);
    qint64 size() const override;
    void startRun() override;
    void endRun() override;
    int firstErrorLine();
    QString lines(int localLineNrFrom, int lineCount) const override;
    QString lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const override;
    int lineCount() const override;
    int knownLineNrs() const override;
    void setDebugMode(bool debug) override;
    void reset() override;
    void dump();

signals:
    void createMarks(const LogParser::MarkData &marks);
    void appendLines(const QStringList &lines);
    void appendDisplayLines(const QStringList &lines, int startOpen, bool overwriteLast, const QVector<LineFormat> &formats);


public slots:
    void addProcessData(const QByteArray &data);

protected:
    Chunk *getChunk(int chunkNr) const override;
    int chunkCount() const override;

private slots:
    void runFinished();

private:
    void appendLineData(const QByteArray &data, Chunk *&chunk);
    void appendEmptyLine();
    void clearLastLine();
//    void logLastLine();
    void updateOutputCache();
    void fetchLog();
    void fetchDisplay();
    void createErrorMarks(LineRef ref, bool readErrorText);
    LineRef nextRef(const LineRef &ref);
    QByteArray lineData(const LineRef &ref);
    Chunk *addChunk(bool startUnit = false);
    void shrinkLog();
    void recalcLineCount();
    LineRef logLineToRef(const int &lineNr);
    Chunk *nextChunk(Chunk *chunk);
    int currentRunLines();
    void updateChunkMetrics(Chunk *chunk, bool cutRemain = false);
    void recalcSize() const;
    void invalidateSize();

private:
    struct InputState {
        int inQuote = 0;        // 0: outside    1: in single quotes    2: in double quotes
        int refStart = -1;      // index of possible reference start
        int lineState = 0;      // 0: content    1: line-end (\n or \r\n)    2: conceal-prev-line (\r)
    };

    QMutex mSkipper;
    QVector<Chunk*> mChunks;
    QVector<Unit> mUnits;
    QVector<QTextCharFormat> mBaseFormat;
    mutable qint64 mSize = 0;   // this is only a caching value
    int mLineCount = 0;
    bool mShrunk = false;
    LogParser *mLogParser = nullptr;
    LogParser::MarksBlockState mState;
    QVector<int> mMarksHead;
    QContiguousCache<int> mMarksTail;
    QVector<LineRef> mMarkers;
    int mShrinkLineCount = 0;
    QTimer mRunFinishedTimer;
    int mErrCount = 0;
    int mCurrentLstLineRef = -1;
    int mCurrentErrorNr = -1;
    QString mCurrentErrText;

    bool mLastLineIsOpen = false;
    int mLastLineLen = 0;
    QStringList mNewLogLines;
    QTime mDisplayCacheChanged;
    QStringList mDisplayNewLines;
    QVector<LineFormat> mDisplayQuickFormats;
    int mDisplayLastLineLen = 0;
    bool mDisplayLinesOverwrite = false;
    int mConcealPos = 0;
    int mAddedLines = 0;
    InputState mInputState;
};

} // namespace studio
} // namespace gams

#endif // MEMORYMAPPER_H
