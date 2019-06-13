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
        int chunkNr = -1;
        int relLine = 0;
    };

public:
    explicit MemoryMapper(QObject *parent = nullptr);
    void setLogParser(LogParser *parser);
    void setLogFile(DynamicFile *logFile);
    qint64 size() const override;
    void startRun() override;
    void endRun() override;
    int visibleLineCount() const override;
    QString lines(int localLineNrFrom, int lineCount) const override;
    QString lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const override;
    int lineCount() const override;
    int knownLineNrs() const override;
    void setDebugMode(bool debug) override;
    void dump();


signals:
    void createMarks(const LogParser::MarkData &marks);

public slots:
    void addProcessData(const QByteArray &data);

protected:
    Chunk *getChunk(int chunkNr) const override;
    int chunkCount() const override;

private slots:
    void parseRemain();

private:
    QByteArray popNextLine();
    Chunk *addChunk(bool startUnit = false);
    void shrinkLog();
    void recalcLineCount();

private:
    QMutex mSkipper;
    QVector<Chunk*> mChunks;
    QVector<Unit> mUnits;
    QVector<QTextCharFormat> mBaseFormat;
    qint64 mSize = 0;
    int mLineCount = 0;
    bool mShrunk = false;
    LogParser *mLogParser = nullptr;
    DynamicFile *mLogFile = nullptr;
    LogParser::MarksBlockState mState;
    int mMarkCount = 0;
    QContiguousCache<LogParser::MarkData> mMarksTail;
    LineRef mParsed;
    int mConcealPos = 0;
};

} // namespace studio
} // namespace gams

#endif // MEMORYMAPPER_H
