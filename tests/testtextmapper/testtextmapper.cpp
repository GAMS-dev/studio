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
#include "testtextmapper.h"
#include "editors/textmapper.h"
#include "logger.h"

#include <QtGlobal>
#include <QStandardPaths>

using gams::studio::TextMapper;

const QString testFileName("testtextmapper.tmp");

void TestTextMapper::initTestCase()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    QString message("Error on opening test file '%1'.");
    bool opened = file.open(QIODevice::WriteOnly);
    if (!opened) {
        qDebug() << file.errorString();
    }
    QVERIFY2(opened, message.arg(file.fileName()).toLatin1());

    // -------------- create a test file
    QTextStream stream(&file);
    stream.setCodec(QTextCodec::codecForName("utf-8"));
    qDebug() << "Codec: " << stream.codec()->name();

    int maxLine = 50000;
    int nrChars = QString::number(maxLine).length();
    stream << "This is line " << QString(nrChars-1, ' ').toLatin1() << 1 << " of the testfile. And here are additional characters to get sufficient long lines." << endl << flush;
    for (int line = 1; line < maxLine-1; ++line) {
        int nrCurrent = QString::number(line+1).length();
        stream << "This is line "<< QString(nrChars-nrCurrent, ' ').toLatin1() << line+1 << " of the testfile. And here are additional characters to get sufficient long lines." << endl;
    }
    stream << "This is line " << maxLine << trUtf8(" of the testfile - the last numerated.") << endl;
    stream << trUtf8("Some characters 'äüößÄÜÖê€µ@' to test the codec.") << flush;
    file.close();
}

void TestTextMapper::cleanupTestCase()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    file.remove();
}

void TestTextMapper::testReadChunk0()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();

    { // ----------------- TEST usage of the test file with TestMapper
        TextMapper tm;
        tm.setCodec(QTextCodec::codecForName("utf-8"));
        tm.setMappingSizes(100, 1024*16, 512);
        QVERIFY2(tm.openFile(file.fileName()), "TextMapper: Error on opening test file.");
        QCOMPARE(tm.sizeMapper().size, size);

        // ---------- check reading chunk 0
        tm.setTopOffset(0);
        QCOMPARE(tm.line(0), "This is line     1 of the testfile. And here are additional characters to get sufficient long lines.");
        QCOMPARE(tm.topChunk(), 0);
        QCOMPARE(tm.absPos(0,0), 0);

    }
}

void TestTextMapper::testReadChunk1()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();

    { // ----------------- TEST usage of the test file with TestMapper
        TextMapper tm;
        tm.setCodec(QTextCodec::codecForName("utf-8"));
        tm.setMappingSizes(100, 1024*16, 512);
        QVERIFY2(tm.openFile(file.fileName()), "TextMapper: Error on opening test file.");
        QCOMPARE(tm.sizeMapper().size, size);

        // ---------- check reading chunk 1
        tm.setTopOffset(20000);
        QCOMPARE(tm.line(0), "This is line   199 of the testfile. And here are additional characters to get sufficient long lines.");
        QCOMPARE(tm.topChunk(), 1);
        QCOMPARE(tm.absTopLine(), 198);

    }
}

void TestTextMapper::testMoveBackAChunk()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();

    { // ----------------- TEST usage of the test file with TestMapper
        TextMapper tm;
        tm.setCodec(QTextCodec::codecForName("utf-8"));
        tm.setMappingSizes(100, 1024*16, 512);
        QVERIFY2(tm.openFile(file.fileName()), "TextMapper: Error on opening test file.");
        QCOMPARE(tm.sizeMapper().size, size);

        // ---------- check moving back across the chunk border
        tm.setTopOffset(20000);
        tm.moveTopByLines(-48);
        QCOMPARE(tm.absTopLine(), 150);
        QCOMPARE(tm.topChunk(), 0);
    }
}

