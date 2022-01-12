/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include "testfilemapper.h"
#include "logger.h"

#include <QtGlobal>
#include <QStandardPaths>
#include <QClipboard>
#include <QApplication>

using gams::studio::FileMapper;

const QString testFileName("testtextmapper.tmp");

void TestFileMapper::initTestCase()
{
    mCurrentPath = QDir::current();
    QFile file(mCurrentPath.absoluteFilePath(testFileName));
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
    stream << "This is line " << QString(nrChars-1, ' ').toLatin1() << 1
           << " of the testfile. And here are additional characters to get sufficient long lines."
           << endl << flush;
    for (int line = 1; line < maxLine-1; ++line) {
        int nrCurrent = QString::number(line+1).length();
        stream << "This is line "<< QString(nrChars-nrCurrent, ' ').toLatin1() << line+1
               << " of the testfile. And here are additional characters to get sufficient long lines." << endl;
    }
    stream << "This is line " << maxLine << trUtf8(" of the testfile - the last numerated.") << endl;
    stream << trUtf8("Some characters 'äüößÄÜÖê€µ@' to test the codec.") << flush;
    file.close();
}
void TestFileMapper::cleanupTestCase()
{
    QFile file(mCurrentPath.absoluteFilePath(testFileName));
    file.remove();
}


void TestFileMapper::init()
{
    mMapper = new FileMapper();
    mMapper->setCodec(QTextCodec::codecForName("utf-8"));
    mMapper->setMappingSizes(100, 1024*16, 512);
    QVERIFY2(mMapper->openFile(mCurrentPath.absoluteFilePath(testFileName), true),
             "TextMapper: Error on opening test file.");
}

void TestFileMapper::cleanup()
{
    mMapper->startRun();
    delete mMapper;
    mMapper = nullptr;
}

void TestFileMapper::testFile()
{
    QFile file(mCurrentPath.absoluteFilePath(testFileName));
    qint64 size = file.size();
    QCOMPARE(mMapper->size(), size);
}


