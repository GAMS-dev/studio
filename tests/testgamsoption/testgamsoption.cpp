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
#include "testgamsoption.h"
//#include "option/option.h"
#include "commonpaths.h"

#include <QStandardPaths>

using gams::studio::Option;
using gams::studio::CommonPaths;

void TestGamsOption::initTestCase()
{
    // given
    const QString expected = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    CommonPaths::setSystemDir(expected.toLatin1());
    // when
    gamsOption = new Option(CommonPaths::systemDir(), "optgams.def");
    if  ( !gamsOption->available() ) {
       QFAIL("expected successful read of optgams.def, but failed");
    }
}

void TestGamsOption::testOptionEnumStrType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("numberOfEnumstr");
    QTest::addColumn<QString>("defaultValue");

    QTest::newRow("Action")     << "Action"      << true  << 6 << "CE";
    QTest::newRow("gdxConvert") << "gdxConvert"  << true  << 3 << "V7";
    QTest::newRow("gdxUels")    << "gdxUels"     << true  << 2 << "squeezed";
    QTest::newRow("SparseRun")  << "SparseRun"   << false << 2 << "On" ;
}

void TestGamsOption::testOptionEnumStrType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, numberOfEnumstr);
    QFETCH(QString, defaultValue);

    QCOMPARE( gamsOption->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( gamsOption->getOptionType(optionName),  optTypeEnumStr);
    QCOMPARE( gamsOption->getValueList(optionName).size() , numberOfEnumstr);
    QVERIFY( QString::compare(gamsOption->getDefaultValue(optionName).toString(), defaultValue, Qt::CaseInsensitive) == 0 );
}

void TestGamsOption::testOptionEnumIntType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("numberOfEnumint");
    QTest::addColumn<int>("defaultValue");

    QTest::newRow("PageContr")  << "PageContr"   << true  << 4  << 3;
    QTest::newRow("LogOption")  << "LogOption"   << true  << 5  << 1;
    QTest::newRow("AppendLog")  << "AppendLog"   << true  << 2  << 0;
    QTest::newRow("MultiPass")  << "MultiPass"   << true  << 3  << 0;
    QTest::newRow("DFormat")    << "DFormat"     << true  << 3  << 0;
}

void TestGamsOption::testOptionEnumIntType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, numberOfEnumint);
    QFETCH(int, defaultValue);

    QCOMPARE( gamsOption->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( gamsOption->getOptionType(optionName),  optTypeEnumInt);
    QCOMPARE( gamsOption->getValueList(optionName).size() , numberOfEnumint);
    QCOMPARE( gamsOption->getDefaultValue(optionName).toInt(), defaultValue );
}

void TestGamsOption::testOptionDoubleType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<double>("lowerBound");
    QTest::addColumn<double>("upperBound");
    QTest::addColumn<double>("defaultValue");

    QTest::newRow("ResLim")  <<  "ResLim"  << true  << 0.0 << gams::studio::OPTION_VALUE_MAXDOUBLE << 1000.0;
    QTest::newRow("OptCR")   << "OptCR"    << true  << 0.0 << gams::studio::OPTION_VALUE_MAXDOUBLE << 0.10;
    QTest::newRow("OptCA")   << "OptCA"    << true  << 0.0 << gams::studio::OPTION_VALUE_MAXDOUBLE << 0.0;
    QTest::newRow("Bratio")  << "Bratio"   << true  << 0.0 << 1.0 << 0.25;
    QTest::newRow("FDDelta") << "FDDelta"  << true << 1.000000000000000E-9 << 1.0 << 1.000000000000000E-5;
}

void TestGamsOption::testOptionDoubleType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(double, lowerBound);
    QFETCH(double, upperBound);
    QFETCH(double, defaultValue);

    QCOMPARE( gamsOption->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( gamsOption->getOptionType(optionName),  optTypeDouble);
    QCOMPARE( gamsOption->getLowerBound(optionName).toDouble(), lowerBound );
    QCOMPARE( gamsOption->getUpperBound(optionName).toDouble(), upperBound );
    QCOMPARE( gamsOption->getDefaultValue(optionName).toDouble(), defaultValue );
}

