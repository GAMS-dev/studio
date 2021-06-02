/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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

void TestCompleter::initTestCase()
{
    mCompleter = new CodeCompleter(nullptr);
    connect(mCompleter, &CodeCompleter::scanSyntax, &mSynSim, &SyntaxSimulator::scanSyntax);
}

void TestCompleter::testDco()
{
    QString descript("Filter: %1 [%2]");

    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::Standard, 0);
    mSynSim.addBlockSyntax(3, SyntaxKind::Put, 0);
    mSynSim.addBlockSyntax(4, SyntaxKind::PutFormula, 0);
    mSynSim.addBlockSyntax(23, SyntaxKind::String, 0);
    mSynSim.addBlockSyntax(24, SyntaxKind::Semicolon, 0);
    mCompleter->updateFilter(3, "put 'abc %system.Date%';");
    QVERIFY2(mCompleter->typeFilter() == cc_Start, descript.arg(QString::number(mCompleter->typeFilter(), 16),
                                                               mCompleter->splitTypes().join(",")).toLatin1());

    mCompleter->updateFilter(6, "put 'abc %system.Date%';");
    QVERIFY2(mCompleter->typeFilter() == cc_Start, descript.arg(QString::number(mCompleter->typeFilter(), 16),
                                                               mCompleter->splitTypes().join(",")).toLatin1());
}



QTEST_MAIN(TestCompleter)
