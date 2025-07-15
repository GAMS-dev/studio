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
#include "testgamsoption.h"
#include "commonpaths.h"

#include <QStandardPaths>

using gams::studio::CommonPaths;

void TestGamsOption::initTestCase()
{
    // given
    const QString expected = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    CommonPaths::setSystemDir(expected.toLatin1());
    // when
    optionTokenizer = new OptionTokenizer(QString("optgams.def"));
    gamsOption = optionTokenizer->getOption();
    if  ( !gamsOption->available() ) {
       QFAIL("expected successful read of optgams.def, but failed");
    }
}

void TestGamsOption::testOptionStringType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QString>("description");

    QTest::newRow("SubSys")        << "SubSys"    << true  << "Name of subsystem configuration file";
    QTest::newRow("ErrNam")        << "ErrNam"     << true  << "Name of error message file";
    QTest::newRow("CurDir")        << "CurDir"     << true  << "Current directory";
    QTest::newRow("WorkDir")       << "WorkDir"    << true  << "Working directory";
}

void TestGamsOption::testOptionStringType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(QString, description);

    QCOMPARE( gamsOption->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( gamsOption->getOptionType(optionName),  optTypeString);
    QCOMPARE( gamsOption->getOptionDefinition(optionName).description, description);

}

void TestGamsOption::testOptionModelTypeString_data()
{
    QTest::addColumn<QString>("optionName");

    QTest::newRow("LP")      << "LP";
    QTest::newRow("MIP")     << "MIP";
    QTest::newRow("RMIP")    << "RMIP";
    QTest::newRow("NLP")     << "NLP";
    QTest::newRow("MCP")     << "MCP";
    QTest::newRow("MPEC")    << "MPEC";
    QTest::newRow("RMPEC")   << "RMPEC";
    QTest::newRow("CNS")     << "CNS";
    QTest::newRow("DNLP")    << "DNLP";
    QTest::newRow("RMINLP")  << "RMINLP";
    QTest::newRow("MINLP")   << "MINLP";
    QTest::newRow("QCP")     << "QCP";
    QTest::newRow("MIQCP")   << "MIQCP";
    QTest::newRow("RMIQCP")  << "RMIQCP";
    QTest::newRow("EMP")     << "EMP";
}

void TestGamsOption::testOptionModelTypeString()
{
    QFETCH(QString, optionName);

    QCOMPARE( gamsOption->getOptionType(optionName), optTypeString );
    QVERIFY( gamsOption->getValueList(optionName).size() > 0 );
    QVERIFY( !gamsOption->getDefaultValue(optionName).toString().isEmpty() );
    for(const OptionValue &value : gamsOption->getValueList(optionName)) {
        QVERIFY( !value.hidden );
        QVERIFY( !value.enumFlag );
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

void TestGamsOption::testOptionEnumStrValue_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("valueIndex");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<QString>("value");
    QTest::addColumn<QString>("description");

    QTest::newRow("Action_R")   << "Action"  << true  << 0 << false << "R"  << "Restart After Solve";
    QTest::newRow("Action_C")   << "Action"  << true  << 1 << false << "C"  << "CompileOnly";
    QTest::newRow("Action_E")   << "Action"  << true  << 2 << false << "E"  << "ExecuteOnly";
    QTest::newRow("Action_CE")  << "Action"  << true  << 3 << false << "CE" << "Compile and Execute";
    QTest::newRow("Action_G")   << "Action"  << true  << 4 << true  << "G"  << "Glue Code Generation";
    QTest::newRow("Action_GT")  << "Action"  << true  << 5 << false << "GT" << "Trace Report";

    QTest::newRow("gdxUels_Squeezed")  << "gdxUels"  << true  << 0 << false << "Squeezed"  << "Write only the UELs to Universe, that are used by the exported symbols";
    QTest::newRow("gdxUels_Full")      << "gdxUels"  << true  << 1 << false << "Full"      << "Write all UELs to Universe";
}

void TestGamsOption::testOptionEnumStrValue()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, valueIndex);
    QFETCH(bool, hidden);
    QFETCH(QString, value);
    QFETCH(QString, description);

    QCOMPARE( gamsOption-> getOptionDefinition(optionName).valid, valid );
    QCOMPARE( gamsOption->getOptionType(optionName),  optTypeEnumStr );
    QCOMPARE( gamsOption->getValueList(optionName).at(valueIndex).hidden, hidden );
    QCOMPARE( gamsOption->getValueList(optionName).at(valueIndex).value.toString().toLower(), value.toLower() );
    QCOMPARE( gamsOption->getValueList(optionName).at(valueIndex).description.toLower(), description.toLower() );
}

