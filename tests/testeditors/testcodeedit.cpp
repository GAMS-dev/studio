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
#include "testcodeedit.h"
#include "editors/editorhelper.h"

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

QTEST_MAIN(TestCodeEdit)