void TestTextMapper::testFetchBeyondChunk()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();

    { // ----------------- TEST usage of the test file with TestMapper
        TextMapper tm;
        tm.setCodec(QTextCodec::codecForName("utf-8"));
        tm.setMappingSizes(100, 1024*16, 512);
        QVERIFY2(tm.openFile(file.fileName()), "TextMapper: Error on opening test file.");
        QCOMPARE(tm.sizeMapper().size, size);

        // ---------- check reading multiple chunks in a row
        tm.moveTopByLines(160);
        // ---------- check fetching lines across the chunk border
        QCOMPARE(tm.absTopLine(), 160);
        QCOMPARE(tm.topChunk(), 0);
        QCOMPARE(tm.line(0), "This is line   161 of the testfile. And here are additional characters to get sufficient long lines.");
        QCOMPARE(tm.line(1), "This is line   162 of the testfile. And here are additional characters to get sufficient long lines.");
        QCOMPARE(tm.line(2), "This is line   163 of the testfile. And here are additional characters to get sufficient long lines.");
        QCOMPARE(tm.line(3), "This is line   164 of the testfile. And here are additional characters to get sufficient long lines.");
        QCOMPARE(tm.line(103), "This is line   264 of the testfile. And here are additional characters to get sufficient long lines.");
        QCOMPARE(tm.line(203), "This is line   364 of the testfile. And here are additional characters to get sufficient long lines.");
        QCOMPARE(tm.topChunk(), 2);
        QCOMPARE(tm.absTopLine(), 160);
        QCOMPARE(tm.topChunk(), 0);
    }
}

void TestTextMapper::testPosString2Raw()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();

    { // ----------------- TEST usage of the test file with TestMapper
        TextMapper tm;
        tm.setCodec(QTextCodec::codecForName("utf-8"));
        tm.setMappingSizes(100, 1024*16, 512);
        QVERIFY2(tm.openFile(file.fileName()), "TextMapper: Error on opening test file.");
        QCOMPARE(tm.sizeMapper().size, size);

        // ---------- check positioning in codec vs raw data
        tm.setTopOffset(5000000);
        QCOMPARE(tm.topChunk(), 305);
        QCOMPARE(tm.line(0), "This is line 49505 of the testfile. And here are additional characters to get sufficient long lines.");

        tm.moveTopByLines(496);
        QCOMPARE(tm.topChunk(), 308);
        //qDebug() << "LAST LINE: " << tm.line(0);
        QCOMPARE(tm.relPos(0,17)-tm.relPos(0,0), 17);
        QCOMPARE(tm.relPos(0,18)-tm.relPos(0,0), 19);
        QCOMPARE(tm.relPos(0,24)-tm.relPos(0,0), 31);
        QCOMPARE(tm.relPos(0,25)-tm.relPos(0,0), 33);

        // ---------- check updating of lineCounting
        tm.setTopOffset(20000);
        qDebug() << "lastChunkWithLines: " << tm.lastChunkWithLines();
        tm.setTopOffset(70000);
        qDebug() << "lastChunkWithLines: " << tm.lastChunkWithLines();
        qDebug() << "Current absolute top line should be less than 0: " << tm.absTopLine();
        QVERIFY(tm.absTopLine() < 0);
        tm.setTopOffset(50000);
        qDebug() << "lastChunkWithLines: " << tm.lastChunkWithLines();
        tm.setTopOffset(35000);
        qDebug() << "lastChunkWithLines: " << tm.lastChunkWithLines();
        tm.setTopOffset(70000);
        QCOMPARE(tm.absTopLine(), 693);

//        qDebug() << "RAW: "  << tm.rawLine(0);
    }
}

