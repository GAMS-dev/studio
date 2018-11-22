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
#include <QClipboard>
#include <QApplication>
#include <QThread>

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


void TestTextMapper::init()
{
    mMapper = new TextMapper();
    mMapper->setCodec(QTextCodec::codecForName("utf-8"));
    mMapper->setMappingSizes(100, 1024*16, 512);
    QVERIFY2(mMapper->openFile(QDir(QDir::tempPath()).absoluteFilePath(testFileName)),
             "TextMapper: Error on opening test file.");
}

void TestTextMapper::cleanup()
{
    delete mMapper;
    mMapper = nullptr;
}

void TestTextMapper::testFile()
{
    QDir tempDir = QDir::tempPath();
    QFile file(tempDir.absoluteFilePath(testFileName));
    qint64 size = file.size();
    QCOMPARE(mMapper->sizeMapper().size, size);
}


void TestTextMapper::testReadChunk0()
{
    // ---------- check reading chunk 0
    mMapper->setTopOffset(0);
    QCOMPARE(mMapper->line(0), "This is line     1 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->topChunk(), 0);
    QCOMPARE(mMapper->absPos(0,0), 0);
}

void TestTextMapper::testReadChunk1()
{
    // ---------- check reading chunk 1
    mMapper->setTopOffset(20000);
    QCOMPARE(mMapper->line(0), "This is line   199 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->topChunk(), 1);
    QCOMPARE(mMapper->absTopLine(), 198);
}

void TestTextMapper::testMoveBackAChunk()
{
    // ---------- check moving back across the chunk border
    mMapper->setTopOffset(20000);
    mMapper->moveTopByLines(-48);
    QCOMPARE(mMapper->absTopLine(), 150);
    QCOMPARE(mMapper->topChunk(), 0);
}

void TestTextMapper::testFetchBeyondChunk()
{
    // ---------- check reading multiple chunks in a row
    mMapper->moveTopByLines(160);
    // ---------- check fetching lines across the chunk border
    QCOMPARE(mMapper->absTopLine(), 160);
    QCOMPARE(mMapper->topChunk(), 0);
    QCOMPARE(mMapper->line(0), "This is line   161 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->line(1), "This is line   162 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->line(2), "This is line   163 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->line(3), "This is line   164 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->line(103), "This is line   264 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->line(203), "This is line   364 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->topChunk(), 2);
    QCOMPARE(mMapper->absTopLine(), 160);
    QCOMPARE(mMapper->topChunk(), 0);
}

void TestTextMapper::testPosString2Raw()
{
    // ---------- check positioning in codec vs raw data
    mMapper->setTopOffset(5000000);
    QCOMPARE(mMapper->topChunk(), 305);
    QCOMPARE(mMapper->line(0), "This is line 49505 of the testfile. And here are additional characters to get sufficient long lines.");

    mMapper->moveTopByLines(496);
    QCOMPARE(mMapper->topChunk(), 308);
    //qDebug() << "LAST LINE: " << mMapper->line(0);
    QCOMPARE(mMapper->relPos(0,17)-mMapper->relPos(0,0), 17);
    QCOMPARE(mMapper->relPos(0,18)-mMapper->relPos(0,0), 19);
    QCOMPARE(mMapper->relPos(0,24)-mMapper->relPos(0,0), 31);
    QCOMPARE(mMapper->relPos(0,25)-mMapper->relPos(0,0), 33);

    // ---------- check updating of lineCounting
    mMapper->setTopOffset(20000);
    qDebug() << "lastChunkWithLines: " << mMapper->lastChunkWithLines();
    mMapper->setTopOffset(70000);
    qDebug() << "lastChunkWithLines: " << mMapper->lastChunkWithLines();
    qDebug() << "Current absolute top line should be less than 0: " << mMapper->absTopLine();
    QVERIFY(mMapper->absTopLine() < 0);
    mMapper->setTopOffset(50000);
    qDebug() << "lastChunkWithLines: " << mMapper->lastChunkWithLines();
    mMapper->setTopOffset(35000);
    qDebug() << "lastChunkWithLines: " << mMapper->lastChunkWithLines();
    mMapper->setTopOffset(70000);
    QCOMPARE(mMapper->absTopLine(), 693);

//        qDebug() << "RAW: "  << mMapper->rawLine(0);
}

void TestTextMapper::testReadLines()
{
    // ---------- check read lines
    mMapper->setTopOffset(70000);
    QVERIFY(mMapper->absTopLine() < 0);
    mMapper->moveTopByLines(-45);
    QString line1 = mMapper->line(0);
    mMapper->moveTopByLines(-1);
    QCOMPARE(mMapper->line(1), line1);
    mMapper->moveTopByLines(-1);
    QStringList lines = mMapper->lines(0, 4).split("\n");
    QCOMPARE(lines.size(), 4);
    QCOMPARE(lines.at(2), line1);
}


void TestTextMapper::testUpdateLineCounting()
{
    // ---------- check updating of lineCounting
    mMapper->setTopOffset(20000);
    QCOMPARE(mMapper->topChunk(), 1);
    QCOMPARE(mMapper->knownLineNrs(), 324);

    mMapper->setTopOffset(70000);
    // NOT all chunks are known from start of file, so the line number just estimated.
    QCOMPARE(mMapper->topChunk(), 4);
    QVERIFY(mMapper->absTopLine() < 0);

    mMapper->setTopOffset(50000);
    QCOMPARE(mMapper->topChunk(), 3);
    QCOMPARE(mMapper->knownLineNrs(), 324);

    mMapper->setTopOffset(40000);
    QCOMPARE(mMapper->topChunk(), 2);
    // all chunks are known from start of file, so the line number is known, too.
    mMapper->setTopOffset(70000);
    QCOMPARE(mMapper->topChunk(), 4);
    QCOMPARE(mMapper->absTopLine(), 693);
    QCOMPARE(mMapper->knownLineNrs(), 811);

    mMapper->setTopLine(0);
    qDebug() << mMapper->line(1);
    qDebug() << mMapper->lines(49999, 1);
    qDebug() << mMapper->lines(50000, 1);
    mMapper->setTopLine(20000);
    mMapper->setTopLine(50000);
    qDebug() << mMapper->line(0);
    QCOMPARE(mMapper->topChunk(), mMapper->chunkCount()-1);
}

void TestTextMapper::testPeekChunkLineNrs()
{
    // ---------- check peek chunk line numbers
    mMapper->setTopOffset(70000);
    QVERIFY(mMapper->absTopLine() < 0);
    mMapper->peekChunksForLineNrs(4);
    QCOMPARE(mMapper->absTopLine(), 693);
}

void TestTextMapper::testPosCalulation()
{
    // ---------- check position calculation
    mMapper->setTopOffset(39800);
    QCOMPARE(mMapper->relTopLine(), 70);
    mMapper->setTopOffset(39900);
    QCOMPARE(mMapper->relTopLine(), 71);
    mMapper->setTopOffset(40000);
    QCOMPARE(mMapper->relTopLine(), 72);
    QVERIFY(mMapper->absTopLine() < 0);
    QCOMPARE(mMapper->relPos(100, 4), 10104);
    mMapper->peekChunksForLineNrs(4);
    QCOMPARE(mMapper->absPos(396, 0) + mMapper->relPos(100, 4), mMapper->absPos(496, 4));
    QCOMPARE(mMapper->absTopLine(), 396);
    QCOMPARE(mMapper->absPos(396, 0), 39895);
    QCOMPARE(mMapper->absPos(496, 4), 49999);
}

void TestTextMapper::testLineNrEstimation()
{
    // ---------- check line number esimation
    mMapper->setTopOffset(200000);
    QCOMPARE(mMapper->absTopLine(), -1984);
    mMapper->peekChunksForLineNrs(1);
    for (int i = 1; i < 11; ++i) {
        if (i==1) QCOMPARE(mMapper->absTopLine(), -1984);
        mMapper->peekChunksForLineNrs(1);
    }
    QCOMPARE(mMapper->absTopLine(), 1980);
}

void TestTextMapper::testFindLine()
{
    // ---------- check find line
    mMapper->setTopOffset(40000);
    QVERIFY(!mMapper->setTopLine(400));
    mMapper->peekChunksForLineNrs(4);
    QVERIFY(mMapper->setTopLine(400));
    QCOMPARE(mMapper->findChunk(800), 4);
    mMapper->peekChunksForLineNrs(16);
    QCOMPARE(mMapper->findChunk(2000), 12);
    QCOMPARE(mMapper->findChunk(1946), 12);
    QCOMPARE(mMapper->findChunk(1945), 11);
    QCOMPARE(mMapper->findChunk(1946+162), 13);
}

void TestTextMapper::testPosAndAnchor()
{
    // ---------- check position and anchor handling
    QPoint pos;
    QPoint anc;
    mMapper->setTopOffset(40000);
    mMapper->setRelPos(1, 10);
    mMapper->getPosAndAnchor(pos, anc);
    QVERIFY(pos.y() < 0);
    QCOMPARE(pos.x(), 10);
    QVERIFY(anc.y() < 0);
    QCOMPARE(anc.x(), 10);
    mMapper->peekChunksForLineNrs(4);
    mMapper->getPosAndAnchor(pos, anc);
    QCOMPARE(pos.y(), 397);

    mMapper->setRelPos(4, 2, QTextCursor::KeepAnchor);
    mMapper->getPosAndAnchor(pos, anc);
    QCOMPARE(pos.y(), 400);
    QCOMPARE(pos.x(), 2);
    QCOMPARE(anc.y(), 397);
    QCOMPARE(anc.x(), 10);
}
void TestTextMapper::testClipboard()
{
    ulong ms = 1; // As the windows clipboard cries on fast changes in a row we have to slow it down a bit

    // ---------- check clipbord content
    QClipboard *clip = QApplication::clipboard();
    clip->clear();
    mMapper->setRelPos(2, 0);
    mMapper->setRelPos(2, 7, QTextCursor::KeepAnchor);
    mMapper->copyToClipboard();
    QCOMPARE(clip->text(), QString("This is"));

    QApplication::instance()->thread()->msleep(ms);
    clip->clear();
    mMapper->setRelPos(2, 89);
    mMapper->setRelPos(3, 7, QTextCursor::KeepAnchor);
    mMapper->copyToClipboard();
    QCOMPARE(clip->text(), QString("long lines.\nThis is"));

    QApplication::instance()->thread()->msleep(ms);
    clip->clear();
    mMapper->setRelPos(300, 0);
    mMapper->setRelPos(400, 7, QTextCursor::KeepAnchor);
    mMapper->copyToClipboard();
    QStringList list = clip->text().split("\n");
    QCOMPARE(list.count(), 101);
    QCOMPARE(list.last(), QString("This is"));

    QApplication::instance()->thread()->msleep(ms);
    clip->clear();
    mMapper->setTopOffset(5000000);
    mMapper->moveTopByLines(496);
    QCOMPARE(mMapper->line(0), trUtf8("Some characters 'äüößÄÜÖê€µ@' to test the codec."));
    QCOMPARE(mMapper->topChunk(), 308);
    mMapper->setRelPos(-1, 0);
    mMapper->setRelPos(0, 0, QTextCursor::KeepAnchor);
    mMapper->copyToClipboard();
    QCOMPARE(clip->text(), trUtf8("This is line 50000 of the testfile - the last numerated.\n"));

    QApplication::instance()->thread()->msleep(ms);
    clip->clear();
    mMapper->setRelPos(0, 0);
    mMapper->setRelPos(0, 100, QTextCursor::KeepAnchor);
    mMapper->copyToClipboard();
    QCOMPARE(clip->text(), mMapper->line(0));

    QApplication::instance()->thread()->msleep(ms);
    clip->clear();
    mMapper->setRelPos(-20000, 0);
    mMapper->setRelPos(0, 100, QTextCursor::KeepAnchor);
    mMapper->copyToClipboard();
    QCOMPARE(mMapper->selectionSize(), 2020015);
    // differs in size by 11 for the last lines 11 utf8 characters taking 2 char each
    QCOMPARE(mMapper->selectionSize(), clip->text().length() + 11);

    clip->clear();
}

QTEST_MAIN(TestTextMapper)
