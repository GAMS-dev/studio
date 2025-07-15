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
#include "testblockcode.h"
#include "syntax/syntaxhighlighter.h"
#include "syntax/syntaxformats.h"
#include "logger.h"

#include <QtGlobal>

using gams::studio::syntax::BlockCode;


void TestBlockCode::testFile()
{
    BlockCode bc(13);
    QCOMPARE(bc.code(), 13);
    QCOMPARE(bc.depth(), 0);
    QCOMPARE(bc.parser(), 0);
    bc = -5;
    QCOMPARE(bc.code(), -1);
    QCOMPARE(bc.depth(), 0);
    QCOMPARE(bc.parser(), 0);
    bc = 13;
    bc.setDepth(2);
    bc.setParser(3);
    QCOMPARE(bc.depth(), 2);
    QCOMPARE(bc.parser(), 3);

    QCOMPARE(bc.setFlavor(511), true);
    QCOMPARE(bc.flavor(), 511);
    QCOMPARE(bc.setFlavor(512), false);
    QCOMPARE(bc.flavor(), 511);
    QCOMPARE(bc.setFlavor(-1), false);
    QCOMPARE(bc.flavor(), 0);

    QCOMPARE(bc.setDepth(1023), true);
    QCOMPARE(bc.depth(), 1023);
    QCOMPARE(bc.setDepth(1024), false);
    QCOMPARE(bc.depth(), 1023);
    QCOMPARE(bc.setDepth(-1), false);
    QCOMPARE(bc.depth(), 0);

    QCOMPARE(bc.setParser(7), true);
    QCOMPARE(bc.parser(), 7);
    QCOMPARE(bc.setParser(8), false);
    QCOMPARE(bc.parser(), 7);
    QCOMPARE(bc.setParser(-1), false);
    QCOMPARE(bc.parser(), 0);

    QCOMPARE(bc.maxValue(), 0x7fffffff);
}

QTEST_MAIN(TestBlockCode)
