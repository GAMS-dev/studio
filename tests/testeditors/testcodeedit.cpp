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
#include "testcodeedit.h"
#include "editors/editorhelper.h"

using namespace gams::studio;

void TestCodeEdit::test_case1()
{
    //              0         1         2
    //              0123456789012345678901234
    QString text = "This is our 1 sample text";
    int pos = 0;
    int offset = 0;

    EditorHelper::nextWord(offset, pos, text);
    QCOMPARE(pos, 4);


    QVERIFY(true);
}

QTEST_MAIN(TestCodeEdit)
