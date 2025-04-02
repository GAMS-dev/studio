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
#include "testcompleter.h"
#include "editors/codecompleter.h"

using namespace gams::studio;
using namespace gams::studio::syntax;

QByteArray TestCompleter::describe(int receive, int expect, QStringList types)
{
    QString descript("Filter: %1 != %2  -> diff: %3\n[%4]");
    return descript.arg(QString::number(receive, 16),
                        QString::number(expect, 16),
                        QString::number((expect & ~receive) | (receive & ~expect), 16),
                        types.join(",")).toLatin1();
}

void TestCompleter::initTestCase()
{
    mCompleter = new CodeCompleter(nullptr);
    connect(mCompleter, &CodeCompleter::scanSyntax, &mSynSim, &SyntaxSimulator::scanSyntax);
}

void TestCompleter::testDco()
{
    QString line;
    int expect;

    // ===== TEST: empty line
    line = "";
    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::Standard, 0);

    expect = cc_Start;
    mCompleter->updateFilter( 0, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    // ===== TEST: empty line in non-GAMS blocks
    line = "";
    expect = ccDcoEnd | cc_Start;

    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::CommentBlock, 0);
    mCompleter->updateFilter( 0, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::IgnoredBlock, 5);
    mCompleter->updateFilter( 0, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::EmbeddedBody, 19);
    mCompleter->updateFilter( 0, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::EmbeddedBody, 20);
    mCompleter->updateFilter( 0, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    expect = ccResEnd | cc_Start;
    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::EmbeddedBody, 0);
    mCompleter->updateFilter( 0, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

}

void TestCompleter::testDeclaration()
{
    QString line;
    int expect;

    // ===== TEST: set declaration
    line = "    i 'canning plants' / seattle  san-diego /;";
    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::Declaration, 16);
    mSynSim.addBlockSyntax(4, SyntaxKind::Declaration, 16);
    mSynSim.addBlockSyntax(6, SyntaxKind::Identifier, 16);
    mSynSim.addBlockSyntax(22, SyntaxKind::IdentifierDescription, 16);
    mSynSim.addBlockSyntax(24, SyntaxKind::IdentifierAssignment, 16);
    mSynSim.addBlockSyntax(32, SyntaxKind::AssignmentLabel, 16);
    mSynSim.addBlockSyntax(33, SyntaxKind::IdentifierAssignment, 16);
    mSynSim.addBlockSyntax(44, SyntaxKind::AssignmentLabel, 16);
    mSynSim.addBlockSyntax(46, SyntaxKind::IdentifierAssignmentEnd, 16);
    mSynSim.addBlockSyntax(47, SyntaxKind::Semicolon, 16);

    expect = ccSysSufC | ccCtConst | ccDeclT;
    mCompleter->updateFilter( 3, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    expect = ccSysSufC | ccCtConst;
    mCompleter->updateFilter(21, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    expect = ccSysSufC | ccCtConst | ccSysDat;
    mCompleter->updateFilter(24, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

}

void TestCompleter::testPut()
{
    QString line;
    int expect;

    // ===== TEST: put command with system constant
    line = "put 'abc %system.Date%';";
    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::Standard, 0);
    mSynSim.addBlockSyntax(3, SyntaxKind::Put, 0);
    mSynSim.addBlockSyntax(4, SyntaxKind::PutFormula, 0);
    mSynSim.addBlockSyntax(23, SyntaxKind::String, 0);
    mSynSim.addBlockSyntax(24, SyntaxKind::Semicolon, 0);

    expect = cc_Start;
    mCompleter->updateFilter( 3, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    expect = cc_Res | ccSysSufR | ccSysSufC | ccCtConst;
    mCompleter->updateFilter( 4, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    expect = ccSysSufC | ccCtConst;
    mCompleter->updateFilter( 6, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    expect = cc_Start & ~(ccDcoStrt | ccDcoEnd);
    mCompleter->updateFilter(24, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));
}



QTEST_MAIN(TestCompleter)
