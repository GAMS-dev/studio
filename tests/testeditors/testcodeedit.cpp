/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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

QTEST_MAIN(TestCodeEdit)