void TestGamsOption::testOptionEnumIntType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("numberOfEnumint");
    QTest::addColumn<int>("defaultValue");

    QTest::newRow("PageContr")  << "PageContr"   << true  << 4  << 2;
    QTest::newRow("LogOption")  << "LogOption"   << true  << 5  << 3;
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

void TestGamsOption::testOptionEnumIntValue_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("valueIndex");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<int>("value");
    QTest::addColumn<QString>("description");

    QTest::newRow("LogLine_0")  << "LogLine"  << true  << 0 << false << 0  << "No line tracing";
    QTest::newRow("LogLine_1")  << "LogLine"  << true  << 1 << false << 1  << "Minimum line tracing";
    QTest::newRow("LogLine_2")  << "LogLine"  << true  << 2 << false << 2  << "Automatic and visually pleasing";

    QTest::newRow("MultiPass_0")  << "MultiPass"  << true  << 0 << false << 0  << "Standard compilation";
    QTest::newRow("MultiPass_1")  << "MultiPass"  << true  << 1 << false << 1  << "Check-out compilation";
    QTest::newRow("MultiPass_2")  << "MultiPass"  << true  << 2 << false << 2  << "as 1, and skip $call and ignore missing file errors with $include and $gdxin as well as unknown dimension errors with empty data statements";

    QTest::newRow("DumpOpt_0")  << "DumpOpt"  << true  << 0  << false << 0  << "No dumpfile";
    QTest::newRow("DumpOpt_1")  << "DumpOpt"  << true  << 1  << false << 1  << "Extract referenced data from the restart file using original set element names";
    QTest::newRow("DumpOpt_2")  << "DumpOpt"  << true  << 2  << false << 2  << "Extract referenced data from the restart file using new set element names";
    QTest::newRow("DumpOpt_3")  << "DumpOpt"  << true  << 3  << false << 3  << "Extract referenced data from the restart file using new set element names and drop symbol text";
    QTest::newRow("DumpOpt_4")  << "DumpOpt"  << true  << 4  << false << 4  << "Extract referenced symbol declarations from the restart file";
    QTest::newRow("DumpOpt_10")  << "DumpOpt" << true  << 5  << true  << 10  << "(Same as 11 and therefore hidden)";
    QTest::newRow("DumpOpt_11")  << "DumpOpt" << true  << 6  << false << 11  << "Write processed input file without comments";
    QTest::newRow("DumpOpt_12")  << "DumpOpt" << true  << 7  << true  << 12  << "(Same as 11 and therefore hidden)";
    QTest::newRow("DumpOpt_19")  << "DumpOpt" << true  << 8  << true  << 19  << "(Same as 21 and therefore hidden)";
    QTest::newRow("DumpOpt_20")  << "DumpOpt" << true  << 9  << true  << 20  << "(Same as 21 and therefore hidden)";
    QTest::newRow("DumpOpt_21")  << "DumpOpt" << true  << 10 << false << 21  << "Write processed input file with all comments";

    QTest::newRow("TraceOpt_0")  << "TraceOpt"  << true  << 0 << false << 0  << "Solver and GAMS step trace";
    QTest::newRow("TraceOpt_1")  << "TraceOpt"  << true  << 1 << false << 1  << "Solver and GAMS exit trace";
    QTest::newRow("TraceOpt_2")  << "TraceOpt"  << true  << 2 << false << 2  << "Solver trace only";
    QTest::newRow("TraceOpt_3")  << "TraceOpt"  << true  << 3 << false << 3  << "Solver trace only in format used for GAMS performance world";
    QTest::newRow("TraceOpt_4")  << "TraceOpt"  << true  << 4 << true  << 4  << "Trace file format supporting NLPEC";
    QTest::newRow("TraceOpt_5")  << "TraceOpt"  << true  << 5 << false << 5  << "GAMS exit trace with all available trace fields";
}