void TestGamsOption::testOptionSynonym_data()
{
    QTest::addColumn<QString>("optionSynonym");
    QTest::addColumn<QString>("optionName");

    QTest::newRow("EY") << "EY" << "EolOnly";
    QTest::newRow("R")  << "R"  << "Restart";
    QTest::newRow("I")  << "I"  << "Input";
    QTest::newRow("O")  << "O"  << "Output";
    QTest::newRow("S")  << "S"  << "Save";
    QTest::newRow("XS")  << "XS" << "XSave";
    QTest::newRow("EF")  << "EF" << "Expand";
    QTest::newRow("PW")  << "PW" << "PageWidth";
    QTest::newRow("PS")  << "PS" << "PageSize";
    QTest::newRow("PC")  << "PC" << "PageContr";
    QTest::newRow("A")   << "A"  << "Action";
    QTest::newRow("SD")  << "SD" << "ScrDir";
    QTest::newRow("SN")  << "SN" << "ScrNam";
    QTest::newRow("CDir")  << "CDir" << "CurDir";
    QTest::newRow("WDir")  << "WDir" << "WorkDir";
    QTest::newRow("PDir")  << "PDir" << "PutDir";
    QTest::newRow("GDir")  << "GDir" << "GridDir";
    QTest::newRow("SCRIPT")  << "SCRIPT" << "ScriptNext";
    QTest::newRow("SF")  << "SF" << "ScriptFrst";
    QTest::newRow("LL")  << "LL" << "LogLine";
    QTest::newRow("lo")  << "LO" << "LogOption";
    QTest::newRow("LF")  << "LF" << "LogFile";
    QTest::newRow("AO")  << "AO" << "AppendOut";
    QTest::newRow("AL")  << "AL" << "AppendLog";
    QTest::newRow("MP")  << "MP" << "MultiPass";
    QTest::newRow("DF")  << "DF" << "DFormat";
    QTest::newRow("TF")  << "TF" << "TFormat";
    QTest::newRow("SCNTR")  << "SCNTR" << "SolverCntr";
    QTest::newRow("SMATR")  << "SMATR" << "SolverMatr";
    QTest::newRow("SINST")  << "SINST" << "SolverInst";
    QTest::newRow("SSTAT")  << "SSTAT" << "SolverStat";
    QTest::newRow("SSOLU")  << "SSOLU" << "SolverSolu";
    QTest::newRow("SDICT")  << "SDICT" << "SolverDict";
    QTest::newRow("PU")   << "PU" << "PoolUse";
    QTest::newRow("PF1")  << "PF1" << "PoolFree1";
    QTest::newRow("PF2")  << "PF2" << "PoolFree2";
    QTest::newRow("PF3")  << "PF3" << "PoolFree3";
    QTest::newRow("PF4")  << "PF4" << "IntVarUp";
    QTest::newRow("PoolFree4")  << "PoolFree4" << "IntVarUp";
    QTest::newRow("PF5")  << "PF5" << "PoolFree5";
    QTest::newRow("PF6")  << "PF6" << "PoolFree6";
    QTest::newRow("PF7")  << "PF7" << "PoolFree7";
    QTest::newRow("CX")  << "CX" << "CodeX";
    QTest::newRow("TM")  << "TM" << "TopMargin";
    QTest::newRow("LM")  << "LM" << "LeftMargin";
    QTest::newRow("BM")  << "BM" << "BotMargin";
    QTest::newRow("DP")  << "DP" << "DumpParms";
    QTest::newRow("IDIR1")  << "IDIR1" << "InputDir1";
    QTest::newRow("IDIR2")  << "IDIR2" << "InputDir2";
    QTest::newRow("IDIR3")  << "IDIR3" << "InputDir3";
    QTest::newRow("IDIR4")  << "IDIR4" << "InputDir4";
    QTest::newRow("IDIR5")  << "IDIR5" << "InputDir5";
    QTest::newRow("IDIR6")  << "IDIR6" << "InputDir6";
    QTest::newRow("IDIR7")  << "IDIR7" << "InputDir7";
    QTest::newRow("IDIR8")  << "IDIR8" << "InputDir8";
    QTest::newRow("IDIR9")  << "IDIR9" << "InputDir9";
    QTest::newRow("IDIR10")  << "IDIR10" << "InputDir10";
    QTest::newRow("IDIR11")  << "IDIR11" << "InputDir11";
    QTest::newRow("IDIR12")  << "IDIR12" << "InputDir12";
    QTest::newRow("IDIR13")  << "IDIR13" << "InputDir13";
    QTest::newRow("IDIR14")  << "IDIR14" << "InputDir14";
    QTest::newRow("IDIR15")  << "IDIR15" << "InputDir15";
    QTest::newRow("IDIR16")  << "IDIR16" << "InputDir16";
    QTest::newRow("IDIR17")  << "IDIR17" << "InputDir17";
    QTest::newRow("IDIR18")  << "IDIR18" << "InputDir18";
    QTest::newRow("IDIR19")  << "IDIR19" << "InputDir19";
    QTest::newRow("IDIR20")  << "IDIR20" << "InputDir20";
    QTest::newRow("IDIR21")  << "IDIR21" << "InputDir21";
    QTest::newRow("IDIR22")  << "IDIR22" << "InputDir22";
    QTest::newRow("IDIR23")  << "IDIR23" << "InputDir23";
    QTest::newRow("IDIR24")  << "IDIR24" << "InputDir24";
    QTest::newRow("IDIR25")  << "IDIR25" << "InputDir25";
    QTest::newRow("IDIR26")  << "IDIR26" << "InputDir26";
    QTest::newRow("IDIR27")  << "IDIR27"<< "InputDir27";
    QTest::newRow("IDIR28")  << "IDIR28" << "InputDir28";
    QTest::newRow("IDIR29")  << "IDIR29" << "InputDir29";
    QTest::newRow("IDIR30")  << "IDIR30" << "InputDir30";
    QTest::newRow("IDIR31")  << "IDIR31"<< "InputDir31";
    QTest::newRow("IDIR32")  << "IDIR32" << "InputDir32";
    QTest::newRow("IDIR33")  << "IDIR33" << "InputDir33";
    QTest::newRow("IDIR34")  << "IDIR34" << "InputDir34";
    QTest::newRow("IDIR35")  << "IDIR35" << "InputDir35";
    QTest::newRow("IDIR36")  << "IDIR36" << "InputDir36";
    QTest::newRow("IDIR37")  << "IDIR37" << "InputDir37";
    QTest::newRow("IDIR38")  << "IDIR38" << "InputDir38";
    QTest::newRow("IDIR39")  << "IDIR39" << "InputDir39";
    QTest::newRow("IDIR40")  << "IDIR40" << "InputDir40";
    QTest::newRow("IDIR")  << "IDIR" << "InputDir";
    QTest::newRow("LDIR")  << "LDIR" << "LibIncDir";
    QTest::newRow("SDIR")  << "SDIR" << "SysIncDir";
    QTest::newRow("U1")  << "U1" << "User1";
    QTest::newRow("U2")  << "U2" << "User2";
    QTest::newRow("U3")  << "U3" << "User3";
    QTest::newRow("U4")  << "U4" << "User4";
    QTest::newRow("U5")  << "U5" << "User5";
    QTest::newRow("FW")  << "FW" << "ForceWork";
    QTest::newRow("GLAN")   << "GLAN"  << "GLanguage";
    QTest::newRow("GFINC")  << "GFINC" << "GFinclude";
    QTest::newRow("UT")     << "UT"    << "UnitType";
    QTest::newRow("GFIMP")  << "GFIMP" << "GFImplicit";
    QTest::newRow("RF")  << "RF" << "Reference";
    QTest::newRow("PF")  << "PF" << "ParmFile";
    QTest::newRow("ER")  << "ER" << "ErrorLog";
    QTest::newRow("JT")  << "JT" << "JobTrace";
    QTest::newRow("TL")  << "TL" << "TraceLevel";
    QTest::newRow("ST")  << "ST" << "SetType";
    QTest::newRow("SP")  << "SP" << "SavePoint";
    QTest::newRow("SL")  << "SL" << "SolveLink";
    QTest::newRow("noSolveSkip")  << "noSolveSkip" << "Sys12";
    QTest::newRow("HL")  << "HL" << "HeapLimit";
    QTest::newRow("SE")  << "SE" << "ScrExt";
    QTest::newRow("AE")  << "AE" << "AppendExpand";
    QTest::newRow("ETL")      << "ETL"     << "ETLim";
    QTest::newRow("PTOL")     <<  "PTOL"   << "ProfileTol";
    QTest::newRow("PFILE")    << "PFILE"   << "ProfileFile";
    QTest::newRow("GScript")  << "GScript" << "GridScript";
    QTest::newRow("DPLP")  << "DPLP" << "DumpParmsLogPrefix";
    QTest::newRow("SO")    << "SO" << "SaveObfuscate";
    QTest::newRow("XSO")   << "XSO" << "XSaveObfuscate";
    QTest::newRow("RN")    << "RN" << "RestartNamed";

    QTest::newRow("Bratio")     << "" << "Bratio";
    QTest::newRow("IterLim")     << "" << "IterLim";
    QTest::newRow("gdxConvert")  << "" << "gdxConvert";
    QTest::newRow("ProcDirPath") << "" << "ProcDirPath";

}

