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
#include "testgamslicenseinfo.h"
#include "gamslicenseinfo.h"
#include "commonpaths.h"

#include <QDebug>

using namespace gams::studio;
using namespace gams::studio::support;

void TestGamsLicenseInfo::initTestCase()
{
    CommonPaths::setSystemDir();
}

void TestGamsLicenseInfo::testGamsLicenseInfo()
{
    GamsLicenseInfo gamsLicenseInfo;
}

void TestGamsLicenseInfo::testSolvers()
{
    GamsLicenseInfo gamsLicenseInfo;
    auto result = gamsLicenseInfo.solvers();
    QVERIFY(result != 0);
}

void TestGamsLicenseInfo::testSolverName()
{
//    GamsLicenseInfo gamsLicenseInfo;
//    auto count = gamsLicenseInfo.solvers();
//    if (!count)
//        QVERIFY2(false, "The number of solver shall not be 0.");
//    auto result = gamsLicenseInfo.solverName(count);
//    QVERIFY(!result.isEmpty());
}

void TestGamsLicenseInfo::testSolverNameZeroIndex()
{
//    GamsLicenseInfo gamsLicenseInfo;
//    if (!gamsLicenseInfo.solvers())
//        QVERIFY2(false, "The number of solver shall not be 0.");
//    auto result = gamsLicenseInfo.solverName(0);
//    QVERIFY(result.isEmpty());
}

void TestGamsLicenseInfo::testSolverNameNegativeIndex()
{
//    GamsLicenseInfo gamsLicenseInfo;
//    auto result = gamsLicenseInfo.solverName(-1);
//    QVERIFY(result.isEmpty());
}

void TestGamsLicenseInfo::testSolverNameOutOfRange()
{
//    GamsLicenseInfo gamsLicenseInfo;
//    auto count = gamsLicenseInfo.solvers() + 1;
//    auto result = gamsLicenseInfo.solverName(count);
//    QVERIFY(result.isEmpty());
}

//void TestGamsLicenseInfo::testSolverInfo()
//{

//}

void TestGamsLicenseInfo::testModelTypeNames()
{
    GamsLicenseInfo gamsLicenseInfo;
    auto modelTypeNames = gamsLicenseInfo.modelTypeNames();
    QCOMPARE(modelTypeNames.size(), 15);
//    QVERIFY(modelTypeNames.contains("lp",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("mip",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("rmip",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("nlp",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("mcp",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("mpec",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("rmpec",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("cns",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("dnlp",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("rminlp",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("minlp",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("qcp",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("miqcp",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("rmiqcp",Qt::CaseInsensitive));
//    QVERIFY(modelTypeNames.contains("emp",Qt::CaseInsensitive));
}

QTEST_MAIN(TestGamsLicenseInfo)
