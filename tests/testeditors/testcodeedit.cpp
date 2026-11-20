/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "testcodeedit.h"
#include "editors/editorhelper.h"
#include "editors/codeedit.h"

using namespace gams::studio;

void TestCodeEdit::test_nextWord()
{
    //              0         1         2
    //              0123456789012345678901234
    QString text = "This is our 1 sample text";
    int pos = 0;
    int offset = 0;

    EditorHelper::nextWord(offset, pos, text);
    QCOMPARE(pos, 5);
    EditorHelper::nextWord(offset, pos, text);
    QCOMPARE(pos, 8);
    EditorHelper::nextWord(offset, pos, text);
    QCOMPARE(pos, 12);
    EditorHelper::nextWord(offset, pos, text);
    QCOMPARE(pos, 14);
    EditorHelper::nextWord(offset, pos, text);
    QCOMPARE(pos, 21);
    EditorHelper::nextWord(offset, pos, text);
    QCOMPARE(pos, 25);  // the nextWord function is required to return text.length()+1 when
                        // reaching the end of a given text

    QVERIFY(true);
}

void TestCodeEdit::test_prevWord()
{
    //              0         1         2
    //              0123456789012345678901234
    QString text = "This is our 1 sample text";
    int pos = 0;
    int offset = 0;

    EditorHelper::prevWord(offset, pos, text);
    QCOMPARE(pos, 0); // nothing should happen

    pos = 24; // set to end

    EditorHelper::prevWord(offset, pos, text);
    QCOMPARE(pos, 21);
    EditorHelper::prevWord(offset, pos, text);
    QCOMPARE(pos, 14);
    EditorHelper::prevWord(offset, pos, text);
    QCOMPARE(pos, 12);
    EditorHelper::prevWord(offset, pos, text);
    QCOMPARE(pos, 8);
    EditorHelper::prevWord(offset, pos, text);
    QCOMPARE(pos, 5);
    EditorHelper::prevWord(offset, pos, text);
    QCOMPARE(pos, 0);
}

void TestCodeEdit::testSecondsToString()
{
    QString res;
    // test seconds
    res = EditorHelper::formatTime(9.123, true);
    QCOMPARE(res, "9.123 s");
    res = EditorHelper::formatTime(10., true);
    QCOMPARE(res, "10.00 s");
    res = EditorHelper::formatTime(11.123);
    QCOMPARE(res, "11.123 s");

    // test minutes
    res = EditorHelper::formatTime(61.12, true);
    QCOMPARE(res, "1:01 m");
    res = EditorHelper::formatTime(60.12);
    QCOMPARE(res, "1:00.120 m");
    res = EditorHelper::formatTime(3541.12, true);
    QCOMPARE(res, "59:01 m");
    res = EditorHelper::formatTime(3541.12);
    QCOMPARE(res, "59:01.120 m");

    // test hours
    res = EditorHelper::formatTime(3630.123, true);
    QCOMPARE(res, "1:01 h");
    res = EditorHelper::formatTime(3630.123);
    QCOMPARE(res, "1:00:30.123 h");
}

void TestCodeEdit::testSecondsToString2()
{
    QString res;
    int unit = 0;
    res = EditorHelper::formatTime2(9.123, unit, true);
    QCOMPARE(res, "9.123");
    res = EditorHelper::formatTime2(10.123, unit, true);
    QCOMPARE(res, "10.12");
    res = EditorHelper::formatTime2(10.123, unit);
    QCOMPARE(res, "10.123");
    unit = 1;
    res = EditorHelper::formatTime2(10.523, unit, true);
    QCOMPARE(res, "0:11");
    res = EditorHelper::formatTime2(10.523, unit);
    QCOMPARE(res, "0:10.523");
    unit = 2;
    res = EditorHelper::formatTime2(10.523, unit, true);
    QCOMPARE(res, "0:00");
    res = EditorHelper::formatTime2(10.523, unit);
    QCOMPARE(res, "0:00:10.523");
    unit = 3;
    res = EditorHelper::formatTime2(10.523, unit, true);
    QCOMPARE(res, "0.00");
    res = EditorHelper::formatTime2(10.523, unit);
    QCOMPARE(res, "0 0:00:10");
}