void TestGamsOption::testOptionSynonym()
{
    QFETCH(QString, optionSynonym);
    QFETCH(QString, optionName);

    if (optionSynonym.isEmpty()) {
        QVERIFY( gamsOption->getNameFromSynonym(optionSynonym).toUpper().isEmpty() );
        QVERIFY( !gamsOption->isASynonym(optionName) );
    } else {
       QVERIFY( gamsOption->isASynonym(optionSynonym) );
       QCOMPARE( gamsOption->getNameFromSynonym(optionSynonym).toUpper(), optionName.toUpper() );
    }
}

void TestGamsOption::testDeprecatedOption_data()
{
    QTest::addColumn<QString>("deprecatedOption");
    QTest::addColumn<QString>("optionDescription");

    QTest::newRow("CtrlZ")      << "CtrlZ"     << "Enable reading control Z";
    QTest::newRow("CtrlM")      << "CtrlM"     << "Enable flexible line ending";
    QTest::newRow("PoolUse")    << "PoolUse"   << "Manage memory pools";
    QTest::newRow("PoolFree1")  << "PoolFree1" << "Manage memory pools N";
    QTest::newRow("PoolFree2")  << "PoolFree2" << "Manage memory pools N";
    QTest::newRow("PoolFree3")  << "PoolFree3" << "Manage memory pools N";
    QTest::newRow("PoolFree5")  << "PoolFree5" << "Manage memory pools N";
    QTest::newRow("PoolFree6")  << "PoolFree6" << "Manage memory pools N";
    QTest::newRow("PoolFree7")  << "PoolFree7" << "Manage memory pools N";
    QTest::newRow("CodeX")      << "CodeX"     << "Controls the allocation of executable code";
    QTest::newRow("NlCon")      << "NlCon"     << "Nonlinear instructions search length";
    QTest::newRow("GLanguage")  << "GLanguage" << "";
    QTest::newRow("GFinclude")  << "GFinclude" << "";
    QTest::newRow("UnitType")   << "UnitType"  << "";
    QTest::newRow("GFImplicit") << "UnitType"  << "";
    QTest::newRow("GFExt")      << "GFExt"     << "";
    QTest::newRow("SetType")    << "SetType"   << "Used to set the type of --keys";
    QTest::newRow("TopMargin")  << "TopMargin" << "Output file page top margin";
    QTest::newRow("LeftMargin") << "LeftMargin"<< "Output file page left margin";
    QTest::newRow("BotMargin")  << "BotMargin" << "Output file page bottom margin, lines added at the end of a page";
}

