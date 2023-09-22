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
#include "testgamslicenseinfo.h"
#include "gamslicenseinfo.h"
#include "commonpaths.h"
#include "cfgmcc.h"

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

void TestGamsLicenseInfo::testSolverId()
{
    const QString solver = "CPLEX";
    GamsLicenseInfo gamsLicenseInfo;
    auto solverId = gamsLicenseInfo.solverId(solver);
    auto solverName = gamsLicenseInfo.solverName(solverId);
    QCOMPARE(solverName, solver);
}

void TestGamsLicenseInfo::testSolverIdLowerCase()
{
    const QString solver = "CPLEX";
    GamsLicenseInfo gamsLicenseInfo;
    auto solverId = gamsLicenseInfo.solverId(solver.toLower());
    auto solverName = gamsLicenseInfo.solverName(solverId);
    QCOMPARE(solverName, solver);
}

void TestGamsLicenseInfo::testSolverIdMixedCase()
{
    const QString solver = "CpLeX";
    GamsLicenseInfo gamsLicenseInfo;
    auto solverId = gamsLicenseInfo.solverId(solver);
    auto solverName = gamsLicenseInfo.solverName(solverId);
    QVERIFY(!solverName.compare(solver, Qt::CaseInsensitive));
}

void TestGamsLicenseInfo::testSolverIdInvalid()
{
    const QString solver = "LALA";
    GamsLicenseInfo gamsLicenseInfo;
    auto solverId = gamsLicenseInfo.solverId(solver);
    QVERIFY2(!solverId, "The solver Id shall be 0 (UNKOWN) for non existing solvers.");
    auto solverName = gamsLicenseInfo.solverName(solverId);
    QVERIFY2(solverName.isEmpty(), "The solver name shall be if invaled solver Ids are used.");
}

void TestGamsLicenseInfo::testSolverName()
{
    GamsLicenseInfo gamsLicenseInfo;
    auto count = gamsLicenseInfo.solvers();
    QVERIFY2(count, "The number of solvers shall not be 0.");
    auto name = gamsLicenseInfo.solverName(count);
    QVERIFY2(!name.isEmpty(), "There shall be a solver name.");
}

void TestGamsLicenseInfo::testSolverNameZeroIndex()
{
    GamsLicenseInfo gamsLicenseInfo;
    auto result = gamsLicenseInfo.solverName(0);
    QVERIFY2(result.isEmpty(), "There is no solver at postion 0 in GAMS.");
}

void TestGamsLicenseInfo::testSolverNameNegativeIndex()
{
    GamsLicenseInfo gamsLicenseInfo;
    auto result = gamsLicenseInfo.solverName(-1);
    QVERIFY2(result.isEmpty(), "There is no solver at postion 0 in GAMS.");
}

void TestGamsLicenseInfo::testSolverNameOutOfRange()
{
    GamsLicenseInfo gamsLicenseInfo;
    auto count = gamsLicenseInfo.solvers() + 1;
    auto result = gamsLicenseInfo.solverName(count);
    QVERIFY2(result.isEmpty(), "There is no solver at postion 0 in GAMS.");
}

void TestGamsLicenseInfo::testSolverNames()
{
    GamsLicenseInfo gamsLicenseInfo;
    auto count = gamsLicenseInfo.solvers();
    QVERIFY2(count, "The number of solvers shall not be 0.");
    auto result = gamsLicenseInfo.solverNames();
    QVERIFY2(count >= result.size(), "The number of solvers shall be greater or equal to the number of available solvers (some might be hidden).");
    QVERIFY2(!result.isEmpty(), "The result shall not be empty.");
}

void TestGamsLicenseInfo::testModelTypeNames()
{
    GamsLicenseInfo gamsLicenseInfo;
    auto modelTypeNames = gamsLicenseInfo.modelTypeNames();
    QCOMPARE(modelTypeNames.size(), 15);
    QVERIFY(modelTypeNames[cfgProc_lp].contains("lp", Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_mip].contains("mip",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_rmip].contains("rmip",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_nlp].contains("nlp",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_mcp].contains("mcp",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_mpec].contains("mpec",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_rmpec].contains("rmpec",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_cns].contains("cns",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_dnlp].contains("dnlp",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_rminlp].contains("rminlp",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_minlp].contains("minlp",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_qcp].contains("qcp",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_miqcp].contains("miqcp",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_rmiqcp].contains("rmiqcp",Qt::CaseInsensitive));
    QVERIFY(modelTypeNames[cfgProc_emp].contains("emp",Qt::CaseInsensitive));
}

void TestGamsLicenseInfo::testSolverCapability()
{
    GamsLicenseInfo gamsLicenseInfo;
    auto solverId = gamsLicenseInfo.solverId("CPLEX");
    bool capable = gamsLicenseInfo.solverCapability(solverId, cfgProc_lp);
    QVERIFY2(capable, "Ther solver CPLEX shall be LP capable.");
}