void TestGamsOption::testOptionEnumIntValue()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, valueIndex);
    QFETCH(bool, hidden);
    QFETCH(int, value);
    QFETCH(QString, description);

    QCOMPARE( gamsOption->getOptionDefinition(optionName).valid, valid );
    QCOMPARE( gamsOption->getOptionType(optionName),  optTypeEnumInt );
    QCOMPARE( gamsOption->getValueList(optionName).at(valueIndex).hidden, hidden );
    QCOMPARE( gamsOption->getValueList(optionName).at(valueIndex).value.toInt(), value );
    QCOMPARE( gamsOption->getValueList(optionName).at(valueIndex).description.toLower(), description.toLower() );
}

void TestGamsOption::testOptionDoubleType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<double>("lowerBound");
    QTest::addColumn<double>("upperBound");
    QTest::addColumn<double>("defaultValue");

    QTest::newRow("ResLim")  <<  "ResLim"  << true  << 0.0 << gams::studio::option::OPTION_VALUE_MAXDOUBLE <<  10000000000.00;
    QTest::newRow("OptCR")   << "OptCR"    << true  << 0.0 << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 1.000000000000000E-4;
    QTest::newRow("OptCA")   << "OptCA"    << true  << 0.0 << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 0.00;
    QTest::newRow("Bratio")  << "Bratio"   << true  << 0.0 << 1.0                                  << 0.25;
    QTest::newRow("FDDelta") << "FDDelta"  << true  << 1.000000000000000E-9 << 1.0                 << 1.000000000000000E-5;
    QTest::newRow("ZeroRes") << "ZeroRes"  << true  << 0.0 << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 0.0;
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

void TestGamsOption::testOptionIntegerType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("lowerBound");
    QTest::addColumn<int>("upperBound");
    QTest::addColumn<int>("defaultValue");

    QTest::newRow("PageWidth")   << "PageWidth"   << true   << 72  << 32767                                      << 32767;
    QTest::newRow("PoolUse")     << "PoolUse"     << true   << 0   << gams::studio::option::OPTION_VALUE_MAXINT  << 0;
    QTest::newRow("CErr")        << "CErr"        << true   << 0   << gams::studio::option::OPTION_VALUE_MAXINT  << 0;
    QTest::newRow("Opt")         << "Opt"         << false  << 0   << gams::studio::option::OPTION_VALUE_MAXINT  << 0;
    QTest::newRow("IterLim")     << "IterLim"     << true   << 0   << gams::studio::option::OPTION_VALUE_MAXINT  << 2147483647;
    QTest::newRow("Seed")        << "Seed"        << true   << 0   << gams::studio::option::OPTION_VALUE_MAXINT  << 3141;
}

void TestGamsOption::testOptionIntegerType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, lowerBound);
    QFETCH(int, upperBound);
    QFETCH(int, defaultValue);

    QCOMPARE( gamsOption->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( gamsOption->getOptionType(optionName),  optTypeInteger);
    QCOMPARE( gamsOption->getLowerBound(optionName).toDouble(), lowerBound );
    QCOMPARE( gamsOption->getUpperBound(optionName).toDouble(), upperBound );
    QCOMPARE( gamsOption->getDefaultValue(optionName).toDouble(), defaultValue );
}

void TestGamsOption::testOptionImmeidateType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QString>("description");

    QTest::newRow("EolOnly")      << "EolOnly"        << true << "Single key-value pairs (immediate switch)";
    QTest::newRow("Error")        << "Error"          << true << "Force a compilation error with message";
    QTest::newRow("ParmFile")     << "ParmFile"       << true << "Command Line Parameter include file";

}