void TestTextMapper::testReadLines()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();

    { // ----------------- TEST usage of the test file with TestMapper
        TextMapper tm;
        tm.setCodec(QTextCodec::codecForName("utf-8"));
        tm.setMappingSizes(100, 1024*16, 512);
        QVERIFY2(tm.openFile(file.fileName()), "TextMapper: Error on opening test file.");
        QCOMPARE(tm.sizeMapper().size, size);

        // ---------- check read lines
        tm.setTopOffset(70000);
        QVERIFY(tm.absTopLine() < 0);
        tm.moveTopByLines(-45);
        QString line1 = tm.line(0);
        tm.moveTopByLines(-1);
        QCOMPARE(tm.line(1), line1);
        tm.moveTopByLines(-1);
        QStringList lines = tm.lines(0, 4).split("\n");
        QCOMPARE(lines.size(), 4);
        QCOMPARE(lines.at(2), line1);
    }
}


void TestTextMapper::testUpdateLineCounting()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();

    { // ----------------- TEST usage of the test file with TestMapper
        TextMapper tm;
        tm.setCodec(QTextCodec::codecForName("utf-8"));
        tm.setMappingSizes(100, 1024*16, 512);
        QVERIFY2(tm.openFile(file.fileName()), "TextMapper: Error on opening test file.");
        QCOMPARE(tm.sizeMapper().size, size);

        // ---------- check updating of lineCounting
        tm.setTopOffset(20000);
        QCOMPARE(tm.topChunk(), 1);
        QCOMPARE(tm.knownLineNrs(), 324);

        tm.setTopOffset(70000);
        // NOT all chunks are known from start of file, so the line number just estimated.
        QCOMPARE(tm.topChunk(), 4);
        QVERIFY(tm.absTopLine() < 0);

        tm.setTopOffset(50000);
        QCOMPARE(tm.topChunk(), 3);
        QCOMPARE(tm.knownLineNrs(), 324);

        tm.setTopOffset(40000);
        QCOMPARE(tm.topChunk(), 2);
        // all chunks are known from start of file, so the line number is known, too.
        tm.setTopOffset(70000);
        QCOMPARE(tm.topChunk(), 4);
        QCOMPARE(tm.absTopLine(), 693);
        QCOMPARE(tm.knownLineNrs(), 811);

//        while (tm.lineCount() < 0) {
//            tm.peekChunksForLineNrs(50);
//            qDebug() << "Last counted chunk: " << tm.lastChunkWithLines() << "  lineCount: " << tm.lineCount();
//        }
        tm.setTopLine(0);
        qDebug() << tm.line(1);
        qDebug() << tm.lines(49999, 1);
        qDebug() << tm.lines(50000, 1);
        tm.setTopLine(20000);
        tm.setTopLine(50000);
        qDebug() << tm.line(0);
        QCOMPARE(tm.topChunk(), tm.chunkCount()-1);
    }
}

void TestTextMapper::testPeekChunkLineNrs()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();

    { // ----------------- TEST usage of the test file with TestMapper
        TextMapper tm;
        tm.setCodec(QTextCodec::codecForName("utf-8"));
        tm.setMappingSizes(100, 1024*16, 512);
        QVERIFY2(tm.openFile(file.fileName()), "TextMapper: Error on opening test file.");
        QCOMPARE(tm.sizeMapper().size, size);

        // ---------- check peek chunk line numbers
        tm.setTopOffset(70000);
        QVERIFY(tm.absTopLine() < 0);
        tm.peekChunksForLineNrs(4);
        QCOMPARE(tm.absTopLine(), 693);
    }
}

