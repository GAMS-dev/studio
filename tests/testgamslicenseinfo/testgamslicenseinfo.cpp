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
    const QString solver = "CPLEXD";
    GamsLicenseInfo gamsLicenseInfo;
    auto solverId = gamsLicenseInfo.solverId(solver);
    auto solverName = gamsLicenseInfo.solverName(solverId);
    QCOMPARE(solverName, solver);
}

void TestGamsLicenseInfo::testSolverIdLowerCase()
{
    const QString solver = "CPLEXD";
    GamsLicenseInfo gamsLicenseInfo;
    auto solverId = gamsLicenseInfo.solverId(solver.toLower());
    auto solverName = gamsLicenseInfo.solverName(solverId);
    QCOMPARE(solverName, solver);
}

void TestGamsLicenseInfo::testSolverIdMixedCase()
{
    const QString solver = "CpLeXd";
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

void TestGamsLicenseInfo::testSolverOptDefFilename_data()
{
    QTest::addColumn<QString>("solverName");
    QTest::addColumn<QString>("optDefFilename");
    QTest::addColumn<bool>("hidden");

    QTest::newRow("ALPHAECP") << "ALPHAECP"  << "optalphaecp.def"  << false;
    QTest::newRow("AMPL")     << "AMPL"      << "optampl.def"      << false;
    QTest::newRow("ANTIGONE") << "ANTIGONE"  << "optantigone.def"  << false;
    QTest::newRow("BARON")    << "BARON"     << "optbaron.def"     << false;
    QTest::newRow("BDMLP")    << "BDMLP"     << "optbdmlp.def"     << false;
    QTest::newRow("BENCH")    << "BENCH"     << "optbench.def"     << false;
    QTest::newRow("BONMIN")   << "BONMIN"    << "optbonmin.def"    << false;
    QTest::newRow("BONMINH")  << "BONMINH"   << "optbonmin.def"    << false;
    QTest::newRow("CBC")      << "CBC"       << "optcbc.def"       << false;

    QTest::newRow("COINBONMIN")  << "COINBONMIN"  << "optbonmin.def"   << true;
    QTest::newRow("COINCBC")     << "COINCBC"     << "optcbc.def"      << true;
    QTest::newRow("COINCOUENNE") << "COINCOUENNE" << "optcouenne.def"  << true;
    QTest::newRow("COINIPOPT")   << "COINIPOPT"   << "optipopt.def"    << true;
    QTest::newRow("COINSCIP")    << "COINSCIP"    << "optcoinscip.def" << true;

    QTest::newRow("CONOPT")   << "CONOPT"    << "optconopt.def"    << false;
    QTest::newRow("CONOPT4")  << "CONOPT4"   << "optconopt4.def"   << false;

    QTest::newRow("CONOPTD")  << "CONOPTD"   << "optconopt.def"    << true;

    QTest::newRow("CONVERT")  << "CONVERT"   << "optconvert.def"   << false;

    QTest::newRow("CONVERTD") << "CONVERTD"  << "optconvert.def"   << true;

    QTest::newRow("COUENNE")  << "COUENNE"   << "optcouenne.def"   << false;
    QTest::newRow("CPLEX")    << "CPLEX"     << "optcplex.def"     << false;
    QTest::newRow("cPleX")    << "cPleX"     << "optcplex.def"     << false;

    QTest::newRow("CPLEXD")   << "CPLEXD"    << "optcplex.def"     << true;

    QTest::newRow("DE")       << "DE"        << "optde.def"        << false;
    QTest::newRow("DECIS")    << "DECIS"     << "optdecis.def"     << false;
    QTest::newRow("DECISC")   << "DECISC"    << "optdecis.def"     << false;
    QTest::newRow("DESCISM")  << "DECISM"    << "optdecis.def"     << false;
    QTest::newRow("DICOPT")   << "DICOPT"    << "optdicopt.def"    << false;
    QTest::newRow("EXAMINER") << "EXAMINER"  << "optexaminer.def"  << false;

    QTest::newRow("EXAMINER2") << "EXAMINER2"  << "optexaminer.def"  << true;

    QTest::newRow("GAMSCHK")  << "GAMSCHK"  << "optgamschk.def"    << false;
    QTest::newRow("GLOMIQO")  << "GLOMIQO"  << "optglomiqo.def"    << false;
    QTest::newRow("GUROBI")  << "GUROBI"    << "optgurobi.def"    << false;
    QTest::newRow("IPOPT")   << "IPOPT"     << "optipopt.def"     << false;
    QTest::newRow("IPOPTH")  << "IPOPTH"    << "optipopt.def"     << false;
    QTest::newRow("JAMS")    << "JAMS"      << "optjams.def"      << false;
    QTest::newRow("KESTREL") << "KESTREL"   << "optkestrel.def"   << false;
    QTest::newRow("KNITRO")  << "KNITRO"    << "optknitro.def"    << false;
    QTest::newRow("LGO")     << "LGO"       << "optlgo.def"       << false;

    QTest::newRow("LGOD")    << "LGOD"      << "optlgo.def"       << true;

    QTest::newRow("LINDO")   << "LINDO"     << "optlindo.def"     << false;
    QTest::newRow("LINDOGLOBAL")   << "LINDOGLOBAL"   << "optlindoglobal.def"   << false;
    QTest::newRow("LINGO")         << "LINGO"         << "optlingo.def"         << false;
    QTest::newRow("LOCALSOLVER")   << "LOCALSOLVER"   << "optlocalsolver.def"   << false;
    QTest::newRow("LOCALSOLVER70") << "LOCALSOLVER70" << "optlocalsolver70.def" << false;
    QTest::newRow("LOGMIP")        << "LOGMIP"        << "optjams.def"          << false;
    QTest::newRow("LS")            << "LS"            << "optls.def"            << false;
    QTest::newRow("MILES")         << "MILES"         << "optmiles.def"         << false;

    QTest::newRow("MILESE")        << "MILESE"        << "optmiles.def"         << true;

    QTest::newRow("MINOS")        << "MINOS"          << "optminos.def"         << false;

    QTest::newRow("MINOS5")       << "MINOS5"         << "optminos.def"         << true;
    QTest::newRow("MINOS55")      << "MINOS55"        << "optminos.def"         << true;

    QTest::newRow("MOSEK")        << "MOSEK"          << "optmosek.def"         << false;
    QTest::newRow("MPECDUMP")     << "MPECDUMP"       << "optmpecdump.def"      << false;
    QTest::newRow("MPSGE")        << "MPSGE"          << "optmpsge.def"         << false;
    QTest::newRow("MSNLP")        << "MSNLP"          << "optmsnlp.def"         << false;
    QTest::newRow("NLPEC")        << "NLPEC"          << "optnlpec.def"         << false;
    QTest::newRow("ODHCPLEX")     << "ODHCPLEX"       << "optodhcplex.def"      << false;
    QTest::newRow("OSICPLEX")     << "OSICPLEX"       << "optosicplex.def"      << false;
    QTest::newRow("OSIGUROBI")    << "OSIGUROBI"      << "optosigurobi.def"     << false;
    QTest::newRow("OSIMOSEK")     << "OSIMOSEK"       << "optosimosek.def"      << false;
    QTest::newRow("OSIEXPRESS")   << "OSIXPRESS"      << "optosixpress.def"     << false;
    QTest::newRow("PATH")         << "PATH"           << "optpath.def"          << false;

    QTest::newRow("PATHC")        << "PATHC"          << "optpath.def"          << true;

    QTest::newRow("PATHNLP")      << "PATHNLP"        << "optpathnlp.def"       << false;
    QTest::newRow("PYOMO")        << "PYOMO"          << "optpyomo.def"         << false;

    QTest::newRow("QUADMINOS")    << "QUADMINOS"      << "optminos.def"         << true;

    QTest::newRow("SBB")          << "SBB"            << "optsbb.def"           << false;

    QTest::newRow("SCENSOLVER")   << "SCENSOLVER"     << "optscensolver.def"    << true;

    QTest::newRow("SCIP")         << "SCIP"           << "optscip.def"          << false;
    QTest::newRow("SELKIE")       << "SELKIE"         << "optselkie.def"        << false;
    QTest::newRow("SNOPT")        << "SNOPT"          << "optsnopt.def"         << false;
    QTest::newRow("SOLVEENGINE")  << "SOLVEENGINE"    << "optsolveengine.def"   << false;
    QTest::newRow("SOPLEX")       << "SOPLEX"         << "optsoplex.def"        << false;
    QTest::newRow("XA")           << "XA"             <<  "optxa.def"           << false;
    QTest::newRow("XPRESS")       << "XPRESS"         << "optxpress.def"        << false;

    QTest::newRow("BADSOLVERNAME")  << "BADSOLVERNAME"   << ""        << false;
    QTest::newRow("EMPTYNAME")      << ""                << ""        << false;

}

void TestGamsLicenseInfo::testSolverOptDefFilename()
{
#if CFGAPIVERSION > 2
    QFETCH(QString, solverName);
    QFETCH(QString, optDefFilename);
    QFETCH(bool, hidden);

    GamsLicenseInfo gamsLicenseInfo;
    QCOMPARE( optDefFilename, gamsLicenseInfo.solverOptDefFileName(solverName));
    QCOMPARE( hidden, gamsLicenseInfo.isSolverHidden(solverName) );
#else
    QVERIFY(true);
#endif
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
    auto solverId = gamsLicenseInfo.solverId("CPLEXD");
    bool capable = gamsLicenseInfo.solverCapability(solverId, cfgProc_lp);
    QVERIFY2(capable, "Ther solver CPLEXD shall be LP capable.");
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
    auto solverKeys = gamsLicenseInfo.solverNames().keys();
    QVERIFY(gamsLicenseInfo.solvers() >= solverKeys.size());
    for (auto solverId : solverKeys) {
        auto result = gamsLicenseInfo.solverLicense(solverId);
        test = result.contains("Demo") || result.contains("Full") ||
                result.contains("Evaluation") || result.contains("Expired");
        if (!test) break;
    }
    QVERIFY(test);
}

QTEST_MAIN(TestGamsLicenseInfo)
