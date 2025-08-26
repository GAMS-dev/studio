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
#include "testgamslicenseinfo.h"
#include "gamslicenseinfo.h"
#include "commonpaths.h"
#include "cfgmcc.h"

#include <QTextStream>

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
    auto dbgstr = QString("%1 : %2").arg(licensePath, license.join("\n"));
    QVERIFY2(gamsLicenseInfo.isLicenseValid(license), dbgstr.toStdString().c_str());
}

void TestGamsLicenseInfo::testGamsDataLocations()
{
    auto actual = GamsLicenseInfo().gamsDataLocations();
    QVERIFY(!actual.isEmpty());
}

void TestGamsLicenseInfo::testGamsConfigLocations()
{
    auto actual = GamsLicenseInfo().gamsConfigLocations();
    QVERIFY(!actual.isEmpty());
}

void TestGamsLicenseInfo::testLocalDistribVersion()
{
    static QRegularExpression regex(R"(^\d{4,4}$)");
    GamsLicenseInfo gamsLicenseInfo;
    auto version = gamsLicenseInfo.localDistribVersion();
    QString verstr = QString::number(version);
    QVERIFY(regex.match(verstr).hasMatch());
}

void TestGamsLicenseInfo::testLocalDistribVersionString()
{
    static QRegularExpression regex("^\\d+\\.\\d+\\.\\d( Alpha| Beta)*$");
    GamsLicenseInfo gamsLicenseInfo;
    auto version = gamsLicenseInfo.localDistribVersionString();
    auto match = regex.match(version);
    QVERIFY(match.hasMatch());
}

void TestGamsLicenseInfo::testLocalDistribVersionStringShort()
{
    static QRegularExpression regex("^\\d+\\.\\d+");
    GamsLicenseInfo gamsLicenseInfo;
    auto version = gamsLicenseInfo.localDistribVersionStringShort();
    auto match = regex.match(version);
    QVERIFY(match.hasMatch());
}

void TestGamsLicenseInfo::testLicenseFromFile_simpleErrorCases()
{
    GamsLicenseInfo licenseInfo;
    auto val1 = licenseInfo.licenseFromFile("notthere.txt");
    QCOMPARE(val1, QStringList());
    auto val2 = licenseInfo.licenseFromFile("notatxt.dat");
    QCOMPARE(val2, QStringList());

    QFile testFile("lic0.txt");
    if (testFile.open(QFile::WriteOnly | QFile::Text)) {
        testFile.write("123\n");
        testFile.write("abc \n");
        testFile.write(" -+_ \n");
        testFile.close();
    }
    QCOMPARE(licenseInfo.licenseFromFile("lic0.txt"), QStringList());

    if (testFile.open(QFile::WriteOnly | QFile::Text)) {
        testFile.write("GAMS_Demo,_for_EULA_and_demo_limitations_see___G240131/0001CB-GEN\n");
        testFile.write(" -+_ \n");
        testFile.close();
    }
    QCOMPARE(licenseInfo.licenseFromFile("lic0.txt"), QStringList());
}

void TestGamsLicenseInfo::testLicenseFromFile_licenseWithLines()
{
    GamsLicenseInfo licenseInfo;
    QFile testFile("lic1.txt");
    if (testFile.open(QFile::WriteOnly | QFile::Text)) {
        testFile.write(testLicense().join('\n').toUtf8());
        testFile.close();
    }
    QCOMPARE(licenseInfo.licenseFromFile("lic1.txt"), testLicense());
}

void TestGamsLicenseInfo::testLicenseFromFile_licenseContinuesLine()
{
    GamsLicenseInfo licenseInfo;
    QFile testFile("lic2.txt");
    if (testFile.open(QFile::WriteOnly | QFile::Text)) {
        testFile.write(testLicense().join("").toUtf8());
        testFile.close();
    }
    QCOMPARE(licenseInfo.licenseFromFile("lic2.txt"), testLicense());
}

void TestGamsLicenseInfo::testLicenseFromFile_randomSpaces()
{
    GamsLicenseInfo licenseInfo;
    QFile testFile("lic3.txt");
    if (testFile.open(QFile::WriteOnly | QFile::Text)) {
        testFile.write(testLicenseWithSpaces().toUtf8());
        testFile.close();
    }
    QCOMPARE(licenseInfo.licenseFromFile("lic3.txt"), testLicense());
}

void TestGamsLicenseInfo::testLicenseFromFile_BOM()
{
    GamsLicenseInfo licenseInfo;
    QFile testFile("lic4.txt");
    if (testFile.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream ostream(&testFile);
        ostream.setGenerateByteOrderMark(true);
        ostream << testLicense().join('\n');
        ostream.flush();
    }
    QCOMPARE(licenseInfo.licenseFromFile("lic4.txt"), testLicense());
}

void TestGamsLicenseInfo::testLicenseDirectory()
{
    auto path = GamsLicenseInfo::licenseDirectory();
    QVERIFY(!path.endsWith(CommonPaths::licenseFile()));
}

void TestGamsLicenseInfo::testLicenseLocation()
{
    auto path = GamsLicenseInfo::licenseLocation();
    QVERIFY(path.endsWith(CommonPaths::licenseFile()));
}

QStringList TestGamsLicenseInfo::testLicense()
{// uses the GAMS 46.0 default license
    QStringList license;
    license << "GAMS_Demo,_for_EULA_and_demo_limitations_see___G240131/0001CB-GEN";
    license << "https://www.gams.com/latest/docs/UG%5FLicense.html_______________";
    license << "1496554900_______________________________________________________";
    license << "0801346905_______________________________________________________";
    license << "DC0000_______g_1_______________________________C_Eval____________";
    license << "1496554900_______________________________________________________";
    license << "0801346905_______________________________________________________";
    license << "DC0000_______g_1_______________________________C_Eval____________";
    return license;
}

QString TestGamsLicenseInfo::testLicenseWithSpaces()
{// uses the GAMS 46.0 default license
    QString license;
    license += "GAMS_Demo,_for_EULA_and_demo_limitations_see___G240131/0001CB-GEN\n\t";
    license += "https://www.gams.com/latest/docs/UG%5FLicense.html_______________";
    license += "1496554900___________________________\v_________________\f___________   ";
    license += "0801346905________________________________________________\r_______";
    license += "       DC0000_______g_1______________\r\r\r_________________C_Eval_______ _____    ";
    license += "1496554900___________________________\v_________________\f___________   ";
    license += "0801346905________________________________________________\r_______";
    license += "DC0000_______g_1______________\r\r\r_________________C_Eval_______ _____    ";
    return license;
}

QTEST_MAIN(TestGamsLicenseInfo)
