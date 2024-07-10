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
#ifndef MEMORYMAPPER_H
#define MEMORYMAPPER_H

#include "chunktextmapper.h"
#include "logparser.h"
#include <QMutex>
#include <QTimer>
#include <QElapsedTimer>

namespace gams {
namespace studio {

class LogStack {
    QString pendingLine; // only initially isNull()
    QStringList data;
public:
    LogStack() {}
    void setLine(const QString &line = "") { pendingLine = line.isNull() ? "" : line; }
    void commitLine() { if (!pendingLine.isNull()) data << pendingLine; pendingLine = ""; }
    QStringList popLines(bool finish = false) {
        if (finish && !pendingLine.isEmpty()) commitLine();
        QStringList res = data; data.clear();
        return res;
    }
    int count() { return data.count(); }
};

class MemoryMapper : public ChunkTextMapper
{
    Q_OBJECT
private: // types
    enum Pending { PendingNothing, PendingBlockCountChange, PendingContentChange };
    Q_DECLARE_FLAGS(Pendings, Pending)

    struct InputState {
        int inQuote = 0;        // 0: outside    1: in single quotes    2: in double quotes
        int refStart = -1;      // index of possible reference start
        int lineState = 0;      // 0: content    1: line-end (\n or \r\n)    2: conceal-prev-line (\r)
    };

    struct Unit {
        Unit(Chunk *chunk = nullptr, const QString &text = QString())
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

    template<typename T>
    class RingBuffer {
    public:
        void resize(int size) { dat.resize(size); }
        int size() { return len; }
        void clear() { len = 0; end = 0; }
        T last() { return (end > 0) ? dat.at(end-1) : len ? dat.last() : T(); }
        void append(const T &val) {
            dat[end] = val;
            end = (end+1) % dat.size();
            if (len < dat.size()) ++len;
        }
        QVector<T> data() {
            if (end && end != len) { // swap data at split-point
                QVector<T> left = dat.mid(0, end);
                memmove(&dat[0], &dat[end], size_t(len-end) * sizeof(T));
                memmove(&dat[len-end], &left[0], size_t(left.length()) * sizeof(T));
                end = len % dat.size();
            }
            return dat.mid(0, len);
        }
    private:
        QVector<T> dat;
        int end = 0;
        int len = 0;
    };

public:
    explicit MemoryMapper(QObject *parent = nullptr);
    ~MemoryMapper() override;
    ChunkTextMapper::Kind kind() const override { return ChunkTextMapper::memoryMapper; }

    void setLogParser(LogParser *parser);
    LogParser *logParser();
    qint64 size() const override;
    void startRun() override;
    void endRun() override;
    int firstErrorLine();
    QString lines(int localLineNrFrom, int lineCount) const override;
    QString lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const override;
    int lineCount() const override;
    int knownLineNrs() const override;
    QString findClosestLst(const int &localLine);
    void updateTheme();
    void dump();

signals:
    void createMarks(const LogParser::MarkData &marks);
    void appendLines(const QStringList &lines, bool overwritePreviousLine);
    void switchLst(const QString &lstName) const;
    void registerGeneratedFile(const QString &fileName);
    void updateView();

public slots:
    void reset() override;
    void addProcessLog(const QByteArray &data);

protected:
    Chunk *getChunk(int chunkNr, bool cache = true) const override;
    int chunkCount() const override;
//    ChunkMetrics* chunkMetrics(int chunkNr) const override;
    void internalRemoveChunk(int chunkNr) override;
    int lastChunkWithLineNr() const override;
    int visibleLineCount() const override;

private slots:
    void runFinished();
    void fetchDisplay();
    void processPending();

private: // methods
    void appendLineData(const QByteArray &data, Chunk *&chunk, bool mark);
    void appendEmptyLine(bool mark = false);
    void clearLastLine();
    void parseNewLine();
    void fetchLog();
    void createErrorMarks(LineRef ref, bool readErrorText);
    LineRef nextRef(const LineRef &ref);
    LineRef prevRef(const LineRef &ref);
    QByteArray lineData(const LineRef &ref);
    Chunk *addChunk(bool startUnit = false);
    void shrinkLog(qint64 minBytes);
    bool ensureSpace(qint64 bytes);
    void recalcLineCount();
    LineRef logLineToRef(const int &lineNr);
    Chunk *nextChunk(Chunk *chunk);
    Chunk *prevChunk(Chunk *chunk);
    int currentRunLines();
    void updateChunkMetrics(Chunk *chunk, bool cutRemain = false);
    void invalidateSize();
    void newPending(Pending pending);
    QString extractLstRef(LineRef lineRef);
    void markLastLine();
    QVector<bool> markedLines(int localLineNrFrom, int lineCount) const;

private: // members
    QVector<Chunk*> mChunks;
    QVector<Unit> mUnits;
    QVector<QTextCharFormat> mBaseFormat;
    mutable qint64 mSize = 0;   // this is only a caching value
    int mLineCount = 0;
    LogParser *mLogParser = nullptr;
    QVector<int> mMarksHead;
    RingBuffer<int> mMarksTail;
    QVector<LineRef> mMarkers;
    int mShrinkLineCount = 0;
    QTimer mRunFinishedTimer;
    int mErrCount = 0;
    int mCurrentLstLineRef = -1;
    int mCurrentErrorNr = -1;
    QString mCurrentErrText;
    Pendings mPending;

    bool mWeakLastLogLine = false;
    bool mLastLineIsOpen = false;
    int mLastLineLen = 0;
    QStringList mNewLogLines;
    LogStack mLogStack;
    QElapsedTimer mDisplayCacheChanged;
    QTimer mPendingTimer;
    int mNewLines = 0;
    bool mInstantRefresh = false;
};

} // namespace studio
} // namespace gams

#endif // MEMORYMAPPER_H