void TestGamsOption::testOptionImmeidateType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(QString, description);

    QCOMPARE( gamsOption->getOptionDefinition(optionName).valid, valid );
    QCOMPARE( gamsOption->getOptionType(optionName),  optTypeImmediate );
    QCOMPARE( gamsOption->getOptionDefinition(optionName).description, description );
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
    QTest::newRow("PF4")        << "PF4"       << "IntVarUp";
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

    QTest::newRow("AsyncSolLst") << "" << "AsyncSolLst";
    QTest::newRow("Bratio")      << "" << "Bratio";
    QTest::newRow("DomLim")      << "" << "DomLim";
    QTest::newRow("GDX")         << "" << "GDX";
    QTest::newRow("IterLim")     << "" << "IterLim";
    QTest::newRow("Keep")        << "" << "Keep";
    QTest::newRow("gdxConvert")  << "" << "gdxConvert";
    QTest::newRow("ProcDirPath") << "" << "ProcDirPath";
    QTest::newRow("Solver")      << "" << "Solver";
    QTest::newRow("TraceOpt")    << "" << "TraceOpt";

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
    QTest::addColumn<QString>("optionType");
    QTest::addColumn<QString>("optionDescription");

    QTest::newRow("CtrlZ")      << "CtrlZ"      << "integer"  << "Enable reading control Z";
    QTest::newRow("CtrlM")      << "CtrlM"      << "integer"  << "Enable flexible line ending";
    QTest::newRow("PoolUse")    << "PoolUse"    << "integer"  << "Manage memory pools";
    QTest::newRow("PoolFree1")  << "PoolFree1"  << "integer"  << "Manage memory pools N";
    QTest::newRow("PoolFree2")  << "PoolFree2"  << "integer"  << "Manage memory pools N";
    QTest::newRow("PoolFree3")  << "PoolFree3"  << "integer"  << "Manage memory pools N";
    QTest::newRow("PoolFree5")  << "PoolFree5"  << "integer"  << "Manage memory pools N";
    QTest::newRow("PoolFree6")  << "PoolFree6"  << "integer"  << "Manage memory pools N";
    QTest::newRow("PoolFree7")  << "PoolFree7"  << "integer"  << "Manage memory pools N";
    QTest::newRow("CodeX")      << "CodeX"      << "integer"  << "Controls the allocation of executable code";
    QTest::newRow("NlCon")      << "NlCon"      << "integer"  << "Nonlinear instructions search length";
    QTest::newRow("GLanguage")  << "GLanguage"  << "integer"  << "";
    QTest::newRow("GFinclude")  << "GFinclude"  << "integer"  << "";
    QTest::newRow("UnitType")   << "UnitType"   << "string"   << "";
    QTest::newRow("GFImplicit") << "GFImplicit" << "integer"  << "";
    QTest::newRow("GFExt")      << "GFExt"      << "integer"  << "";
    QTest::newRow("SetType")    << "SetType"    << "integer"  << "Used to set the type of --keys";
    QTest::newRow("TopMargin")  << "TopMargin"  << "integer"  << "Output file page top margin";
    QTest::newRow("LeftMargin") << "LeftMargin" << "integer"  << "Output file page left margin";
    QTest::newRow("BotMargin")  << "BotMargin"  << "integer"  << "Output file page bottom margin, lines added at the end of a page";
}

void TestGamsOption::testDeprecatedOption()
{
    QFETCH(QString, deprecatedOption);
    QFETCH(QString, optionType);
    QFETCH(QString, optionDescription);

    QVERIFY( gamsOption->isValid(deprecatedOption) );
    QCOMPARE( gamsOption->getGroupNumber(deprecatedOption), 4 );
    QCOMPARE( gamsOption->getOptionTypeName(gamsOption->getOptionType(deprecatedOption)), optionType );
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
    //QFETCH(QString, optionGroupName);
    QFETCH(QString, optionGroupDescription);
    QFETCH(QString, optionType);

    QCOMPARE( gamsOption->getGroupNumber(optionName), groupNumber );
    QCOMPARE( gamsOption->getGroupDescription(optionName), optionGroupDescription );
    QCOMPARE( gamsOption->getOptionTypeName(gamsOption->getOptionType(optionName)), optionType );
}