void TestCodeEdit::testBytesToString()
{
    QString res;
    // test bytes & kbytes
    res = EditorHelper::formatMemory(123, true);
    QCOMPARE(res, "  123 B ");
    res = EditorHelper::formatMemory(3123, true);
    QCOMPARE(res, " 3.12 KB");
    res = EditorHelper::formatMemory(3123);
    QCOMPARE(res, "  3.123 KB");
    res = EditorHelper::formatMemory(23123, true);
    QCOMPARE(res, "23.12 KB");
    res = EditorHelper::formatMemory(23123);
    QCOMPARE(res, " 23.123 KB");
    res = EditorHelper::formatMemory(123123, true);
    QCOMPARE(res, "123.1 KB");
    res = EditorHelper::formatMemory(123123);
    QCOMPARE(res, "123.123 KB");

    // test mbytes
    res = EditorHelper::formatMemory(23123123, true);
    QCOMPARE(res, "23.12 MB");
    res = EditorHelper::formatMemory(23123123);
    QCOMPARE(res, " 23.123 MB");
    res = EditorHelper::formatMemory(123123123, true);
    QCOMPARE(res, "123.1 MB");
    res = EditorHelper::formatMemory(123123123);
    QCOMPARE(res, "123.123 MB");

    // test gbytes
    res = EditorHelper::formatMemory(3123123123, true);
    QCOMPARE(res, " 3.12 GB");
    res = EditorHelper::formatMemory(3123123123);
    QCOMPARE(res, "  3.123 GB");
    res = EditorHelper::formatMemory(23123123123, true);
    QCOMPARE(res, "23.12 GB");
    res = EditorHelper::formatMemory(23123123123);
    QCOMPARE(res, " 23.123 GB");
    res = EditorHelper::formatMemory(123123123123, true);
    QCOMPARE(res, "123.1 GB");
    res = EditorHelper::formatMemory(123123123123);
    QCOMPARE(res, "123.123 GB");

    // test tbytes
    res = EditorHelper::formatMemory(3123123123123, true);
    QCOMPARE(res, " 3.12 TB");
    res = EditorHelper::formatMemory(3123123123123);
    QCOMPARE(res, "  3.123 TB");
    res = EditorHelper::formatMemory(23123123123123, true);
    QCOMPARE(res, "23.12 TB");
    res = EditorHelper::formatMemory(23123123123123);
    QCOMPARE(res, " 23.123 TB");
    res = EditorHelper::formatMemory(123123123123123, true);
    QCOMPARE(res, "123.1 TB");
    res = EditorHelper::formatMemory(123123123123123);
    QCOMPARE(res, "123.123 TB");

}

void TestCodeEdit::testBytesToString2()
{
    QString res;
    int unit = 0;
    res = EditorHelper::formatMemory2(123, unit, true);
    QCOMPARE(res, "123");
    res = EditorHelper::formatMemory2(3123, unit, true);
    QCOMPARE(res, "3123");
    res = EditorHelper::formatMemory2(23123, unit, true);
    QCOMPARE(res, "23123");

    unit = 1;
    res = EditorHelper::formatMemory2(123, unit, true);
    QCOMPARE(res, "0.123");
    res = EditorHelper::formatMemory2(3123, unit, true);
    QCOMPARE(res, "3.123");
    res = EditorHelper::formatMemory2(23123, unit, true);
    QCOMPARE(res, "23.12");
    res = EditorHelper::formatMemory2(123123, unit, true);
    QCOMPARE(res, "123.1");
    res = EditorHelper::formatMemory2(3123123, unit, true);
    QCOMPARE(res, " 3123");

    res = EditorHelper::formatMemory2(123, unit);
    QCOMPARE(res, "0.123");
    res = EditorHelper::formatMemory2(3123, unit);
    QCOMPARE(res, "3.123");
    res = EditorHelper::formatMemory2(23123, unit);
    QCOMPARE(res, "23.123");
    res = EditorHelper::formatMemory2(123123, unit);
    QCOMPARE(res, "123.123");
    res = EditorHelper::formatMemory2(3123123, unit);
    QCOMPARE(res, "3123.123");

    unit = 2;
    res = EditorHelper::formatMemory2(123123, unit, true);
    QCOMPARE(res, "0.123");
    res = EditorHelper::formatMemory2(3123123, unit, true);
    QCOMPARE(res, "3.123");
    res = EditorHelper::formatMemory2(23123123, unit, true);
    QCOMPARE(res, "23.12");
    res = EditorHelper::formatMemory2(123123123, unit, true);
    QCOMPARE(res, "123.1");
    res = EditorHelper::formatMemory2(3123123123, unit, true);
    QCOMPARE(res, " 3123");

    res = EditorHelper::formatMemory2(123123, unit);
    QCOMPARE(res, "0.123");
    res = EditorHelper::formatMemory2(123123123, unit);
    QCOMPARE(res, "123.123");
    res = EditorHelper::formatMemory2(3123123123, unit);
    QCOMPARE(res, "3123.123");

    unit = 3;
    res = EditorHelper::formatMemory2(123123123, unit, true);
    QCOMPARE(res, "0.123");
    res = EditorHelper::formatMemory2(123123123123, unit, true);
    QCOMPARE(res, "123.1");
    res = EditorHelper::formatMemory2(3123123123123, unit, true);
    QCOMPARE(res, " 3123");

    res = EditorHelper::formatMemory2(123123123, unit);
    QCOMPARE(res, "0.123");
    res = EditorHelper::formatMemory2(123123123123, unit);
    QCOMPARE(res, "123.123");
    res = EditorHelper::formatMemory2(3123123123213, unit);
    QCOMPARE(res, "3123.123");

    unit = 4;
    res = EditorHelper::formatMemory2(123123123123, unit, true);
    QCOMPARE(res, "0.123");
    res = EditorHelper::formatMemory2(123123123123123, unit, true);
    QCOMPARE(res, "123.1");
    res = EditorHelper::formatMemory2(3123123123123123, unit, true);
    QCOMPARE(res, " 3123");

    res = EditorHelper::formatMemory2(123123123123, unit);
    QCOMPARE(res, "0.123");
    res = EditorHelper::formatMemory2(123123123123123, unit);
    QCOMPARE(res, "123.123");
    res = EditorHelper::formatMemory2(3123123123213123, unit);
    QCOMPARE(res, "3123.123");
}