void TestGamsLicenseInfo::testSolverCapabilityInvalidSolver()
{
    GamsLicenseInfo gamsLicenseInfo;
    int solvers = gamsLicenseInfo.solvers();
    bool capable = gamsLicenseInfo.solverCapability(solvers+2, cfgProc_lp);
    QVERIFY2(!capable, "There shall be no capability returned.");
}

void TestGamsLicenseInfo::testSolverCapabilityInvalidModelType()
{
    GamsLicenseInfo gamsLicenseInfo;
    int solvers = gamsLicenseInfo.solvers();
    QVERIFY2(solvers > 0, "There shall be at least one solver.");
    bool capable = gamsLicenseInfo.solverCapability(1, cfgProc_nrofmodeltypes+1);
    QVERIFY2(!capable, "There shall be no capability returned.");
}

void TestGamsLicenseInfo::testSolverCapabilityInvalidSolverNegative()
{
    GamsLicenseInfo gamsLicenseInfo;
    bool capable = gamsLicenseInfo.solverCapability(-1, cfgProc_lp);
    QVERIFY2(!capable, "There shall be no capability returned.");
}

void TestGamsLicenseInfo::testSolverCapabilityInvalidModelTypeNegative()
{
    GamsLicenseInfo gamsLicenseInfo;
    int solvers = gamsLicenseInfo.solvers();
    QVERIFY2(solvers > 0, "There shall be at least one solver.");
    bool capable = gamsLicenseInfo.solverCapability(1, -1);
    QVERIFY2(!capable, "There shall be no capability returned.");
}

void TestGamsLicenseInfo::testSolverCapabilityBothInvalid()
{
    GamsLicenseInfo gamsLicenseInfo;
    int solvers = gamsLicenseInfo.solvers();
    bool capable = gamsLicenseInfo.solverCapability(solvers+1, cfgProc_nrofmodeltypes+1);
    QVERIFY2(!capable, "There shall be no capability returned.");
}

void TestGamsLicenseInfo::testSolverCapabilityBothInvalidNegative()
{
    GamsLicenseInfo gamsLicenseInfo;
    bool capable = gamsLicenseInfo.solverCapability(-1, -1);
    QVERIFY2(!capable, "There shall be no capability returned.");
}

void TestGamsLicenseInfo::testSolverLicense()
{
    bool test = true;
    GamsLicenseInfo gamsLicenseInfo;
    const auto solverKeys = gamsLicenseInfo.solverNames().keys();
    QVERIFY(gamsLicenseInfo.solvers() >= solverKeys.size());
    for(const auto solverId : solverKeys) {
        auto solverName = gamsLicenseInfo.solverName(solverId);
        auto result = gamsLicenseInfo.solverLicense(solverName, solverId);
        test = result.contains("Demo") || result.contains("Full") ||
                result.contains("Evaluation") || result.contains("Expired") ||
                result.contains("Community") || result.contains("None");
        if (!test) break;
    }
    QVERIFY(test);
}

void TestGamsLicenseInfo::testIsLicenseValid()
{
    GamsLicenseInfo gamsLicenseInfo;
    QVERIFY(gamsLicenseInfo.isLicenseValid());
}

void TestGamsLicenseInfo::testIsLicenseValidText()
{
    GamsLicenseInfo gamsLicenseInfo;
    QCOMPARE(gamsLicenseInfo.isLicenseValid({"lalala"}), false);
    QStringList license;
    QCOMPARE(gamsLicenseInfo.isLicenseValid(license), false);
    auto dataPaths = gamsLicenseInfo.gamsDataLocations();
    auto licensePath = CommonPaths::gamsLicenseFilePath(dataPaths);
    QFile file(licensePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "ERROR: Could not read file!";
    }
    license = QString(file.readAll()).split('\n');
    file.close();
    // TODO(AF): enable when we know what the CI issue is
    //auto dbgstr = QString("%1 : %2").arg(licensePath, license.join("\n"));
    //QVERIFY2(gamsLicenseInfo.isLicenseValid(license), dbgstr.toStdString().c_str());
}

void TestGamsLicenseInfo::testGamsDataLocations()
{
    auto actual = GamsLicenseInfo().gamsDataLocations();
    QVERIFY(!actual.isEmpty());
}

void TestGamsLicenseInfo::testGamsConfigLocations()
{
    auto actual = GamsLicenseInfo().gamsConfigLocations();
    qDebug() << actual;
    QVERIFY(!actual.isEmpty());
}

void TestGamsLicenseInfo::testLocalDistribVersion()
{
    GamsLicenseInfo gamsLicenseInfo;
    auto version = gamsLicenseInfo.localDistribVersion();
    QVERIFY(version >= QString(GAMS_VERSION_STRING).replace('.', "").toInt());
}

void TestGamsLicenseInfo::testLocalDistribVersionString()
{
    static QRegularExpression regex("^\\d+\\.\\d+\\.\\d( Alpha| Beta)*$");
    GamsLicenseInfo gamsLicenseInfo;
    auto version = gamsLicenseInfo.localDistribVersionString();
    auto match = regex.match(version);
    QVERIFY(match.hasMatch());
}

QTEST_MAIN(TestGamsLicenseInfo)