void TestGamsOption::testInvalidOption_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("nameValid");
    QTest::addColumn<bool>("synonymValid");

    QTest::newRow("action_valid")      << "action"     << true     << false;
    QTest::newRow("a_valid")           << "a"          << false    << true;
    QTest::newRow("lo_valid")          << "lo"         << false    << true;
    QTest::newRow("reference_valid")   << "reference"  << true     << false;
    QTest::newRow("s_valid")           << "s"          << false    << true;
    QTest::newRow("WDir_valid")        << "WDir"       << false    << true;

    QTest::newRow("CtrlZ_valid")       << "CtrlZ"       << true    << false;
    QTest::newRow("PoolUse_valid")     << "PoolUse"     << true    << false;

    QTest::newRow("HoldFix_invalid")    << "HoldFix"    << false    << false;
    QTest::newRow("InputDir04_invalid") << "InputDir04" << false    << false;
    QTest::newRow("ref_invalid")        << "ref"        << false    << false;
    QTest::newRow("saction_invalid")    << "saction"    << false    << false;
    QTest::newRow("UserDLL_invalid")    << "UserDLL"    << false    << false;
}

void TestGamsOption::testInvalidOption()
{
    QFETCH(QString, optionName);
    QFETCH(bool, nameValid);
    QFETCH(bool, synonymValid);

    QCOMPARE( gamsOption->isValid(optionName), nameValid);
    QCOMPARE( gamsOption->isASynonym(optionName), synonymValid);
}