void TestGamsOption::testDeprecatedOption()
{
    QFETCH(QString, deprecatedOption);
    QFETCH(QString, optionDescription);

    QVERIFY( gamsOption->isValid(deprecatedOption) );
    QVERIFY( gamsOption->isDeprecated(deprecatedOption) );
    QCOMPARE( gamsOption->getDescription(deprecatedOption).trimmed().toUpper(), optionDescription.trimmed().toUpper());
}

void TestGamsOption::testDoubleDashedOption_data()
{
    QTest::addColumn<QString>("option");
    QTest::addColumn<bool>("isDoubleDashedOption");
    QTest::addColumn<bool>("isValidDoubleDashedOptionName");

    QTest::newRow("EY") << "EY" << false << true;
    QTest::newRow("R")  << "R"  << false << true;
    QTest::newRow("--xyz")  << "--xyz" << true << true;
    QTest::newRow("//xyz")  << "//xyz" << true << true;
    QTest::newRow("-/xyz")  << "//xyz" << true << true;
    QTest::newRow("/-xyz")  << "//xyz" << true << true;
    QTest::newRow("----xyz")    << "----xyz" << true  << true;
    QTest::newRow("-/-/--xyz")  << "----xyz" << true  << true;
    QTest::newRow("-xyz")   << "-xyz"        << false << true;
    QTest::newRow("-%xyz")  << "-%xyz"       << false << false;
    QTest::newRow("--")     << "--"          << true  << false;
    QTest::newRow("--x")    << "--x"         << true  << true;
    QTest::newRow("--1")    << "--1"         << true  << false;
    QTest::newRow("--xyz_1234")   << "--xyz_1234"  << true  << true;
    QTest::newRow("--1234xyz")    << "--1234xyz"   << true  << false;
    QTest::newRow("--_xyz_1234")  << "--_xyz_1234" << true  << false;
    QTest::newRow("--xyz@1234")   << "--xyz@1234"  << true  << false;
}

