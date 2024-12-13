/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include "testmemorymapper.h"
#include "logger.h"

#include <QtGlobal>
#include <QStandardPaths>
#include <QClipboard>
#include <QApplication>
//#include <QThread>

using gams::studio::MemoryMapper;

const QString testFileName("testtextmapper.tmp");

void TestMemoryMapper::init()
{
    mMapper = new MemoryMapper();
    mMapper->setEncoding("UTF-8");
    mMapper->setMappingSizes(10, 128, 16);
    mMapper->startRun();
}

void TestMemoryMapper::cleanup()
{
    mMapper->endRun();
    delete mMapper;
    mMapper = nullptr;
}

void TestMemoryMapper::testAddLine()
{
    QByteArray arr;
    arr.append("0 ###############\r\n");
    arr.append("1 ###############\r\n");
    arr.append("2 ###############\r\n");
    arr.append("3 ###############\r\n");
    mMapper->addProcessLog(arr);

    arr = ("4 ###############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("5 ##############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("6 #############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("--\n");
    mMapper->addProcessLog(arr);
    arr = ("--\r\n");
    mMapper->addProcessLog(arr);
    arr = ("-\r\n");
    mMapper->addProcessLog(arr);
    arr = ("7 ###############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("8 ###############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("9 ###############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("A ###############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("B ###############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("C ###############\r\n");
    mMapper->addProcessLog(arr);
    mMapper->dump();
    arr = ("D ###############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("E ###############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("F ###############\r\n");
    mMapper->addProcessLog(arr);
    mMapper->dump();
    mMapper->startRun();
    arr = ("G ###############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("H ###############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("I ###############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("J ###############\r\n");
    mMapper->addProcessLog(arr);
    arr = ("K ###############\r\n");
    mMapper->addProcessLog(arr);
    mMapper->dump();

    mMapper->moveVisibleTopLine(10);
    DEB() << "LINES:\n" << mMapper->lines(3,7);
}

//void TestMemoryMapper::testReadChunk0()
//{
//    int max = 1234567890;
//    int c[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};
//    int digits = max<c[4] ? (max<c[2] ? (max<c[1] ? 1 : 2) : (max<c[3] ? 3 : 4))
//              : (max<c[8] ? (max<c[6] ? (max<c[5] ? 5 : 6) : (max<c[7] ? 7 : 8)) : (max<c[9] ? 9 : 10));
//    qDebug() << " max: " << max << " digits: " << digits;

//    // ---------- check reading chunk 0
//    mMapper->setVisibleTopLine(0.0);
//    QCOMPARE(mMapper->lines(0,1), "This is line     1 of the testfile. And here are additional characters to get sufficient long lines.");
//    QCOMPARE(mMapper->topChunk(), 0);
//}

//void TestMemoryMapper::testReadChunk1()
//{
//    // ---------- check reading chunk 1
//    mMapper->setVisibleTopLine(0.005);
//    QCOMPARE(mMapper->lines(0,1), "This is line   218 of the testfile. And here are additional characters to get sufficient long lines.");
//    QCOMPARE(mMapper->topChunk(), 1);
//    QCOMPARE(mMapper->absTopLine(), 217);
//}

//void TestMemoryMapper::testMoveBackAChunk()
//{
//    // ---------- check moving back across the chunk border
//    mMapper->setVisibleTopLine(0.005);
//    mMapper->moveVisibleTopLine(-67);
//    QCOMPARE(mMapper->absTopLine(), 150);
//    QCOMPARE(mMapper->topChunk(), 0);
//}

//void TestMemoryMapper::testFetchBeyondChunk()
//{
//    // ---------- check reading multiple chunks in a row
//    mMapper->moveVisibleTopLine(193);
//    // ---------- check fetching lines across the chunk border
//    QCOMPARE(mMapper->absTopLine(), 160);
//    QCOMPARE(mMapper->topChunk(), 0);
//    QCOMPARE(mMapper->lines(0,1), "This is line   161 of the testfile. And here are additional characters to get sufficient long lines.");
//    QCOMPARE(mMapper->lines(1,1), "This is line   162 of the testfile. And here are additional characters to get sufficient long lines.");
//    QCOMPARE(mMapper->lines(2,1), "This is line   163 of the testfile. And here are additional characters to get sufficient long lines.");
//    QCOMPARE(mMapper->lines(3,1), "This is line   164 of the testfile. And here are additional characters to get sufficient long lines.");
//    QCOMPARE(mMapper->lines(103,1), "This is line   264 of the testfile. And here are additional characters to get sufficient long lines.");
//    QCOMPARE(mMapper->lines(203,1), "This is line   364 of the testfile. And here are additional characters to get sufficient long lines.");
//    QCOMPARE(mMapper->topChunk(), 2);
//    QCOMPARE(mMapper->absTopLine(), 160);
//    QCOMPARE(mMapper->topChunk(), 0);
//}

//void TestMemoryMapper::testReadLines()
//{
//    // ---------- check read lines
//    mMapper->setVisibleTopLine(0.0104);
//    QCOMPARE(mMapper->topChunk(), 3);
//    QVERIFY(mMapper->absTopLine() < 0);
//    mMapper->moveVisibleTopLine(-45);
//    QString line1 = mMapper->lines(0,1);
//    mMapper->moveVisibleTopLine(-1);
//    QCOMPARE(mMapper->lines(1,1), line1);
//    mMapper->moveVisibleTopLine(-1);
//    QStringList lines = mMapper->lines(0, 4).split("\n");
//    QCOMPARE(lines.size(), 4);
//    QCOMPARE(lines.at(2), line1);
//}


//void TestMemoryMapper::testUpdateLineCounting()
//{
//    // ---------- check updating of lineCounting
//    mMapper->setVisibleTopLine(0.004);
//    QCOMPARE(mMapper->topChunk(), 1);
//    QCOMPARE(mMapper->knownLineNrs(), 324);

//    mMapper->setVisibleTopLine(0.015);
//    // NOT all chunks are known from start of file, so the line number just estimated.
//    QCOMPARE(mMapper->topChunk(), 4);
//    QVERIFY(mMapper->absTopLine() < 0);

//    mMapper->setVisibleTopLine(0.012);
//    QCOMPARE(mMapper->topChunk(), 3);
//    QCOMPARE(mMapper->knownLineNrs(), 324);

//    mMapper->setVisibleTopLine(0.01);
//    QCOMPARE(mMapper->topChunk(), 2);
//    // all chunks are known from start of file, so the line number is known, too.
//    mMapper->setVisibleTopLine(0.015);
//    QCOMPARE(mMapper->topChunk(), 4);
//    QCOMPARE(mMapper->absTopLine(), 717);
//    QCOMPARE(mMapper->knownLineNrs(), 811);

//    mMapper->setVisibleTopLine(0);
//    qDebug() << mMapper->lines(1,1);
//    qDebug() << mMapper->lines(49999, 1);
//    qDebug() << mMapper->lines(50000, 1);
//    mMapper->setVisibleTopLine(20000);
//    mMapper->setVisibleTopLine(1.0);
//    qDebug() << mMapper->lines(0,1);
//}

//void TestMemoryMapper::testPeekChunkLineNrs()
//{
//    // ---------- check peek chunk line numbers
//    mMapper->setVisibleTopLine(0.015);
//    QVERIFY(mMapper->absTopLine() < 0);
//    mMapper->peekChunksForLineNrs();
//    QCOMPARE(mMapper->absTopLine(), 717);
//}

//void TestMemoryMapper::testLineNrEstimation()
//{
//    // ---------- check line number esimation
//    mMapper->setVisibleTopLine(0.04);
//    QCOMPARE(mMapper->absTopLine(), -1971);
//    mMapper->peekChunksForLineNrs();
//    for (int i = 1; i < 11; ++i) {
//        if (i==1) QCOMPARE(mMapper->absTopLine(), -1971);
//        mMapper->peekChunksForLineNrs();
//    }
//    QCOMPARE(mMapper->absTopLine(), 1967);
//}

//void TestMemoryMapper::testPosAndAnchor()
//{
//    // ---------- check position and anchor handling
//    QPoint pos;
//    QPoint anc;
//    mMapper->setVisibleTopLine(0.00859); // 40000
//    mMapper->setPosRelative(1, 10);
//    pos = mMapper->position();
//    anc = mMapper->anchor();
//    QVERIFY(pos.y() < 0);
//    QCOMPARE(pos.x(), 10);
//    QVERIFY(anc.y() < 0);
//    QCOMPARE(anc.x(), 10);
//    mMapper->peekChunksForLineNrs();
//    pos = mMapper->position();
//    anc = mMapper->anchor();
//    QCOMPARE(pos.y(), 397);

//    mMapper->setPosRelative(4, 2, QTextCursor::KeepAnchor);
//    pos = mMapper->position();
//    anc = mMapper->anchor();
//    QCOMPARE(pos.y(), 400);
//    QCOMPARE(pos.x(), 2);
//    QCOMPARE(anc.y(), 397);
//    QCOMPARE(anc.x(), 10);

//    mMapper->peekChunksForLineNrs();
//    mMapper->peekChunksForLineNrs();
//    mMapper->peekChunksForLineNrs();
//    mMapper->peekChunksForLineNrs();
//    mMapper->setVisibleTopLine(1620);
//    mMapper->setPosRelative(0, 0);
//    mMapper->setPosRelative(310, 1, QTextCursor::KeepAnchor);
//    QCOMPARE(mMapper->anchor(true).y(), 0);
//    QCOMPARE(mMapper->position(true).y(), 99); // cropped to buffer
//    mMapper->setVisibleTopLine(1920);
//    QCOMPARE(mMapper->position(true).y(), 10);
//}


QTEST_MAIN(TestMemoryMapper)