void TestCodeEdit::testSplitLines()
{
    QList<QStringView> lines;
    QList<int> lengths;
    // LF and CRLF mixed, the '\r' must not count to the line length
    QString text("ab\r\ncdef\n\nxyz\r");
    QCOMPARE(EditorHelper::splitLines(text, lines, &lengths), 4);
    QCOMPARE(lines.size(), 4);
    QCOMPARE(lines.at(0).toString(), QString("ab"));
    QCOMPARE(lines.at(1).toString(), QString("cdef"));
    QCOMPARE(lines.at(2).toString(), QString(""));
    QCOMPARE(lines.at(3).toString(), QString("xyz"));
    QCOMPARE(lengths, QList<int>({2, 4, 0, 3}));

    text = "single line";
    QCOMPARE(EditorHelper::splitLines(text, lines), 11);
    QCOMPARE(lines.size(), 1);

    text = "";
    QCOMPARE(EditorHelper::splitLines(text, lines, &lengths), 0);
    QCOMPARE(lines.size(), 1);
    QCOMPARE(lengths, QList<int>({0}));
}

void TestCodeEdit::testCutLines()
{
    QList<QStringView> lines;
    QString text("0123456789\r\nabc\n\nABCDEFGHIJKLMNOP");
    EditorHelper::splitLines(text, lines);
    QCOMPARE(EditorHelper::cutLines(lines, 2, 5), "23456\nc\n\nCDEFG");
    QCOMPARE(EditorHelper::cutLines(lines, 0, 100), "0123456789\nabc\n\nABCDEFGHIJKLMNOP");
    // a slice right of all lines contains only the line breaks
    QCOMPARE(EditorHelper::cutLines(lines, 20, 5), "\n\n\n");
}

void TestCodeEdit::testToDocColumn()
{
    // document starts at column 5, the document line has 20 characters
    QCOMPARE(EditorHelper::toDocColumn(10, 5, 20), 5);
    QCOMPARE(EditorHelper::toDocColumn(5, 5, 20), 0);
    QCOMPARE(EditorHelper::toDocColumn(3, 5, 20), 0);   // left of the document
    QCOMPARE(EditorHelper::toDocColumn(30, 5, 20), 20); // behind the end of the document line
    QCOMPARE(EditorHelper::toDocColumn(7, 5, 0), 0);    // empty document line (line shorter than the start)
    QCOMPARE(EditorHelper::toDocColumn(5, 0, -1), 0);
}

void TestCodeEdit::testIsInColumnBuffer()
{
    // the document contains the columns 10 to 39, 20 columns are visible
    QVERIFY(EditorHelper::isInColumnBuffer(10, 20, 10, 30));
    QVERIFY(EditorHelper::isInColumnBuffer(20, 20, 10, 30));
    QVERIFY(!EditorHelper::isInColumnBuffer(21, 20, 10, 30));
    QVERIFY(!EditorHelper::isInColumnBuffer(9, 20, 10, 30));
    // not built yet
    QVERIFY(!EditorHelper::isInColumnBuffer(0, 20, -1, -1));
}