void TestTextMapper::testPosCalulation()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();

    { // ----------------- TEST usage of the test file with TestMapper
        TextMapper tm;
        tm.setCodec(QTextCodec::codecForName("utf-8"));
        tm.setMappingSizes(100, 1024*16, 512);
        QVERIFY2(tm.openFile(file.fileName()), "TextMapper: Error on opening test file.");
        QCOMPARE(tm.sizeMapper().size, size);

        // ---------- check position calculation
        tm.setTopOffset(39800);
        QCOMPARE(tm.relTopLine(), 70);
        tm.setTopOffset(39900);
        QCOMPARE(tm.relTopLine(), 71);
        tm.setTopOffset(40000);
        QCOMPARE(tm.relTopLine(), 72);
        QVERIFY(tm.absTopLine() < 0);
        QCOMPARE(tm.relPos(100, 4), 10104);
        tm.peekChunksForLineNrs(4);
        QCOMPARE(tm.absPos(396, 0) + tm.relPos(100, 4), tm.absPos(496, 4));
        QCOMPARE(tm.absTopLine(), 396);
        QCOMPARE(tm.absPos(396, 0), 39895);
        QCOMPARE(tm.absPos(496, 4), 49999);
    }
}

void TestTextMapper::testLineNrEstimation()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();

    { // ----------------- TEST usage of the test file with TestMapper
        TextMapper tm;
        tm.setCodec(QTextCodec::codecForName("utf-8"));
        tm.setMappingSizes(100, 1024*16, 512);
        QVERIFY2(tm.openFile(file.fileName()), "TextMapper: Error on opening test file.");
        QCOMPARE(tm.sizeMapper().size, size);

        // ---------- check line number esimation
        tm.setTopOffset(200000);
        QCOMPARE(tm.absTopLine(), -1984);
        tm.peekChunksForLineNrs(1);
        for (int i = 1; i < 11; ++i) {
            if (i==1) QCOMPARE(tm.absTopLine(), -1984);
            tm.peekChunksForLineNrs(1);
        }

        QCOMPARE(tm.absTopLine(), 1980);
    }
}

void TestTextMapper::testFindLine()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();

    { // ----------------- TEST usage of the test file with TestMapper
        TextMapper tm;
        tm.setCodec(QTextCodec::codecForName("utf-8"));
        tm.setMappingSizes(100, 1024*16, 512);
        QVERIFY2(tm.openFile(file.fileName()), "TextMapper: Error on opening test file.");
        QCOMPARE(tm.sizeMapper().size, size);

        // ---------- check find line
        tm.setTopOffset(40000);
        QVERIFY(!tm.setTopLine(400));
        tm.peekChunksForLineNrs(4);
        QVERIFY(tm.setTopLine(400));
        QCOMPARE(tm.findChunk(800), 4);
        tm.peekChunksForLineNrs(16);
        QCOMPARE(tm.findChunk(2000), 12);
        QCOMPARE(tm.findChunk(1946), 12);
        QCOMPARE(tm.findChunk(1945), 11);
        QCOMPARE(tm.findChunk(1946+162), 13);
    }
}

void TestTextMapper::testPosAndAnchor()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();

    { // ----------------- TEST usage of the test file with TestMapper
        TextMapper tm;
        tm.setCodec(QTextCodec::codecForName("utf-8"));
        tm.setMappingSizes(100, 1024*16, 512);
        QVERIFY2(tm.openFile(file.fileName()), "TextMapper: Error on opening test file.");
        QCOMPARE(tm.sizeMapper().size, size);

        // ---------- check position and anchor handling
        QPoint pos;
        QPoint anc;
        tm.setTopOffset(40000);
        tm.setRelPos(1, 10);
        tm.getPosAndAnchor(pos, anc);
        QVERIFY(pos.y() < 0);
        QCOMPARE(pos.x(), 10);
        QVERIFY(anc.y() < 0);
        QCOMPARE(anc.x(), 10);
        tm.peekChunksForLineNrs(4);
        tm.getPosAndAnchor(pos, anc);
        QCOMPARE(pos.y(), 397);

        tm.setRelPos(4, 2, QTextCursor::KeepAnchor);
        tm.getPosAndAnchor(pos, anc);
        QCOMPARE(pos.y(), 400);
        QCOMPARE(pos.x(), 2);
        QCOMPARE(anc.y(), 397);
        QCOMPARE(anc.x(), 10);
    }
}

QTEST_MAIN(TestTextMapper)
