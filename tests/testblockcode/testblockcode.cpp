/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
    BlockCode bc(12);
    QCOMPARE(bc.code(), 12);
    QCOMPARE(bc.kind(), gams::studio::syntax::SyntaxKind::Semicolon);
    QCOMPARE(bc.depth(), 0);
    QCOMPARE(bc.parser(), 0);
    bc = -5;
    QCOMPARE(bc.code(), -1);
    QCOMPARE(bc.depth(), 0);
    QCOMPARE(bc.parser(), 0);
    bc = 12;
    bc.setDepth(2);
    bc.setParser(5);
    QCOMPARE(bc.kind(), gams::studio::syntax::SyntaxKind::Semicolon);
    QCOMPARE(bc.depth(), 2);
    QCOMPARE(bc.parser(), 5);

    QCOMPARE(bc.setFlavor(31), true);
    QCOMPARE(bc.flavor(), 31);
    QCOMPARE(bc.setFlavor(32), false);
    QCOMPARE(bc.flavor(), 31);
    QCOMPARE(bc.setFlavor(-1), false);
    QCOMPARE(bc.flavor(), 0);

    QCOMPARE(bc.setDepth(255), true);
    QCOMPARE(bc.depth(), 255);
    QCOMPARE(bc.setDepth(256), false);
    QCOMPARE(bc.depth(), 255);
    QCOMPARE(bc.setDepth(-1), false);
    QCOMPARE(bc.depth(), 0);

    QCOMPARE(bc.setParser(7), true);
    QCOMPARE(bc.parser(), 7);
    QCOMPARE(bc.setParser(8), false);
    QCOMPARE(bc.parser(), 7);
    QCOMPARE(bc.setParser(-1), false);
    QCOMPARE(bc.parser(), 0);
}

QTEST_MAIN(TestBlockCode)
