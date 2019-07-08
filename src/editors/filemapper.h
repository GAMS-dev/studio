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
#ifndef FILEMAPPER_H
#define FILEMAPPER_H

#include <QTextCodec>
#include <QVector>
#include <QFile>
#include <QSet>
#include <QTextCursor>
#include <QTextDocument>
#include <QMutex>
#include <QTimer>
#include "abstracttextmapper.h"
//#include "syntax.h"

namespace gams {
namespace studio {

///
/// class FileMapper
/// Opens a file into chunks of QByteArrays that are loaded on request. Uses indexes to build the lines for the
/// model on the fly.
///
class FileMapper: public AbstractTextMapper
{
    Q_OBJECT
public:
    FileMapper(QObject *parent = nullptr);
    ~FileMapper() override;

    bool openFile(const QString &fileName, bool initAnchor);
    qint64 size() const override { return mSize; }
    void startRun() override;
    void endRun() override;
    int lineCount() const override;

public slots:
    void peekChunksForLineNrs();

protected:
    Chunk *getChunk(int chunkNr) const override;
    void chunkUncached(Chunk *&chunk) const override;

private slots:
    void closeAndReset();
    void closeFile();                                           //2FF

private:
    bool reload();

private:
    mutable QFile mFile;        // mutable to provide consistant logical const-correctness
    mutable QMutex mMutex;
    mutable QTimer mTimer;

    qint64 mSize = 0;

    QTimer mPeekTimer;
};

} // namespace studio
} // namespace gams

#endif // FILEMAPPER_H
