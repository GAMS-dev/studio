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
#include "testfilemapper.h"
#include "logger.h"

#include <QtGlobal>
#include <QStandardPaths>
#include <QClipboard>
#include <QApplication>
#include "fastfilemapper.h"

using gams::studio::FastFileMapper;
const QString testFileName("testtextmapper.tmp");

void TestFileMapper::initTestCase()
{
    mCurrentPath = QDir::current();
    QFile file(mCurrentPath.absoluteFilePath(testFileName));
    QString message("Error on opening test file '%1'.");
    bool opened = file.open(QIODevice::WriteOnly | QIODevice::Text);
    if (!opened) {
        qDebug() << file.errorString();
    }
    QVERIFY2(opened, message.arg(file.fileName()).toLatin1());

    // -------------- create a test file
    QTextStream stream(&file);

    int maxLine = 5000;
    int nrChars = QString::number(maxLine).length();
    stream << "This is line " << QString(nrChars-1, ' ').toLatin1() << 1
           << " of the testfile. And here are additional characters to get sufficient long lines\n";
    for (int line = 1; line < maxLine-1; ++line) {
        int nrCurrent = QString::number(line+1).length();
        stream << "This is line "<< QString(nrChars-nrCurrent, ' ').toLatin1() << line+1
               << " of the testfile. And here are additional characters to get sufficient long lines\n";
    }
    stream.flush();
    DEB() << "Line " << maxLine << " starts at " << stream.pos();
    stream << "This is line " << maxLine << " of the testfile - the last numerated.";
    stream << "Some characters 'äüößÄÜÖê€µ@' to test the codec. END";
    stream.flush();
    DEB() << "Stream ends at " << stream.pos();
    stream.flush();
    file.close();
}
void TestFileMapper::cleanupTestCase()
{
    QFile file(mCurrentPath.absoluteFilePath(testFileName));
    file.remove();
}


void TestFileMapper::init()
{
    mMapper = new FastFileMapper();
    mMapper->setEncoding("UTF-8");
    mMapper->setOverscanLines(10);
    QVERIFY2(mMapper->openFile(mCurrentPath.absoluteFilePath(testFileName), true),
             "TextMapper: Error on opening test file.");
    mMapper->setVisibleLineCount(20);
    QCOMPARE(mMapper->visibleLineCount(), 20);
    mMapper->waitForCountThread();
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
    DEB() << "size " << size;
    QCOMPARE(mMapper->size(), size);
    QCOMPARE(mMapper->lineCount(), 5000);
#ifdef _WIN64
    // for 2-byte linebreaks \r\n
    qint64 lineLen = 100;
#else
    // for 1-byte linebreaks \n
    qint64 lineLen = 99;
#endif
    qint64 calculatedSize = (5000 * lineLen) + 20;
    QCOMPARE(mMapper->checkField(FastFileMapper::fVirtualLastLineEnd), calculatedSize);

    QCOMPARE(mMapper->visibleTopLine(), 0);
    mMapper->lines(0, 20);
    QCOMPARE(mMapper->checkField(FastFileMapper::fCacheFirst), 0);
    QCOMPARE(mMapper->checkField(FastFileMapper::fCacheLast), 40);
    mMapper->setVisibleTopLine(4980);
    QString lines = mMapper->lines(4980 - mMapper->visibleTopLine(), 20);
    DEB() << "TAIL: \"" << lines.mid(lines.size() - 3) << "\"";
    QCOMPARE(lines.mid(lines.size() - 3), QString("END"));
    QCOMPARE(mMapper->checkField(FastFileMapper::fCacheFirst), 4970);
    QCOMPARE(mMapper->checkField(FastFileMapper::fCacheLast), 5000);
}


void TestFileMapper::testPosAndAnchor()
{
    // ---------- check position and anchor handling
    QPoint pos;
    QPoint anc;
    mMapper->setVisibleTopLine(0.08); // 400
    mMapper->setPosRelative(0, 10);
    pos = mMapper->position();
    anc = mMapper->anchor();
    QCOMPARE(pos.y(), 400);
    QCOMPARE(pos.x(), 10);
    QCOMPARE(anc.y(), 400);
    QCOMPARE(anc.x(), 10);
    mMapper->setPosRelative(-3, 10);
    pos = mMapper->position();
    anc = mMapper->anchor();
    QCOMPARE(pos.y(), 397);

    mMapper->setPosRelative(3, 2, QTextCursor::KeepAnchor);
    pos = mMapper->position();
    anc = mMapper->anchor();
    QCOMPARE(pos.y(), 403);
    QCOMPARE(pos.x(), 2);
    QCOMPARE(anc.y(), 397);
    QCOMPARE(anc.x(), 10);

    mMapper->setVisibleTopLine(1620);
    mMapper->setPosRelative(0, 0);
    mMapper->setPosRelative(310, 1, QTextCursor::KeepAnchor);
    QCOMPARE(mMapper->anchor(true).y(), 0);
    QCOMPARE(mMapper->position(true).y(), FastFileMapper::cursorBeyondEnd);
    mMapper->setVisibleTopLine(1920);
    QCOMPARE(mMapper->position(true).y(), 10);

    mMapper->setVisibleTopLine(4980);
    QString lines = mMapper->lines(4980 - mMapper->visibleTopLine(), 20);
    DEB() << "TAIL:\n" << lines.mid(lines.size() - 207);
    QCOMPARE(mMapper->checkField(FastFileMapper::fCacheFirst), 4970);
    QCOMPARE(mMapper->checkField(FastFileMapper::fCacheLast), 5000);
    mMapper->setPosRelative(18, 0);

#ifdef _WIN64
    // for 2-byte linebreaks \r\n
    qint64 lineLen = 100;
#else
    // for 1-byte linebreaks \n
    qint64 lineLen = 99;
#endif
    qint64 offset = 4998 * lineLen;
    DEB() << "pos " << offset;
    QCOMPARE(mMapper->checkField(FastFileMapper::fVirtualLastLineEnd), (5000 * lineLen) + 20);
    QCOMPARE(mMapper->checkField(FastFileMapper::fPosLineStartInFile), offset);
    mMapper->setPosRelative(19, 0);
    QCOMPARE(mMapper->checkField(FastFileMapper::fPosLineStartInFile), offset + lineLen);
    mMapper->setPosRelative(20, 0); // when set beyond EOF, it's set to EOF
    QCOMPARE(mMapper->checkField(FastFileMapper::fPosLineStartInFile), offset + lineLen);
}


QTEST_MAIN(TestFileMapper)
