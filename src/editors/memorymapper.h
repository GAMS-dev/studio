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
#include "file/dynamicfile.h"

namespace gams {
namespace studio {

class MemoryMapper : public AbstractTextMapper
{
    Q_OBJECT
private:
    struct Unit {
        Unit(int idx = -1, QString text = QString()) : firstChunkIndex(idx), foldText(text), folded(true) {}
        int firstChunkIndex;
        QString foldText;
        bool folded;
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
    bool setMappingSizes(int bufferedLines, int chunkSizeInBytes, int chunkOverlap) override;
    void startRun() override;
    void endRun() override;
    QString lines(int localLineNrFrom, int lineCount) const override;
    bool isEmpty() const override;


signals:

public slots:
    void addProcessData(const QByteArray &data);
    void setJumpToLogEnd(bool state);
    void repaint();

protected:
    int chunkCount() const override;
    Chunk *getChunk(int chunkNr) const override;

private:
    QByteArray popNextLine();
    bool parseRemain();
    void startUnit();
    Chunk *addChunk();

private:
    QVector<Chunk*> mChunks;
    QVector<Unit> mUnits;
    qint64 mSize = 0;
    LogParser *mLogParser = nullptr;
    DynamicFile *mLogFile = nullptr;
    LogParser::MarksBlockState *mState = nullptr;
    LineRef mParsed;
    int mConcealPos = 0;

};

} // namespace studio
} // namespace gams

#endif // MEMORYMAPPER_H