void TestGamsOption::testTokenize_data()
{
    QTest::addColumn<QString>("commandLineParameter");
    QTest::addColumn<int>("numberOfParameters");
    QTest::addColumn<QList<OptionItem>>("items");
    QTest::addColumn<QList<bool>>("recurrent");

    QTest::newRow("invalid"  )      << "abc=1" << 1
                                    << QList<OptionItem>{ OptionItem(-1, "abc", "1") }
                                    << QList<bool> { false };
    QTest::newRow("synonym_1"  )    << "a=c" << 1
                                    << QList<OptionItem>{ OptionItem(28, "a", "c") }
                                    << QList<bool> { false };
    QTest::newRow("synonym_2"  )    << "a=c action=ce" << 2
                                    << QList<OptionItem>{ OptionItem(28, "a", "c"), OptionItem(28, "action", "ce") }
                                    << QList<bool> { true, true };
    QTest::newRow("synonym_3"  )    << "-a=c action=ce" << 2
                                    << QList<OptionItem>{ OptionItem(28, "-a", "c"), OptionItem(28, "action", "ce") }
                                    << QList<bool> { true, true };
    QTest::newRow("combined_1")     << "action=c gdx abc"        << 2
                                    << QList<OptionItem>{ OptionItem(28, "action", "c"), OptionItem(154, "gdx", "abc") }
                                    << QList<bool> { false, false };
    QTest::newRow("combined_2")     << "-reslim 1 reslim 100"        << 2
                                    << QList<OptionItem>{ OptionItem(157, "-reslim", "1"), OptionItem(157, "reslim", "100") }
                                    << QList<bool> { true, true };
    QTest::newRow("combined_3")     << "action=ce -reslim 1 reslim 100"  << 3
                                    << QList<OptionItem>{ OptionItem(28, "action", "ce"), OptionItem(157, "-reslim", "1"), OptionItem(157, "reslim", "100") }
                                    << QList<bool> { false, true, true };
    QTest::newRow("combined_4")     << "gdx=abc gdx=default reslim 100"  << 3
                                    << QList<OptionItem>{  OptionItem(154, "gdx", "abc"),  OptionItem(154, "gdx", "default"), OptionItem(157, "reslim", "100") }
                                    << QList<bool> { true, true, false };
    QTest::newRow("combined_5")     << "gdx=abc gdx default reslim=100 XX 0.1"  << 4
                                    << QList<OptionItem>{  OptionItem(154, "gdx", "abc"),  OptionItem(154, "gdx", "default"), OptionItem(157, "reslim", "100"), OptionItem(-1, "XX", "0.1") }
                                    << QList<bool> { true, true, false, false };
    QTest::newRow("dash_1")         << "-action=c"               << 1
                                    << QList<OptionItem>{ OptionItem(28, "-action", "c") }
                                    << QList<bool> { false };
    QTest::newRow("dash_2")         << "-a=ce"                   << 1
                                    << QList<OptionItem>{ OptionItem(28, "-a", "ce") }
                                    << QList<bool> { false };
    QTest::newRow("slash_1")        << "/action=ce"              << 1
                                    << QList<OptionItem>{ OptionItem(28, "/action", "ce") }
                                    << QList<bool> { false };
    QTest::newRow("slash_2")        << "/a gt"                   << 1
                                    << QList<OptionItem>{ OptionItem(28, "/a", "gt") }
                                    << QList<bool> { false };
    QTest::newRow("dash_slash_1")   << "-action=c /action=ce"    << 2
                                    << QList<OptionItem>{ OptionItem(28, "-action", "c"), OptionItem(28, "/action", "ce") }
                                    << QList<bool> { true, true };
    QTest::newRow("dash_slash_2")   << "-action=c /action=ce action=gt"   << 3
                                    << QList<OptionItem>{ OptionItem(28, "-action", "c"), OptionItem(28, "/action", "ce"), OptionItem(28, "action", "gt") }
                                    << QList<bool> { true, true, true };
    QTest::newRow("dash_slash_3")   << "-action=c /action=ce -reslim 1000"   << 3
                                    << QList<OptionItem>{ OptionItem(28, "-action", "c"), OptionItem(28, "/action", "ce"), OptionItem(157, "-reslim", "1000") }
                                    << QList<bool> { true, true, false };
    QTest::newRow("doubledash_1")   << "--A 1 --B 2"             << 2
                                    << QList<OptionItem>{ OptionItem(-1, "--A", "1"), OptionItem(-1, "--B", "2") }
                                    << QList<bool> { false, false };
    QTest::newRow("doubledash_2")   << "action=ce --A 1 --B 2"   << 3
                                    << QList<OptionItem>{ OptionItem(28, "action", "ce"), OptionItem(-1, "--A", "1"), OptionItem(-1, "--B", "2") }
                                    << QList<bool> { false, false, false };
    QTest::newRow("doubledash_3")   << "action=ce --A 1 A c"   << 3
                                    << QList<OptionItem>{ OptionItem(28, "action", "ce"), OptionItem(-1, "--A", "1"), OptionItem(28, "A", "c"), }
                                    << QList<bool> { true, false, true };
    QTest::newRow("doubledash_4")   << "action=ce --A 1 -A c"   << 3
                                    << QList<OptionItem>{ OptionItem(28, "action", "ce"), OptionItem(-1, "--A", "1"), OptionItem(28, "-A", "c"), }
                                    << QList<bool> { true, false, true };
}

void TestGamsOption::testTokenize()
{
    QFETCH(QString, commandLineParameter);
    QFETCH(int, numberOfParameters);
    QFETCH(QList<OptionItem>, items);
    QFETCH(QList<bool>, recurrent);

    QList<OptionItem> optionItems = optionTokenizer->tokenize(commandLineParameter);
    QCOMPARE(optionItems.size(), numberOfParameters);
    for(int i=0; i<optionItems.size(); i++) {
        QCOMPARE( optionItems.at(i).key, items.at(i).key );
        QCOMPARE( optionItems.at(i).value, items.at(i).value );
        QCOMPARE( optionItems.at(i).optionId, items.at(i).optionId );
        QCOMPARE( optionItems.at(i).recurrent, recurrent.at(i) );
    }
}

void TestGamsOption::cleanupTestCase()
{
    if (optionTokenizer)
       delete optionTokenizer;
}

QTEST_MAIN(TestGamsOption)