void TestCodeEdit::testScrollByPixels()
{
    qreal remain = 0.0;
    // within a unit
    QCOMPARE(EditorHelper::scrollByPixels(10, remain, 3.5, 7.0, 100), 10);
    QCOMPARE(remain, 3.5);
    // into the next unit
    QCOMPARE(EditorHelper::scrollByPixels(10, remain, 4.0, 7.0, 100), 11);
    QCOMPARE(remain, 0.5);
    // back into the previous unit
    QCOMPARE(EditorHelper::scrollByPixels(11, remain, -1.0, 7.0, 100), 10);
    QCOMPARE(remain, 6.5);
    // small moves back and forth don't change the unit
    QCOMPARE(EditorHelper::scrollByPixels(10, remain, 0.25, 7.0, 100), 10);
    QCOMPARE(EditorHelper::scrollByPixels(10, remain, -0.25, 7.0, 100), 10);
    QCOMPARE(remain, 6.5);

    // limited at the start
    remain = 2.0;
    QCOMPARE(EditorHelper::scrollByPixels(0, remain, -5.0, 7.0, 100), 0);
    QCOMPARE(remain, 0.0);
    // limited at the end, no partial unit beyond the maximum
    remain = 0.0;
    QCOMPARE(EditorHelper::scrollByPixels(99, remain, 3.0, 7.0, 100), 99);
    QCOMPARE(remain, 3.0);
    QCOMPARE(EditorHelper::scrollByPixels(99, remain, 10.0, 7.0, 100), 100);
    QCOMPARE(remain, 0.0);
    QCOMPARE(EditorHelper::scrollByPixels(100, remain, 1.0, 7.0, 100), 100);
    QCOMPARE(remain, 0.0);
    // nothing to scroll
    QCOMPARE(EditorHelper::scrollByPixels(0, remain, 5.0, 7.0, 0), 0);
    QCOMPARE(remain, 0.0);
}

void TestCodeEdit::testScrollByPixelsSmallSteps()
{
    // a slow touchpad movement: many small steps add up exactly
    qreal remain = 0.0;
    int pos = 0;
    for (int i = 0; i < 1000; ++i)
        pos = EditorHelper::scrollByPixels(pos, remain, 0.1, 7.2, 1000);
    QCOMPARE(pos, 13); // 100 pixels = 13 * 7.2 + 6.4
    QVERIFY(qAbs(remain - 6.4) < 1e-6);
    for (int i = 0; i < 1000; ++i)
        pos = EditorHelper::scrollByPixels(pos, remain, -0.1, 7.2, 1000);
    QCOMPARE(pos, 0);
    QVERIFY(remain < 1e-6);
}

void TestCodeEdit::testWheelAxisTouchpad()
{
    int lock = 0;
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::ScrollBegin, QPoint(0, 0)), 0);
    // a horizontal swipe keeps horizontal, even if the vertical movement dominates single events
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::ScrollUpdate, QPoint(10, 2)), int(Qt::Horizontal));
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::ScrollUpdate, QPoint(1, 3)), int(Qt::Horizontal));
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::ScrollUpdate, QPoint(0, 1)), int(Qt::Horizontal));
    // the momentum after releasing the touchpad too
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::ScrollMomentum, QPoint(1, 2)), int(Qt::Horizontal));
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::ScrollMomentum, QPoint(0, 1)), int(Qt::Horizontal));
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::ScrollEnd, QPoint(0, 0)), int(Qt::Horizontal));
    QCOMPARE(lock, 0);

    // the next gesture decides again
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::ScrollBegin, QPoint(0, 0)), 0);
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::ScrollUpdate, QPoint(1, 10)), int(Qt::Vertical));
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::ScrollUpdate, QPoint(5, 1)), int(Qt::Vertical));
}

void TestCodeEdit::testWheelAxisMouseWheel()
{
    // a mouse wheel has no phase, each event decides on its own
    int lock = 0;
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::NoScrollPhase, QPoint(0, 30)), int(Qt::Vertical));
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::NoScrollPhase, QPoint(30, 0)), int(Qt::Horizontal));
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::NoScrollPhase, QPoint(0, -30)), int(Qt::Vertical));
    QCOMPARE(EditorHelper::wheelAxis(lock, Qt::NoScrollPhase, QPoint(0, 0)), 0);
}

QTEST_MAIN(TestCodeEdit)