void TestFileMapper::testReadChunk0()
{
    int max = 1234567890;
    int c[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};
    int digits = max<c[4] ? (max<c[2] ? (max<c[1] ? 1 : 2) : (max<c[3] ? 3 : 4))
              : (max<c[8] ? (max<c[6] ? (max<c[5] ? 5 : 6) : (max<c[7] ? 7 : 8)) : (max<c[9] ? 9 : 10));
    qDebug() << " max: " << max << " digits: " << digits;

    // ---------- check reading chunk 0
    mMapper->setVisibleTopLine(0.0);
    QCOMPARE(mMapper->lines(0,1), "This is line     1 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->topChunk(), 0);
}

void TestFileMapper::testReadChunk1()
{
    // ---------- check reading chunk 1
    mMapper->setVisibleTopLine(0.005);
    QCOMPARE(mMapper->lines(0,1), "This is line   218 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->topChunk(), 1);
    QCOMPARE(mMapper->absTopLine(), 217);
}

void TestFileMapper::testMoveBackAChunk()
{
    // ---------- check moving back across the chunk border
    mMapper->setVisibleTopLine(0.005);
    mMapper->moveVisibleTopLine(-67);
    QCOMPARE(mMapper->absTopLine(), 150);
    QCOMPARE(mMapper->topChunk(), 0);
}

void TestFileMapper::testFetchBeyondChunk()
{
    // ---------- check reading multiple chunks in a row
    mMapper->moveVisibleTopLine(193);
    // ---------- check fetching lines across the chunk border
    QCOMPARE(mMapper->absTopLine(), 160);
    QCOMPARE(mMapper->topChunk(), 0);
    QCOMPARE(mMapper->lines(0,1), "This is line   161 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->lines(1,1), "This is line   162 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->lines(2,1), "This is line   163 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->lines(3,1), "This is line   164 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->lines(103,1), "This is line   264 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->lines(203,1), "This is line   364 of the testfile. And here are additional characters to get sufficient long lines.");
    QCOMPARE(mMapper->topChunk(), 2);
    QCOMPARE(mMapper->absTopLine(), 160);
    QCOMPARE(mMapper->topChunk(), 0);
}

void TestFileMapper::testReadLines()
{
    // ---------- check read lines
    mMapper->setVisibleTopLine(0.0104);
    QCOMPARE(mMapper->topChunk(), 3);
    QVERIFY(mMapper->absTopLine() < 0);
    mMapper->moveVisibleTopLine(-45);
    QString line1 = mMapper->lines(0,1);
    mMapper->moveVisibleTopLine(-1);
    QCOMPARE(mMapper->lines(1,1), line1);
    mMapper->moveVisibleTopLine(-1);
    QStringList lines = mMapper->lines(0, 4).split("\n");
    QCOMPARE(lines.size(), 4);
    QCOMPARE(lines.at(2), line1);
}


void TestFileMapper::testUpdateLineCounting()
{
    // ---------- check updating of lineCounting
    mMapper->setVisibleTopLine(0.004);
    QCOMPARE(mMapper->topChunk(), 1);
    QCOMPARE(mMapper->knownLineNrs(), 324);

    mMapper->setVisibleTopLine(0.015);
    // NOT all chunks are known from start of file, so the line number just estimated.
    QCOMPARE(mMapper->topChunk(), 4);
    QVERIFY(mMapper->absTopLine() < 0);

    mMapper->setVisibleTopLine(0.012);
    QCOMPARE(mMapper->topChunk(), 3);
    QCOMPARE(mMapper->knownLineNrs(), 324);

    mMapper->setVisibleTopLine(0.01);
    QCOMPARE(mMapper->topChunk(), 2);
    // all chunks are known from start of file, so the line number is known, too.
    mMapper->setVisibleTopLine(0.015);
    QCOMPARE(mMapper->topChunk(), 4);
    QCOMPARE(mMapper->absTopLine(), 717);
    QCOMPARE(mMapper->knownLineNrs(), 811);

    mMapper->setVisibleTopLine(0);
    qDebug() << mMapper->lines(1,1);
    qDebug() << mMapper->lines(49999, 1);
    qDebug() << mMapper->lines(50000, 1);
    mMapper->setVisibleTopLine(20000);
    mMapper->setVisibleTopLine(1.0);
    qDebug() << mMapper->lines(0,1);
}

void TestFileMapper::testPeekChunkLineNrs()
{
    // ---------- check peek chunk line numbers
    mMapper->setVisibleTopLine(0.015);
    QVERIFY(mMapper->absTopLine() < 0);
    mMapper->peekChunksForLineNrs();
    QCOMPARE(mMapper->absTopLine(), 717);
}

void TestFileMapper::testLineNrEstimation()
{
    // ---------- check line number esimation
    mMapper->setVisibleTopLine(0.04);
    QCOMPARE(mMapper->absTopLine(), -1971);
    mMapper->peekChunksForLineNrs();
    for (int i = 1; i < 11; ++i) {
        if (i==1) QCOMPARE(mMapper->absTopLine(), -1971);
        mMapper->peekChunksForLineNrs();
    }
    QCOMPARE(mMapper->absTopLine(), 1967);
}

void TestFileMapper::testPosAndAnchor()
{
    // ---------- check position and anchor handling
    QPoint pos;
    QPoint anc;
    mMapper->setVisibleTopLine(0.00859); // 40000
    mMapper->setPosRelative(1, 10);
    pos = mMapper->position();
    anc = mMapper->anchor();
    QVERIFY(pos.y() < 0);
    QCOMPARE(pos.x(), 10);
    QVERIFY(anc.y() < 0);
    QCOMPARE(anc.x(), 10);
    mMapper->peekChunksForLineNrs();
    pos = mMapper->position();
    anc = mMapper->anchor();
    QCOMPARE(pos.y(), 397);

    mMapper->setPosRelative(4, 2, QTextCursor::KeepAnchor);
    pos = mMapper->position();
    anc = mMapper->anchor();
    QCOMPARE(pos.y(), 400);
    QCOMPARE(pos.x(), 2);
    QCOMPARE(anc.y(), 397);
    QCOMPARE(anc.x(), 10);

    mMapper->peekChunksForLineNrs();
    mMapper->peekChunksForLineNrs();
    mMapper->peekChunksForLineNrs();
    mMapper->peekChunksForLineNrs();
    mMapper->setVisibleTopLine(1620);
    mMapper->setPosRelative(0, 0);
    mMapper->setPosRelative(310, 1, QTextCursor::KeepAnchor);
    QCOMPARE(mMapper->anchor(true).y(), 0);
    QCOMPARE(mMapper->position(true).y(), 99); // cropped to buffer
    mMapper->setVisibleTopLine(1920);
    QCOMPARE(mMapper->position(true).y(), 10);
}


QTEST_MAIN(TestFileMapper)