void TestGamsOption::testDoubleDashedOption()
{
    QFETCH(QString, option);
    QFETCH(bool, isDoubleDashedOption);
    QFETCH(bool, isValidDoubleDashedOptionName);

    QCOMPARE( gamsOption->isDoubleDashedOption(option), isDoubleDashedOption );
    QCOMPARE( gamsOption->isDoubleDashedOptionNameValid( gamsOption->getOptionKey(option) ),
              isValidDoubleDashedOptionName );
}

void TestGamsOption::testOptionGroup_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<int>("groupNumber");
    QTest::addColumn<QString>("optionGroupName");
    QTest::addColumn<QString>("optionGroupDescription");
    QTest::addColumn<QString>("optionType");

    QTest::newRow("action_1")      << "action"     << 1 << "general" << "General options" << "enumstr";
    QTest::newRow("charSet_1")     << "charSet"    << 1 << "general" << "General options" << "enumint";
    QTest::newRow("dFormat_1")     << "dFormat"    << 1 << "general" << "General options" << "enumint";
    QTest::newRow("eolOnly_1")     << "eolOnly"    << 1 << "general" << "General options" << "immediate";
    QTest::newRow("fdDelta_1")     << "fdDelta"    << 1 << "general" << "General options" << "double";
    QTest::newRow("GDX_1")         << "GDX"        << 1 << "general" << "General options" << "string";
    QTest::newRow("HeapLimit_1")   << "HeapLimit"  << 1 << "general" << "General options" << "double";
    QTest::newRow("input_1")       << "input"      << 1 << "general" << "General options" << "string";
    QTest::newRow("jobtrace_1")    << "jobtrace"   << 1 << "general" << "General options" << "string";
    QTest::newRow("keep_1")        << "keep"       << 1 << "general" << "General options" << "enumint";
    QTest::newRow("LogOption_1")   << "LogOption"  << 1 << "general" << "General options" << "enumint";
    QTest::newRow("lstTitleLeftAligned_1")  << "lstTitleLeftAligned"  << 1 << "general" << "General options" << "enumint";
    QTest::newRow("multipass_1")   << "multipass"  << 1 << "general" << "General options" << "enumint";
    QTest::newRow("noNewVarEqu_1") << "noNewVarEqu"  << 1 << "general" << "General options" << "enumint";
    QTest::newRow("output_1")      << "output"       << 1 << "general" << "General options" << "string";
    QTest::newRow("ParmFile_1")    << "ParmFile"     << 1 << "general" << "General options" << "immediate";
    QTest::newRow("ParmFile_1")    << "ParmFile"     << 1 << "general" << "General options" << "immediate";
    QTest::newRow("reference_1")   << "reference"    << 1 << "general" << "General options" << "string";
    QTest::newRow("suppress_1")    << "suppress"     << 1 << "general" << "General options" << "enumint";
    QTest::newRow("TraceOpt_1")    << "TraceOpt"     << 1 << "general" << "General options" << "enumint";
    QTest::newRow("user3_1")       << "user3"        << 1 << "general" << "General options" << "string";
    QTest::newRow("workDir_1")     << "workDir"      << 1 << "general" << "General options" << "string";
    QTest::newRow("zeroRes_1")     << "zeroRes"      << 1 << "general" << "General options" << "double";

    QTest::newRow("fSave_2")     << "fSave"      << 2 << "saverestar" << "Save and Restart related options" << "enumint";
    QTest::newRow("restart_2")   << "restart"    << 2 << "saverestar" << "Save and Restart related options" << "string";
    QTest::newRow("restartNamed_2")   << "restartNamed"    << 2 << "saverestar" << "Save and Restart related options" << "string";
    QTest::newRow("save_2")           << "restart"         << 2 << "saverestar" << "Save and Restart related options" << "string";
    QTest::newRow("saveObfuscate_2")  << "saveObfuscate"   << 2 << "saverestar" << "Save and Restart related options" << "string";
    QTest::newRow("symPrefix_2")      << "symPrefix"       << 2 << "saverestar" << "Save and Restart related options" << "string";
    QTest::newRow("xSave_2")          << "xSave"           << 2 << "saverestar" << "Save and Restart related options" << "string";
    QTest::newRow("xSaveObfuscate_2") << "xSaveObfuscate"  << 2 << "saverestar" << "Save and Restart related options" << "string";

    QTest::newRow("Bratio_3")       << "Bratio"       << 3 << "solvers" << "Solver related options" << "double";
    QTest::newRow("CNS_3")          << "CNS"          << 3 << "solvers" << "Solver related options" << "string";
    QTest::newRow("DNLP_3")         << "DNLP"         << 3 << "solvers" << "Solver related options" << "string";
    QTest::newRow("EMP_3")          << "EMP"          << 3 << "solvers" << "Solver related options" << "string";
    QTest::newRow("forceOptFile_3") << "forceOptFile" << 3 << "solvers" << "Solver related options" << "integer";
    QTest::newRow("holdfixed_3")    << "holdfixed"    << 3 << "solvers" << "Solver related options" << "enumint";
    QTest::newRow("IterLim_3")      << "IterLim"      << 3 << "solvers" << "Solver related options" << "integer";
    QTest::newRow("LimRow_3")       << "LimRow"       << 3 << "solvers" << "Solver related options" << "integer";
    QTest::newRow("MIP_3")          << "MIP"          << 3 << "solvers" << "Solver related options" << "string";
    QTest::newRow("nodLim_3")       << "nodLim"       << 3 << "solvers" << "Solver related options" << "integer";
    QTest::newRow("optFile_3")      << "optFile"      << 3 << "solvers" << "Solver related options" << "integer";
    QTest::newRow("QCP_3")          << "QCP"          << 3 << "solvers" << "Solver related options" << "string";
    QTest::newRow("ResLim_3")       << "ResLim"       << 3 << "solvers" << "Solver related options" << "double";
    QTest::newRow("SolveLink_3")    << "SolveLink"    << 3 << "solvers" << "Solver related options" << "enumint";
    QTest::newRow("threads_3")      << "threads"      << 3 << "solvers" << "Solver related options" << "integer";
    QTest::newRow("WorkSpace_3")    << "WorkSpace"    << 3 << "solvers" << "Solver related options" << "double";

}

void TestGamsOption::testOptionGroup()
{
    QFETCH(QString, optionName);
    QFETCH(int, groupNumber);
    QFETCH(QString, optionGroupName);
    QFETCH(QString, optionGroupDescription);
    QFETCH(QString, optionType);

    QCOMPARE( gamsOption->getGroupNumber(optionName), groupNumber );
    QCOMPARE( gamsOption->getGroupDescription(optionName), optionGroupDescription );
    QCOMPARE( gamsOption->getOptionTypeName(gamsOption->getOptionType(optionName)), optionType );
}

void TestGamsOption::cleanupTestCase()
{
    if (gamsOption)
       delete gamsOption;
}

QTEST_MAIN(TestGamsOption)
