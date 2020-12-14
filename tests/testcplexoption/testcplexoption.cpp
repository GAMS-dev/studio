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
#include <QStandardPaths>
#include <QtMath>
#include <QTextCodec>

#include "commonpaths.h"
#include "testcplexoption.h"
#include "gclgms.h"
#include "optcc.h"

using gams::studio::CommonPaths;

void TestCPLEXOption::initTestCase()
{
    // given
    const QString expected = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    CommonPaths::setSystemDir(expected.toLatin1());
    // when
    QString optdef = "optcplex.def";
    optionTokenizer = new OptionTokenizer(optdef);
    if  ( !optionTokenizer->getOption()->available() ) {
       QFAIL("expected successful read of optcplex.def, but failed");
    }

    // when
    char msg[GMS_SSSIZE];
    optCreateD(&mOPTHandle, CommonPaths::systemDir().toLatin1(), msg, sizeof(msg));
    if (msg[0] != '\0')
        Dcreated = false;
    else
        Dcreated = true;

    // test cplex for now
    if (optReadDefinition(mOPTHandle, QDir(CommonPaths::systemDir()).filePath(optdef).toLatin1())) {
        optdefRead = false;
    } else {
        optdefRead = true;
    }
}

void TestCPLEXOption::testOptionBooleanType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("defaultValue");
    QTest::addColumn<QString>("description");

    QTest::newRow("record")           << "record"            << false << 0  << "Records invocations of Callable Library routines";
    QTest::newRow("relaxfixedinfeas") << "relaxfixedinfeas"  << true  << 0  << "accept small infeasibilties in the solve of the fixed problem";
    QTest::newRow("sifting")          << "sifting"           << true  << 1  << "switch for sifting from simplex optimization";
    QTest::newRow("solvefinal")       << "solvefinal"        << true  << 1  << "switch to solve the problem with fixed discrete variables";
    QTest::newRow("usercutnewint")    << "usercutnewint"     << true  << 1  << "calls the cut generator if the solver found a new integer feasible solution";
}

void TestCPLEXOption::testOptionBooleanType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, defaultValue);
    QFETCH(QString, description);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeBoolean);
    QCOMPARE( optionTokenizer->getOption()->getDefaultValue(optionName).toInt(), defaultValue );
    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).description, description);
}

void TestCPLEXOption::testOptionStringType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QString>("description");

    QTest::newRow("iatriggerfile") << "iatriggerfile" << false << "file that triggers the reading of a secondary option file";
    QTest::newRow("miptrace")      << "miptrace"      << true  << "filename of MIP trace file";
    QTest::newRow("probread")      << "probread"      << false << "reads a problem from a Cplex file";
    QTest::newRow("readflt")       << "readflt"       << true  << "reads Cplex solution pool filter file";
    QTest::newRow("readparams")    << "readparams"    << false << "read Cplex parameter file";
}

void TestCPLEXOption::testOptionStringType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(QString, description);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeString);
    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).description, description);
}

void TestCPLEXOption::testOptionStrListType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QString>("description");

    QTest::newRow("computeserver") << "computeserver" << true  << "address and port of Cplex remote object server";
    QTest::newRow("objrng")        << "objrng"        << true  << "do objective ranging";
    QTest::newRow("rhsrng")        << "rhsrng"        << true  << "do right-hand-side ranging";
    QTest::newRow("secret")        << "secret"        << false << "pass on secret CPLEX options";
}

void TestCPLEXOption::testOptionStrListType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(QString, description);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeStrList);
    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).description, description);

}

void TestCPLEXOption::testOptionEnumStrValue_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("valueIndex");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<QString>("value");
    QTest::addColumn<QString>("description");

    QTest::newRow("rerun_auto")  << "rerun"  << true  << 0 << false << "auto" << "Automatic";
    QTest::newRow("rerun_yes")   << "rerun"  << true  << 1 << false << "yes"  << "Rerun infeasible models with presolve turned off";
    QTest::newRow("rerun_no")    << "rerun"  << true  << 2 << false << "no"   << "Do not rerun infeasible models";
    QTest::newRow("rerun_nono")  << "rerun"  << true  << 3 << false << "nono" << "Do not rerun infeasible fixed MIP models" ;

    QTest::newRow("cuts_-1")     << "cuts"   << true  << 0 << false << "-1"   << "Do not generate cuts" ;
    QTest::newRow("cuts_0")      << "cuts"   << true  << 1 << false << "0"    << "Determined automatically" ;
    QTest::newRow("cuts_1")      << "cuts"   << true  << 2 << false << "1"    << "Generate cuts moderately" ;
    QTest::newRow("cuts_2")      << "cuts"   << true  << 3 << false << "2"    << "Generate cuts aggressively" ;
    QTest::newRow("cuts_3")      << "cuts"   << true  << 4 << false << "3"    << "Generate cuts very aggressively" ;
    QTest::newRow("cuts_4")      << "cuts"   << true  << 5 << false << "4"    << "Generate cuts highly aggressively" ;
    QTest::newRow("cuts_5")      << "cuts"   << true  << 6 << false << "5"    << "Generate cuts extremely aggressively" ;
    QTest::newRow("cuts_yes")    << "cuts"   << true  << 7 << true  << "yes"  << "Determined automatically. This is a deprecated setting. Please use 1 to 5." ;
    QTest::newRow("cuts_no")     << "cuts"   << true  << 8 << true  << "no"   << "Do not generate cuts. This is a deprecated setting. Please use 0." ;

    QTest::newRow("lpalg_barrier")  << "lpalg"  << false  << 0 << false << "barrier" << "Barrier";
    QTest::newRow("lpalg_default")  << "lpalg"  << false  << 1 << false << "default" << "Automatic";
    QTest::newRow("lpalg_primal")   << "lpalg"  << false  << 2 << false << "primal"  << "Primal Simplex";
    QTest::newRow("lpalg_dual")     << "lpalg"  << false  << 3 << false << "dual"    << "Dual Simplex" ;
    QTest::newRow("lpalg_network")  << "lpalg"  << false  << 4 << false << "network" << "Network Simplex" ;

    QTest::newRow("rerun_auto")  << "rerun"   << true  << 0 << false << "auto"   << "Automatic" ;
    QTest::newRow("rerun_yes")   << "rerun"   << true  << 1 << false << "yes"    << "Rerun infeasible models with presolve turned off" ;
    QTest::newRow("rerun_no")    << "rerun"   << true  << 2 << false << "no"     << "Do not rerun infeasible models" ;
    QTest::newRow("rerun_nono")  << "rerun"   << true  << 3 << false << "nono"   << "Do not rerun infeasible fixed MIP models";
}

void TestCPLEXOption::testOptionEnumStrValue()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, valueIndex);
    QFETCH(bool, hidden);
    QFETCH(QString, value);
    QFETCH(QString, description);

    QCOMPARE( optionTokenizer->getOption()-> getOptionDefinition(optionName).valid, valid );
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeEnumStr );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).hidden, hidden );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).value.toString().toLower(), value.toLower() );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).description.toLower(), description.toLower() );
}

void TestCPLEXOption::testOptionEnumIntType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("numberOfEnumint");
    QTest::addColumn<int>("defaultValue");

    QTest::newRow("solnpoolpop")        << "solnpoolpop"        << true  << 2  << 1;
    QTest::newRow("solnpoolintensity")  << "solnpoolintensity"  << true  << 5  << 0;
    QTest::newRow("startalg")           << "startalg"           << true  << 7  << 0;
    QTest::newRow("tuningmeasure")      << "tuningmeasure"      << true  << 2  << 1;
    QTest::newRow("varsel")             << "varsel"             << true  << 6  << 0;
    QTest::newRow("workeralgorithm")    << "workeralgorithm"    << true  << 6  << 0;
}

void TestCPLEXOption::testOptionEnumIntType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, numberOfEnumint);
    QFETCH(int, defaultValue);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeEnumInt);
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).size() , numberOfEnumint);
    QCOMPARE( optionTokenizer->getOption()->getDefaultValue(optionName).toInt(), defaultValue );
}

void TestCPLEXOption::testOptionEnumIntValue_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("valueIndex");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<int>("value");
    QTest::addColumn<QString>("description");

    QTest::newRow("baralg_0")  << "baralg"  << true  << 0 << false << 0  << "Same as 1 for MIP subproblems, 3 otherwise";
    QTest::newRow("baralg_1")  << "baralg"  << true  << 1 << false << 1  << "Infeasibility-estimate start";
    QTest::newRow("baralg_2")  << "baralg"  << true  << 2 << false << 2  << "Infeasibility-constant start";
    QTest::newRow("baralg_3")  << "baralg"  << true  << 3 << false << 3  << "standard barrier algorithm" ;

    QTest::newRow("bendersstrategy_-1") << "bendersstrategy"  << true  << 0 << false << -1 << "Off";
    QTest::newRow("bendersstrategy_0")  << "bendersstrategy"  << true  << 1 << false << 0  << "Automatic";
    QTest::newRow("bendersstrategy_1")  << "bendersstrategy"  << true  << 2 << false << 1  << "Apply user annotations";
    QTest::newRow("bendersstrategy_2")  << "bendersstrategy"  << true  << 3 << false << 2  << "Apply user annotations with automatic support for subproblems";
    QTest::newRow("bendersstrategy_3")  << "bendersstrategy"  << true  << 4 << false << 3  << "Apply automatic decomposition" ;

    QTest::newRow("qpmethod_0")  << "qpmethod"  << true  << 0 << false << 0  << "Automatic";
    QTest::newRow("qpmethod_1")  << "qpmethod"  << true  << 1 << false << 1  << "Primal Simplex";
    QTest::newRow("qpmethod_2")  << "qpmethod"  << true  << 2 << false << 2  << "Dual Simplex";
    QTest::newRow("qpmethod_3")  << "qpmethod"  << true  << 3 << false << 3  << "Network Simplex";
    QTest::newRow("qpmethod_4")  << "qpmethod"  << true  << 4 << false << 4  << "Barrier";
    QTest::newRow("qpmethod_5")  << "qpmethod"  << true  << 5 << false << 5  << "Sifting";
    QTest::newRow("qpmethod_6")  << "qpmethod"  << true  << 6 << false << 6  << "Concurrent dual, barrier, and primal";

    QTest::newRow("rampupduration_-1")  << "rampupduration"  << true  << 0 << false << -1  << "Turns off ramp up";
    QTest::newRow("rampupduration_0")   << "rampupduration"  << true  << 1 << false << 0   << "Automatic";
    QTest::newRow("rampupduration_1")   << "rampupduration"  << true  << 2 << false << 1   << "Dynamically switch to distributed tree search";
}

void TestCPLEXOption::testOptionEnumIntValue()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, valueIndex);
    QFETCH(bool, hidden);
    QFETCH(int, value);
    QFETCH(QString, description);

    QCOMPARE( optionTokenizer->getOption()-> getOptionDefinition(optionName).valid, valid );
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeEnumInt );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).hidden, hidden );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).value.toInt(), value );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).description.toLower(), description.toLower() );
}

void TestCPLEXOption::testOptionDoubleType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<double>("lowerBound");
    QTest::addColumn<double>("upperBound");
    QTest::addColumn<double>("defaultValue");

    QTest::newRow("barepcomp")   << "barepcomp"     << true  << 1e-012 << 1e+075 << 1e-008;
    QTest::newRow("bttol")       << "bttol"         << true  << 0.0  << 1.0      << 1.0;
    QTest::newRow("cutsfactor")  << "cutsfactor"    << true  << -1.0 << 1e+075   << -1.0;
    QTest::newRow("divfltlo")    << "divfltlo"      << true  << gams::studio::option::OPTION_VALUE_MINDOUBLE << gams::studio::option::OPTION_VALUE_MAXDOUBLE << gams::studio::option::OPTION_VALUE_MINDOUBLE;
    QTest::newRow("epgap")       << "epgap"         << true  << 0.0  << 1.0    << 0.0001;
    QTest::newRow(".feaspref")   << ".feaspref"     << true  << 0.0  << 1e+020 << 1.0;
    QTest::newRow("miptracetime") << "miptracetime" << false  << 0.0  << gams::studio::option::OPTION_VALUE_MAXDOUBLE  << 1.0;
    QTest::newRow("neteprhs")    << "neteprhs"    << true  << 1e-011 << 0.1 <<  1e-006;
    QTest::newRow("objllim")     << "objllim"     << true  << gams::studio::option::OPTION_VALUE_MINDOUBLE << gams::studio::option::OPTION_VALUE_MAXDOUBLE << -1e+075;
    QTest::newRow("polishafterepgap")  << "polishafterepgap" << true << 0.0 << 1.0      << 0.0;
    QTest::newRow("rampuptimelimit")   << "rampuptimelimit"  << true << 0.0 << 1e+075   << 1e+075;
    QTest::newRow("solnpoolgap")       << "solnpoolgap"      << true << 0.0 << 1e+075   << 1e+075;
    QTest::newRow("tuningdettilim")    << "tuningdettilim"   << true << 0.0 << 1.0e+75  << 1.0e+75;
    QTest::newRow("workmem")           << "workmem"          << true << 0.0 << 1.0e+75  << 2048.0;
}

void TestCPLEXOption::testOptionDoubleType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(double, lowerBound);
    QFETCH(double, upperBound);
    QFETCH(double, defaultValue);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeDouble);
    QCOMPARE( optionTokenizer->getOption()->getLowerBound(optionName).toDouble(), lowerBound );
    QCOMPARE( optionTokenizer->getOption()->getUpperBound(optionName).toDouble(), upperBound );
    QCOMPARE( optionTokenizer->getOption()->getDefaultValue(optionName).toDouble(), defaultValue );
}

void TestCPLEXOption::testOptionIntegerType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("lowerBound");
    QTest::addColumn<int>("upperBound");
    QTest::addColumn<int>("defaultValue");

    QTest::newRow("mipstart")          << "mipstart"          << true  << 0  << 6                                 << 0;
    QTest::newRow("perlim")            << "perlim"            << true  << 0  << 2100000000                        << 0;
    QTest::newRow("polishafterintsol") << "polishafterintsol" << true  << 1  << 2147483647                        << 2147483647;
    QTest::newRow("populatelim")       << "populatelim"       << true  << 1  << 2100000000                        << 20;
    QTest::newRow("prepass")           << "prepass"           << true  << -1 << 2100000000                        << -1;
}

void TestCPLEXOption::testOptionIntegerType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, lowerBound);
    QFETCH(int, upperBound);
    QFETCH(int, defaultValue);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeInteger);
    QCOMPARE( optionTokenizer->getOption()->getLowerBound(optionName).toDouble(), lowerBound );
    QCOMPARE( optionTokenizer->getOption()->getUpperBound(optionName).toDouble(), upperBound );
    QCOMPARE( optionTokenizer->getOption()->getDefaultValue(optionName).toDouble(), defaultValue );
}

void TestCPLEXOption::testOptionImmeidateType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QString>("description");

    QTest::newRow("readfile")          << "readfile"          << false << "read secondary option file";
    QTest::newRow("nobounds")          << "nobounds"          << false << "ignores bounds on options";
}

void TestCPLEXOption::testOptionImmeidateType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(QString, description);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeImmediate);
    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).description, description);
}

void TestCPLEXOption::testOptionSynonym_data()
{
    QTest::addColumn<QString>("optionSynonym");
    QTest::addColumn<QString>("optionName");

    QTest::newRow("advbasis")     << "advbasis"     << "advind";
    QTest::newRow("backtracking") << "backtracking" << "bttol";
    QTest::newRow("clonelogs")    << "clonelogs"    << "clonelog";
    QTest::newRow("intsolutionlim") <<  "intsolutionlim" << "intsollim";
    QTest::newRow("mipdisplayint")  << "mipdisplayint"   << "mipinterval";
    QTest::newRow("setobjllim")     << "setobjllim" << "objllim";
    QTest::newRow("presolve")       << "presolve"   << "preind";
    QTest::newRow("variableselect") << "variableselect" << "varsel";
    QTest::newRow("writepremps")    << "writepremps"    << "writepre";

    QTest::newRow("auxrootthreads ") <<  "" << "auxrootthreads";
    QTest::newRow("polishafternode") <<  "" << "polishafternode";
    QTest::newRow("barorder")        <<  "" << "barorder";
    QTest::newRow("bendersstrategy") <<  "" << "bendersstrategy";
    QTest::newRow("cpumask")         <<  "" << "cpumask";
    QTest::newRow("disjcuts")        <<  "" << "disjcuts";
    QTest::newRow("epint")           <<  "" << "epint";
    QTest::newRow("feasoptmode")     <<  "" << "feasoptmode";
    QTest::newRow("heurfreq")        <<  "" << "heurfreq";
    QTest::newRow("lbheur")          <<  "" << "lbheur";
    QTest::newRow("lpmethod")        <<  "" << "lpmethod";
    QTest::newRow("mipdisplay")      <<  "" << "mipdisplay";
    QTest::newRow("netepopt")        <<  "" << "netepopt";
    QTest::newRow("objllim")         <<  "" << "objllim";
    QTest::newRow("polishafterdettime") <<  "" << "polishafterdettime";
    QTest::newRow("qtolin")             <<  "" << "qtolin";
    QTest::newRow("rampuptimelimit") <<  "" << "rampuptimelimit";
    QTest::newRow("siftitlim")       <<  "" << "siftitlim";
    QTest::newRow("tuningdettilim")  <<  "" << "tuningdettilim";
    QTest::newRow("usercutmult")     <<  "" << "usercutmult";
    QTest::newRow("varsel")          <<  "" << "varsel";
    QTest::newRow("workeralgorithm") <<  "" << "workeralgorithm";
    QTest::newRow("zerohalfcut")     <<  "" << "zerohalfcut";
}


void TestCPLEXOption::testOptionSynonym()
{
    QFETCH(QString, optionSynonym);
    QFETCH(QString, optionName);

    if (optionSynonym.isEmpty()) {
        QVERIFY( optionTokenizer->getOption()->getNameFromSynonym(optionSynonym).toUpper().isEmpty() );
        QVERIFY( !optionTokenizer->getOption()->isASynonym(optionName) );
    } else {
       QVERIFY( optionTokenizer->getOption()->isASynonym(optionSynonym) );
       QCOMPARE( optionTokenizer->getOption()->getNameFromSynonym(optionSynonym).toUpper(), optionName.toUpper() );
    }
}

void TestCPLEXOption::testHiddenOption_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<QString>("description");
    QTest::addColumn<QString>("optionType");

    QTest::newRow("dtprefix")      << "dtprefix"     << true  << "prefix for the GDX file for the open nodes"                               << "string";
    QTest::newRow("iafile")        << "iafile"       << true  << "options for interactive option setting come from a file"                  << "string";
    QTest::newRow("secret")        << "secret"       << true  << "pass on secret CPLEX options"                                             << "strlist";
}

void TestCPLEXOption::testHiddenOption()
{
    QFETCH(QString, optionName);
    QFETCH(bool, hidden);
    QFETCH(QString, description);
    QFETCH(QString, optionType);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, !hidden);
    QCOMPARE( optionTokenizer->getOption()->getOptionTypeName(optionTokenizer->getOption()->getOptionType(optionName)), optionType );
}

void TestCPLEXOption::testDeprecatedOption_data()
{
    QTest::addColumn<QString>("deprecatedOption");
    QTest::addColumn<bool>("isASynonym");
    QTest::addColumn<QString>("optionType");
    QTest::addColumn<QString>("optionDescription");

    QTest::newRow("crossoveralg") << "crossoveralg" << false << "enumstr" << "use parameter barcrossalg to select crossover algorithm";
    QTest::newRow("lpalg")        << "lpalg"        << false << "enumstr" << "use parameter lpmethod to specify algorithm used for LP problems";
    QTest::newRow("polishtime")  << "polishtime"    << false << "double"  << "time spent polishing a solution" ;

    QTest::newRow("writepremps") << "writepremps"   << true  << "string"  << "produce a Cplex LP/MPS/SAV file of the presolved problem";
    QTest::newRow("dgradient")   << "dgradient"     << true  << "enumint"  << "dual simplex pricing";
    QTest::newRow("backtracking") << "backtracking" << true   << "double"  << "backtracking limit";
}


void TestCPLEXOption::testDeprecatedOption()
{
    QFETCH(QString, deprecatedOption);
    QFETCH(bool, isASynonym);
    QFETCH(QString, optionType);
    QFETCH(QString, optionDescription);

    if (isASynonym) {
       QVERIFY( !optionTokenizer->getOption()->isValid(deprecatedOption) );
       QVERIFY( optionTokenizer->getOption()->isASynonym(deprecatedOption) );

       QString optionName = optionTokenizer->getOption()->getNameFromSynonym(deprecatedOption);
       QCOMPARE( optionTokenizer->getOption()->getOptionTypeName(optionTokenizer->getOption()->getOptionType(optionName)), optionType );
       QCOMPARE( optionTokenizer->getOption()->getDescription(optionName).toLower(), optionDescription.trimmed().toLower());
       QVERIFY( !optionTokenizer->getOption()->isDeprecated(optionName) );
    } else {
        QVERIFY( !optionTokenizer->getOption()->isValid(deprecatedOption) );
        QVERIFY( !optionTokenizer->getOption()->isASynonym(deprecatedOption) );
        QVERIFY( optionTokenizer->getOption()->isDeprecated(deprecatedOption) );
        QCOMPARE( optionTokenizer->getOption()->getOptionTypeName(optionTokenizer->getOption()->getOptionType(deprecatedOption)), optionType );
        QCOMPARE( optionTokenizer->getOption()->getDescription(deprecatedOption).toLower(), optionDescription.trimmed().toLower());
    }
}

void TestCPLEXOption::testOptionGroup_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<int>("groupNumber");
    QTest::addColumn<QString>("optionGroupName");
    QTest::addColumn<QString>("optionGroupDescription");
    QTest::addColumn<QString>("optionType");

    QTest::newRow("advind_1")         << "advind"         << 1 << "general" << "Preprocessing and General Options" << "enumint";
    QTest::newRow("clocktype_1")      << "clocktype"      << 1 << "general" << "Preprocessing and General Options" << "enumint";
    QTest::newRow("dettilim_1")       << "dettilim"       << 1 << "general" << "Preprocessing and General Options" << "double" ;
    QTest::newRow("freegamsmodel_1")  << "freegamsmodel"  << 1 << "general" << "Preprocessing and General Options" << "boolean";
    QTest::newRow("interactive_1")    << "interactive"    << 1 << "general" << "Preprocessing and General Options" << "boolean";
    QTest::newRow("lpmethod_1")       << "lpmethod"       << 1 << "general" << "Preprocessing and General Options" << "enumint";
    QTest::newRow("memoryemphasis_1") << "memoryemphasis" << 1 << "general" << "Preprocessing and General Options" << "boolean";
    QTest::newRow("names_1")          << "names"          << 1 << "general" << "Preprocessing and General Options" << "boolean";
    QTest::newRow("tuningrepeat_1")   << "tuningrepeat"   << 1 << "general" << "Preprocessing and General Options" << "integer";

    QTest::newRow("dynamicrows_2")    << "dynamicrows"  << 2 << "simplexalg" << "Simplex Algorithmic Options"  << "enumint";
    QTest::newRow("epper_2")          << "epper"        << 2 << "simplexalg" << "Simplex Algorithmic Options"  << "double" ;
    QTest::newRow("pricelim_2")       << "pricelim"     << 2 << "simplexalg" << "Simplex Algorithmic Options"  << "integer";
    QTest::newRow("sifting_2")        << "sifting"      << 2 << "simplexalg" << "Simplex Algorithmic Options"   << "boolean";

    QTest::newRow("itlim_3")          << "itlim"        << 3 << "simplexlim" << "Simplex Limit Options" << "integer";
    QTest::newRow("netitlim_3")       << "netitlim"     << 3 << "simplexlim" << "Simplex Limit Options" << "integer";
    QTest::newRow("objllim_3")        << "objllim"      << 3 << "simplexlim" << "Simplex Limit Options" << "double";
    QTest::newRow("objulim_3")        << "objulim"      << 3 << "simplexlim" << "Simplex Limit Options" << "double";
    QTest::newRow("singlim_3")        << "singlim"      << 3 << "simplexlim" << "Simplex Limit Options" << "integer";

    QTest::newRow("epmrk_4")          << "epmrk"        << 4 << "simplextol" << "Simplex Tolerance Options" << "double";
    QTest::newRow("epopt_4")          << "epopt"        << 4 << "simplextol" << "Simplex Tolerance Options" << "double";
    QTest::newRow("eprhs_4")          << "eprhs"        << 4 << "simplextol" << "Simplex Tolerance Options" << "double";
    QTest::newRow("netepopt_4")       << "netepopt"     << 4 << "simplextol" << "Simplex Tolerance Options" << "double";
    QTest::newRow("neteprhs_4")       << "neteprhs"     << 4 << "simplextol" << "Simplex Tolerance Options" << "double";

    QTest::newRow("baralg _5")        << "baralg"       << 5 << "barrier" << "Barrier Specific Options" << "enumint";
    QTest::newRow("barcrossalg_5")    << "barcrossalg"  << 5 << "barrier" << "Barrier Specific Options" << "enumint";
    QTest::newRow("barstartalg_5")    << "barstartalg"  << 5 << "barrier" << "Barrier Specific Options" << "enumint";

    QTest::newRow("siftalg _6")       << "siftalg"      << 6 << "siftopt" << "Sifting Specific Options" << "enumint";
    QTest::newRow("siftitlim _6")     << "siftitlim"    << 6 << "siftopt" << "Sifting Specific Options" << "integer";

    QTest::newRow(".benderspartition_7")  << ".benderspartition"    << 7 << "mipalg" << "MIP Algorithmic Options" << "integer";
    QTest::newRow("cliques_7")            << "cliques"              << 7 << "mipalg" << "MIP Algorithmic Options" << "enumint";
    QTest::newRow("disjcuts_7")           << "disjcuts"             << 7 << "mipalg" << "MIP Algorithmic Options" << "enumint";
    QTest::newRow("fpheur_7")             << "fpheur"               << 7 << "mipalg" << "MIP Algorithmic Options" << "enumint";
    QTest::newRow("gubcovers_7")          << "gubcovers"            << 7 << "mipalg" << "MIP Algorithmic Options" << "enumint";
    QTest::newRow("heurfreq_7")           << "heurfreq"             << 7 << "mipalg" << "MIP Algorithmic Options" << "integer";
    QTest::newRow("implbd_7")             << "implbd"               << 7 << "mipalg" << "MIP Algorithmic Options" << "enumint";
    QTest::newRow("nodefileind_7")        << "nodefileind"          << 7 << "mipalg" << "MIP Algorithmic Options" << "enumint";
    QTest::newRow("probe_7")              << "probe"                << 7 << "mipalg" << "MIP Algorithmic Options" << "enumint";
    QTest::newRow("qpmakepsdind_7")       << "qpmakepsdind"         << 7 << "mipalg" << "MIP Algorithmic Options" << "boolean";
    QTest::newRow("repeatpresolve_7")     << "repeatpresolve"       << 7 << "mipalg" << "MIP Algorithmic Options" << "enumint";
    QTest::newRow("submipnodelim_7")      << "submipnodelim"        << 7 << "mipalg" << "MIP Algorithmic Options" << "integer";
    QTest::newRow("varse_7")              << "varsel"               << 7 << "mipalg" << "MIP Algorithmic Options" << "enumint";
    QTest::newRow("workeralgorithm_7")    << "workeralgorithm"      << 7 << "mipalg" << "MIP Algorithmic Options" << "enumint";
    QTest::newRow("zerohalfcuts_7")       << "zerohalfcuts"         << 7 << "mipalg" << "MIP Algorithmic Options" << "enumint";

    QTest::newRow("auxrootthreads_8") << "auxrootthreads" << 8 << "miplim" << "MIP Limit Options"    << "integer";
    QTest::newRow("cutpass_8")        << "cutpass"        << 8 << "miplim" << "MIP Limit Options"    << "integer";
    QTest::newRow("fraccand _8")      << "fraccand"       << 8 << "miplim" << "MIP Limit Options"    << "integer";
    QTest::newRow("intsollim _8")     << "intsollim"      << 8 << "miplim" << "MIP Limit Options"    << "integer";
    QTest::newRow("nodelim_8")        << "nodelim"        << 8 << "miplim" << "MIP Limit Options"    << "integer";
    QTest::newRow("probetime_8")      << "probetime"      << 8 << "miplim" << "MIP Limit Options"    << "double";
    QTest::newRow("rampupduration_8") << "rampupduration" << 8 << "miplim" << "MIP Limit Options"    << "enumint";
    QTest::newRow("trelim_8")         << "trelim"         << 8 << "miplim" << "MIP Limit Options"    << "double";

    QTest::newRow(".divflt_9")        << ".divflt"        << 9 << "solpool" << "MIP Solution Pool Options"    << "double";
    QTest::newRow("divfltup_9")       << "divfltup"       << 9 << "solpool" << "MIP Solution Pool Options"    << "double";
    QTest::newRow("populatelim_9")    << "populatelim"    << 9 << "solpool" << "MIP Solution Pool Options"    << "integer";
    QTest::newRow("readflt_9")        << "readflt"        << 9 << "solpool" << "MIP Solution Pool Options"    << "string";
    QTest::newRow("solnpoolmerge_9")  << "solnpoolmerge"  << 9 << "solpool" << "MIP Solution Pool Options"    << "string";
    QTest::newRow("solnpoolpopdel_9") << "solnpoolpopdel" << 9 << "solpool" << "MIP Solution Pool Options"    << "string";

    QTest::newRow("bendersfeascuttol_10")  << "bendersfeascuttol" << 10 << "miptol" << "MIP Tolerance Options"    << "double";
    QTest::newRow("bendersoptcuttol_10")   << "bendersoptcuttol"  << 10 << "miptol" << "MIP Tolerance Options"    << "double";
    QTest::newRow("epagap_10")             << "epagap"            << 10 << "miptol" << "MIP Tolerance Options"    << "double";
    QTest::newRow("epgap_10")              << "epgap"             << 10 << "miptol" << "MIP Tolerance Options"    << "double";
    QTest::newRow("epint_10")              << "epint"             << 10 << "miptol" << "MIP Tolerance Options"    << "double";
    QTest::newRow("objdif_10")             << "objdif"            << 10 << "miptol" << "MIP Tolerance Options"    << "double";
    QTest::newRow("relobjdif_10")          << "objdif"            << 10 << "miptol" << "MIP Tolerance Options"    << "double";

    QTest::newRow("bardisplay_11")   << "bardisplay"         << 11 << "output" << "Output Options"    << "enumint";
    QTest::newRow("clonelog_11")     << "clonelog"           << 11 << "output" << "Output Options"    << "enumint";
    QTest::newRow("miptracenode_11") << "miptracenode"       << 11 << "output" << "Output Options"    << "integer";
    QTest::newRow("netdisplay_11")   << "netdisplay"         << 11 << "output" << "Output Options"    << "enumint";
    QTest::newRow("quality_11")      << "quality"            << 11 << "output" << "Output Options"    << "boolean";
    QTest::newRow("simdisplay_11")   << "simdisplay"         << 11 << "output" << "Output Options"    << "enumint";
    QTest::newRow("writeparam_11")   << "writeparam"         << 11 << "output" << "Output Options"    << "string";

    QTest::newRow("usercutcall_12")  << "usercutcall"        << 12 << "bch" << "BCH Facility Options" << "string";
    QTest::newRow("usergdxin_12")    << "usergdxin"          << 12 << "bch" << "BCH Facility Options" << "string";
    QTest::newRow("userkeep_12")     << "userkeep"           << 12 << "bch" << "BCH Facility Options" << "boolean";

}

void TestCPLEXOption::testOptionGroup()
{
    QFETCH(QString, optionName);
    QFETCH(int, groupNumber);
    QFETCH(QString, optionGroupName);
    QFETCH(QString, optionGroupDescription);
    QFETCH(QString, optionType);

    QCOMPARE( optionTokenizer->getOption()->getGroupNumber(optionName), groupNumber );
    QCOMPARE( optionTokenizer->getOption()->getGroupName(optionName), optionGroupName );
    QCOMPARE( optionTokenizer->getOption()->getGroupDescription(optionName), optionGroupDescription );
    QCOMPARE( optionTokenizer->getOption()->getOptionTypeName(optionTokenizer->getOption()->getOptionType(optionName)), optionType );

}

void TestCPLEXOption::testInvalidOption_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("nameValid");
    QTest::addColumn<bool>("synonymValid");

    QTest::newRow("feasopt_valid")   << "feasopt"   << true     << false;
    QTest::newRow("itlim_valid")     << "itlim"     << true     << false;
    QTest::newRow("advbasis_valid")  << "advbasis"  << false    << true;
    QTest::newRow("opttol_valid")    << "opttol"    << false    << true;
    QTest::newRow("feasibtol_valid") << "feasibtol" << false    << true;
    QTest::newRow("reslim_valid")    << "reslim"    << false    << true;

    QTest::newRow("iiis_invalid")    << "iiis"     << false    << false;
    QTest::newRow("mipstar_invalid") << "mipstar"  << false    << false;
    QTest::newRow("nitlim_invalid")  << "nitlim"   << false    << false;

}

void TestCPLEXOption::testInvalidOption()
{
    QFETCH(QString, optionName);
    QFETCH(bool, nameValid);
    QFETCH(bool, synonymValid);

    QCOMPARE( optionTokenizer->getOption()->isValid(optionName), nameValid);
    QCOMPARE( optionTokenizer->getOption()->isASynonym(optionName), synonymValid);
}

void TestCPLEXOption::testReadOptionFile_data()
{
    // given
    QFile outputFile(QDir(".").absoluteFilePath("cplex.op2"));
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
        QFAIL("expected to open cplex.op2 to write, but failed");

    QTextStream out(&outputFile);
    out << "advind=0" << Qt::endl;                     // integer
    out << "advind -1" << Qt::endl;                    // integer
    out << "*----------------------- " << Qt::endl;
    out << "*  this is a comment  line" << Qt::endl;
    out << "*" << Qt::endl;
    out << "" << Qt::endl;
    out << "cuts 2" << Qt::endl;                       // enumstr
    out << "cost.feaspref 0.9" << Qt::endl;            // dot option
    out << "cost.feaspref(*,*) 4" << Qt::endl;         // dot option
    out << "benderspartitioninstage 1" << Qt::endl;    // boolean
    out << "dettilim 1e+075" << Qt::endl;              // double
    out << "secret def 1 34" << Qt::endl;               // strlist
    out << "computeserver  https://somewhere.org/" << Qt::endl;
    out << "rerun=auto" << Qt::endl;                    // string
    out << "*  eprhs=0.001" << Qt::endl;
    out << "solnpoolcapacity=1100000000" << Qt::endl;   // integer
    out << "* x.benderspartition 4" << Qt::endl;        // dot option
    out << "solnpoolintensity  3" << Qt::endl;          // integer
    out << "tuning str1, str2, str3" << Qt::endl;       // strlist
    out << "tuning str 4";                          // strlist
    out << ", str 5";
    out << "" << Qt::endl;
    out << "indic equ1(i,j,k)$bin1(i,k) 1" << Qt::endl;
    out << "scalex no" << Qt::endl;                      // unknown option
    out << "siftitlim abc" << Qt::endl;                  // integer incorrectvalue
    out << "barqcpepcomp -1.234" << Qt::endl;            // double out of reange value
    out << "barqcpepcomp x" << Qt::endl;                 // double incorrectvalue
    out << "bbinterval 7 1" << Qt::endl;                 // integer too many values
    out << "rerun YES"      << Qt::endl;                 // bool value upper case
    out << "xyz.feaspref(i) 1" << Qt::endl;              // dot option

    out << "advind 2 ! this sets the option to two" << Qt::endl;  // eol comment
    out << "workdir /x/y!/z/a/b"                    << Qt::endl;  // eol comment
    out << "workdir \"/x/y!/z/a/b\""                << Qt::endl;  // eol comment
    out << "epgap  0.00001 ! 1MIP solve criteria to optimal (default is GAMS OptCA/OptCR)"   << Qt::endl;  // eol comment
    out << "epgap  w0.00001 ! 2MIP solve criteria to optimal (default is GAMS OptCA/OptCR)"  << Qt::endl;  // eol comment
    out << "epgap  1E-5!3MIP solve criteria to optimal (default is GAMS OptCA/OptCR)"     << Qt::endl;  // eol comment
    out << "neteprhs 1e-003  ! feasibility tolerance for the network simplex method " << Qt::endl; // eol comment

    out << "varsel 1  asd" << Qt::endl;
    out << "barqcpepcomp   !convergence tolerance for the barrier optimizer for QCPs" << Qt::endl;
    out << "non-exist-option 0" << Qt::endl;
    out << "another-non-exist-option" << Qt::endl;
    out << "epint ;" << Qt::endl;
    out << "record !" << Qt::endl;

    outputFile.close();

    //
    QString optFile = QDir(".").absoluteFilePath("cplex.op2");
    QList<SolverOptionItem *> items = optionTokenizer->readOptionFile(optFile, QTextCodec::codecForLocale());

    // then
    QCOMPARE( items.size(), 41 );

    QTest::addColumn<bool>("optionItem_disabledFlag");
    QTest::addColumn<bool>("disabledFlag");
    QTest::addColumn<QString>("optionItem_optionKey");
    QTest::addColumn<QString>("optionKey");
    QTest::addColumn<QVariant>("optionItem_optionValue");
    QTest::addColumn<QVariant>("optionValue");
    QTest::addColumn<QString>("optionItem_optionText");
    QTest::addColumn<QString>("optionText");
    QTest::addColumn<int>("optionItem_optionId");
    QTest::addColumn<int>("optionId");
    QTest::addColumn<int>("optionItem_error");
    QTest::addColumn<int>("error");

    // comments
    QTest::newRow("*----------------------- ")  << items.at(2)->disabled <<  true
                           << items.at(2)->key      << "*----------------------- "
                           << items.at(2)->value    << QVariant("")
                           << items.at(2)->text     << ""
                           << items.at(2)->optionId << -1
                           << static_cast<int>(items.at(2)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("*")     << items.at(4)->disabled <<  true
                           << items.at(4)->key      << "*"
                           << items.at(4)->value    << QVariant("")
                           << items.at(4)->text     << ""
                           << items.at(4)->optionId << -1
                           << static_cast<int>(items.at(5)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("")     << items.at(5)->disabled <<  true
                           << items.at(5)->key      << ""
                           << items.at(5)->value    << QVariant("")
                           << items.at(5)->text     << ""
                           << items.at(5)->optionId << -1
                           << static_cast<int>(items.at(5)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("*  eprhs=0.001") << items.at(14)->disabled <<  true
                           << items.at(14)->key      << "*  eprhs=0.001"
                           << items.at(14)->value    << QVariant("")
                           << items.at(14)->text     << ""
                           << items.at(14)->optionId << -1
                           << static_cast<int>(items.at(14)->error)    << static_cast<int>(OptionErrorType::No_Error);

    // valid options
    QTest::newRow("advind=0")  << items.at(0)->disabled <<  false
                           << items.at(0)->key      << "advind"
                           << items.at(0)->value    << QVariant("0")
                           << items.at(0)->text     << ""
                           << items.at(0)->optionId << 6
                           << static_cast<int>(items.at(0)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("advind -1")  << items.at(1)->disabled <<  false
                           << items.at(1)->key      << "advind -1"
                           << items.at(1)->value    << QVariant("")
                           << items.at(1)->text     << ""
                           << items.at(1)->optionId << -1
                           << static_cast<int>(items.at(1)->error)    << static_cast<int>(OptionErrorType::Incorrect_Value_Type);
    QTest::newRow("cuts 2")  << items.at(6)->disabled <<  false
                           << items.at(6)->key      << "cuts"
                           << items.at(6)->value    << QVariant("2")
                           << items.at(6)->text     << ""
                           << items.at(6)->optionId << 45
                           << static_cast<int>(items.at(6)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("cost.feaspref 0.9")  << items.at(7)->disabled <<  false
                           << items.at(7)->key      << "cost.feaspref"
                           << items.at(7)->value    << QVariant("0.9")
                           << items.at(7)->text     << ""
                           << items.at(7)->optionId << 74
                           << static_cast<int>(items.at(7)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("cost.feaspref(*,*) 4")  << items.at(8)->disabled <<  false
                           << items.at(8)->key      << "cost.feaspref(*,*)"
                           << items.at(8)->value    << QVariant("4")
                           << items.at(8)->text     << ""
                           << items.at(8)->optionId << 74
                           << static_cast<int>(items.at(8)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("benderspartitioninstage 1")  << items.at(9)->disabled <<  false
                           << items.at(9)->key      << "benderspartitioninstage"
                           << items.at(9)->value    << QVariant("1")
                           << items.at(9)->text     << ""
                           << items.at(9)->optionId << 27
                           << static_cast<int>(items.at(9)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("dettilim 1e+075") << items.at(10)->disabled <<  false
                           << items.at(10)->key      << "dettilim"
                           << items.at(10)->value    << QVariant("1e+075")
                           << items.at(10)->text     << ""
                           << items.at(10)->optionId << 50
                           << static_cast<int>(items.at(10)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("secret def 1 34") << items.at(11)->disabled <<  false
                           << items.at(11)->key      << "secret"
                           << items.at(11)->value    << QVariant("def 1 34")
                           << items.at(11)->text     << ""
                           << items.at(11)->optionId << 182
                           << static_cast<int>(items.at(11)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("computeserver  https://somewhere.org/") << items.at(12)->disabled <<  false
                           << items.at(12)->key      << "computeserver"
                           << items.at(12)->value    << QVariant("https://somewhere.org/")
                           << items.at(12)->text     << ""
                           << items.at(12)->optionId << 38
                           << static_cast<int>(items.at(12)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("rerun=auto") << items.at(13)->disabled <<  false
                           << items.at(13)->key      << "rerun"
                           << items.at(13)->value    << QVariant("auto")
                           << items.at(13)->text     << ""
                           << items.at(13)->optionId << 176
                           << static_cast<int>(items.at(12)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("solnpoolcapacity=1100000000")
                           << items.at(15)->disabled <<  false
                           << items.at(15)->key      << "solnpoolcapacity"
                           << items.at(15)->value    << QVariant("1100000000")
                           << items.at(15)->text     << ""
                           << items.at(15)->optionId << 193
                           << static_cast<int>(items.at(15)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("solnpoolintensity  3") << items.at(17)->disabled <<  false
                           << items.at(17)->key      << "solnpoolintensity"
                           << items.at(17)->value    << QVariant("3")
                           << items.at(17)->text     << ""
                           << items.at(17)->optionId << 195
                           << static_cast<int>(items.at(17)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("tuning str1, str2, str3")
                           << items.at(18)->disabled <<  false
                           << items.at(18)->key      << "tuning"
                           << items.at(18)->value    << QVariant("str1, str2, str3")
                           << items.at(18)->text     << ""
                           << items.at(18)->optionId << 216
                           << static_cast<int>(items.at(18)->error)    << static_cast<int>(OptionErrorType::No_Error);
    // "tuning str 4"                          // strlist
    // ", str 5"
    QTest::newRow("tuning str 4, str 5")
                           << items.at(19)->disabled <<  false
                           << items.at(19)->key      << "tuning"
                           << items.at(19)->value    << QVariant("str 4, str 5")
                           << items.at(19)->text     << ""
                           << items.at(19)->optionId << 216
                           << static_cast<int>(items.at(19)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("indic equ1(i,j,k)$bin1(i,k) 1")
                           << items.at(20)->disabled <<  false
                           << items.at(20)->key      << "indic equ1(i,j,k)$bin1(i,k) 1"
                           << items.at(20)->value    << QVariant("")
                           << items.at(20)->text     << ""
                           << items.at(20)->optionId << -1
                           << static_cast<int>(items.at(20)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("rerun YES")
                           << items.at(26)->disabled <<  false
                           << items.at(26)->key      << "rerun"
                           << items.at(26)->value    << QVariant("YES")
                           << items.at(26)->text     << ""
                           << items.at(26)->optionId << 176
                           << static_cast<int>(items.at(26)->error)    << static_cast<int>(OptionErrorType::No_Error);

    // invalid options
    QTest::newRow("scalex no")         // bool value upper case
                           << items.at(21)->disabled <<  false
                           << items.at(21)->key      << "scalex no"
                           << items.at(21)->value    << QVariant("")
                           << items.at(21)->text     << ""
                           << items.at(21)->optionId << -1
                           << static_cast<int>(items.at(21)->error)    << static_cast<int>(OptionErrorType::UserDefined_Error);
    QTest::newRow("siftitlim abc")     // integer incorrectvalue
                           << items.at(22)->disabled <<  false
                           << items.at(22)->key      << "siftitlim abc"
                           << items.at(22)->value    << QVariant("")
                           << items.at(22)->text     << ""
                           << items.at(22)->optionId << -1
                           << static_cast<int>(items.at(22)->error)    << static_cast<int>(OptionErrorType::Incorrect_Value_Type);
    QTest::newRow("barqcpepcomp -1.234")     // double out of reange value
                           << items.at(23)->disabled <<  false
                           << items.at(23)->key      << "barqcpepcomp"
                           << items.at(23)->value    << QVariant("-1.234")
                           << items.at(23)->text     << ""
                           << items.at(23)->optionId << 21
                           << static_cast<int>(items.at(23)->error)    << static_cast<int>(OptionErrorType::Value_Out_Of_Range);
    QTest::newRow("barqcpepcomp x")     // double incorrectvalue
                           << items.at(24)->disabled <<  false
                           << items.at(24)->key      << "barqcpepcomp x"
                           << items.at(24)->value    << QVariant("")
                           << items.at(24)->text     << ""
                           << items.at(24)->optionId << -1
                           << static_cast<int>(items.at(24)->error)    << static_cast<int>(OptionErrorType::Incorrect_Value_Type);
    QTest::newRow("bbinterval 7 1")     // integer too many values
                           << items.at(25)->disabled <<  false
                           << items.at(25)->key      << "bbinterval 7 1"
                           << items.at(25)->value    << QVariant("")
                           << items.at(25)->text     << ""
                           << items.at(25)->optionId << -1
                           << static_cast<int>(items.at(25)->error)    << static_cast<int>(OptionErrorType::Incorrect_Value_Type);
    QTest::newRow("xyz.feaspref(i) 1")         // dot option
                           << items.at(27)->disabled <<  false
                           << items.at(27)->key      << "xyz.feaspref(i) 1"
                           << items.at(27)->value    << QVariant("")
                           << items.at(27)->text     << ""
                           << items.at(27)->optionId << -1
                           << static_cast<int>(items.at(27)->error)    << static_cast<int>(OptionErrorType::UserDefined_Error);

    // with End Of Line Comments
    QTest::newRow("advind 2 ! this sets the option to two")
                           << items.at(28)->disabled <<  false
                           << items.at(28)->key      << "advind"
                           << items.at(28)->value    << QVariant("2")
                           << items.at(28)->text     << "this sets the option to two"
                           << items.at(28)->optionId << 6
                           << static_cast<int>(items.at(28)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("workdir /x/y!/z/a/b")
                           << items.at(29)->disabled <<  false
                           << items.at(29)->key      << "workdir"
                           << items.at(29)->value    << QVariant("/x/y")
                           << items.at(29)->text     << "/z/a/b"
                           << items.at(29)->optionId << 252
                           << static_cast<int>(items.at(29)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("workdir /x/y!/z/a/b")
                           << items.at(29)->disabled <<  false
                           << items.at(29)->key      << "workdir"
                           << items.at(29)->value    << QVariant("/x/y")
                           << items.at(29)->text     << "/z/a/b"
                           << items.at(29)->optionId << 252
                           << static_cast<int>(items.at(29)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("workdir \"/x/y!/z/a/b\"")
                           << items.at(30)->disabled <<  false
                           << items.at(30)->key      << "workdir"
                           << items.at(30)->value    << QVariant("\"/x/y!/z/a/b\"")
                           << items.at(30)->text     << ""
                           << items.at(30)->optionId << 252
                           << static_cast<int>(items.at(30)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("epgap  0.00001 ! 1MIP solve criteria to optimal (default is GAMS OptCA/OptCR)")
                           << items.at(31)->disabled <<  false
                           << items.at(31)->key      << "epgap"
                           << items.at(31)->value    << QVariant("0.00001")
                           << items.at(31)->text     << "1MIP solve criteria to optimal (default is GAMS OptCA/OptCR)"
                           << items.at(31)->optionId << 65
                           << static_cast<int>(items.at(31)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("epgap  w0.00001 ! 2MIP solve criteria to optimal (default is GAMS OptCA/OptCR)")
                           << items.at(32)->disabled <<  false
                           << items.at(32)->key      << "epgap w0.00001"
                           << items.at(32)->value    << QVariant("")
                           << items.at(32)->text     << "2MIP solve criteria to optimal (default is GAMS OptCA/OptCR)"
                           << items.at(32)->optionId << -1
                           << static_cast<int>(items.at(32)->error)    << static_cast<int>(OptionErrorType::Incorrect_Value_Type);
    QTest::newRow("epgap  1E-5!3MIP solve criteria to optimal (default is GAMS OptCA/OptCR)")
                           << items.at(33)->disabled <<  false
                           << items.at(33)->key      << "epgap"
                           << items.at(33)->value    << QVariant("1E-5")
                           << items.at(33)->text     << "3MIP solve criteria to optimal (default is GAMS OptCA/OptCR)"
                           << items.at(33)->optionId << 65
                           << static_cast<int>(items.at(33)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("neteprhs 1e-003  ! feasibility tolerance for the network simplex method ")
            << items.at(34)->disabled <<  false
            << items.at(34)->key      << "neteprhs"
            << items.at(34)->value    << QVariant("1e-003")
            << items.at(34)->text     << "feasibility tolerance for the network simplex method"
            << items.at(34)->optionId << 120
            << static_cast<int>(items.at(34)->error)    << static_cast<int>(OptionErrorType::No_Error);

    // test negative control flows
    QTest::newRow("varsel 1  asd")
            << items.at(35)->disabled <<  false
            << items.at(35)->key      << "varsel 1  asd"
            << items.at(35)->value    << QVariant("")
            << items.at(35)->text     << ""
            << items.at(35)->optionId << -1
            << static_cast<int>(items.at(35)->error)    << static_cast<int>(OptionErrorType::Incorrect_Value_Type);

    QTest::newRow("barqcpepcomp   !convergence tolerance for the barrier optimizer for QCPs")
            << items.at(36)->disabled <<  false
            << items.at(36)->key      << "barqcpepcomp"
            << items.at(36)->value    << QVariant("")
            << items.at(36)->text     << "convergence tolerance for the barrier optimizer for QCPs"
            << items.at(36)->optionId << -1
            << static_cast<int>(items.at(36)->error)    << static_cast<int>(OptionErrorType::UserDefined_Error);

    QTest::newRow("non-exist-option 0")
            << items.at(37)->disabled <<  false
            << items.at(37)->key      << "non-exist-option 0"
            << items.at(37)->value    << QVariant("")
            << items.at(37)->text     << ""
            << items.at(37)->optionId << -1
            << static_cast<int>(items.at(37)->error)    << static_cast<int>(OptionErrorType::UserDefined_Error);

    QTest::newRow("another-non-exist-option")
            << items.at(38)->disabled <<  false
            << items.at(38)->key      << "another-non-exist-option"
            << items.at(38)->value    << QVariant("")
            << items.at(38)->text     << ""
            << items.at(38)->optionId << -1
            << static_cast<int>(items.at(38)->error)    << static_cast<int>(OptionErrorType::UserDefined_Error);

    QTest::newRow("epint ;")   // incorrect EOL comment char
            << items.at(39)->disabled <<  false
            << items.at(39)->key      << "epint ;"
            << items.at(39)->value    << QVariant("")
            << items.at(39)->text     << ""
            << items.at(39)->optionId << -1
            << static_cast<int>(items.at(39)->error)    << static_cast<int>(OptionErrorType::Incorrect_Value_Type);

    QTest::newRow("record !")   // incorrect EOL comment char
            << items.at(40)->disabled <<  false
            << items.at(40)->key      << "record"
            << items.at(40)->value    << QVariant("")
            << items.at(40)->text     << ""
            << items.at(40)->optionId << -1
            << static_cast<int>(items.at(40)->error)    << static_cast<int>(OptionErrorType::UserDefined_Error);
}

void TestCPLEXOption::testReadOptionFile()
{
    QFETCH(bool, optionItem_disabledFlag);
    QFETCH(bool, disabledFlag);
    QFETCH(QString, optionItem_optionKey);
    QFETCH(QString, optionKey);
    QFETCH(QVariant, optionItem_optionValue);
    QFETCH(QVariant, optionValue);
    QFETCH(QString, optionItem_optionText);
    QFETCH(QString, optionText);
//    QFETCH(int, optionItem_optionId);
//    QFETCH(int, optionId);
    QFETCH(int, optionItem_error);
    QFETCH(int, error);

    QCOMPARE( optionItem_disabledFlag, disabledFlag );
    QCOMPARE( optionItem_optionKey, optionKey );
    QCOMPARE( optionItem_optionValue, optionValue );
    QCOMPARE( optionItem_optionText, optionText );
//    QCOMPARE( optionItem_optionId, optionId );
    QCOMPARE( optionItem_error, error );
}

//void TestCPLEXOption::testReadOptionFile_2()
//{
//    // given
//    QFile outputFile(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("cplex.op3"));
//    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
//        QFAIL("expected to open cplex.op2 to write, but failed");

//    QTextStream out(&outputFile);
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "* ask CPLEX to construct an OPT file tuned for the problem" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "TUNING CPLXTUNE.OPT  " << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "" << endl;
//    out << "* presolve reduce" << endl;
//    out << "* \"0\" no presolve" << endl;
//    out << "* \"1\" presolve (default)" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "*PRESOLVE   1" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "* scale" << endl;
//    out << "* The scale option influences the scaling of the problem matrix. (default = 0)" << endl;
//    out << "* -1 No scaling" << endl;
//    out << "*  0 Standard scaling - An equilibrium scaling method is implemented which is" << endl;
//    out << "*    generally very effective." << endl;
//    out << "*  1 Modified, more aggressive scaling method that can produce improvements on" << endl;
//    out << "*    some problems. This scaling should be used if the problem is observed to" << endl;
//    out << "*    have difficulty staying feasible during the solution process." << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "SCALE -1" << endl;
//    out << "" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "*iterationlim 400000" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "* activate RERUNning with Primal Simplex if presolve detects INF" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "RERUN YES" << endl;
//    out << "* infeas finder" << endl;
//    out << "  iis yes" << endl;
//    out << "" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "* Select Solution algorithm" << endl;
//    out << "*     - default is dual simplex" << endl;
//    out << "*   1 - activate primal simplex" << endl;
//    out << "*   4 - activate barrier interior point" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "*  lpmethod 1" << endl;
//    out << "   lpmethod 4" << endl;
//    out << "" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "* Select Barrier Crossover Algorithm" << endl;
//    out << "*   -1 - No crossover (for testing)" << endl;
//    out << "*    0 - Automatic" << endl;
//    out << "*    1 - Primal" << endl;
//    out << "*    2 - Dual" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "*  barcrossalg -1" << endl;
//    out << "" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "* dump probem" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "* put it out column-wise in MPS format" << endl;
//    out << "*WRITEMPS test.mps" << endl;
//    out << "* row-wise representation" << endl;
//    out << "*WRITELP" << endl;
//    out << "" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "* MIP solve criteria to optimal (default is GAMS OptCA/OptCR)" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "*  epagap 0" << endl;
//    out << " epgap  0.00001" << endl;
//    out << "*  epgap 0" << endl;
//    out << "" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "* Have MIP LP solve done using Barrier" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "  startalg 4" << endl;
//    out << "" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "* Reduce memory use (when running out of memory during solve)" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "memoryemphasis 1" << endl;
//    out << "" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "* Request extra passes at reducing model size (when having memory issues)" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "*aggind 4" << endl;
//    out << "" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "* Invoke parallel Barrier/MIP processing = n-1 CPUs, though even 2 for DUO, and run deterministic" << endl;
//    out << "*---------------------------------------------------------------------------*" << endl;
//    out << "  threads -1" << endl;
//    out << "  parallelmode 1" << endl;
//    out << "  heurfreq 1" << endl;
//    out << "  rinsheur 1" << endl;
//    out << "" << endl;

//    // when
//    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("cplex.op3");
//    QList<SolverOptionItem *> items = optionTokenizer->readOptionFile(optFile);

//    // then
//    QCOMPARE( items.size(), 90 );

//    QCOMPARE( items.at(3)->key, "tuning" );
//    QCOMPARE( items.at(3)->value.toString(), "CPLXTUNE.OPT" );
//    QCOMPARE( items.at(3)->text, "TUNING CPLXTUNE.OPT  " );

//    QCOMPARE( items.at(21)->optionId, -1 );
////    QCOMPARE( items.at(21)->key, "SCALE" );
////    QCOMPARE( items.at(21)->value.toString(), "-1" );
//    QCOMPARE( items.at(21)->text, "SCALE -1" );
//    QCOMPARE( items.at(21)->error, Deprecated_Option );

//    QCOMPARE( items.at(30)->optionId, 176 );
//    QCOMPARE( items.at(30)->key, "rerun" );
//    QCOMPARE( items.at(30)->value.toString(), "YES" );
//    QCOMPARE( items.at(30)->text, "RERUN YES" );
//    QCOMPARE( items.at(30)->error, No_Error );

//    QCOMPARE( items.at(32)->optionId, 89 );
//    QCOMPARE( items.at(32)->key, "iis" );
//    QCOMPARE( items.at(32)->value.toString(), "yes" );
//    QCOMPARE( items.at(32)->text, "  iis yes" );
//    QCOMPARE( items.at(32)->error, No_Error );

//    QCOMPARE( items.at(41)->optionId, 97 );
//    QCOMPARE( items.at(41)->key, "lpmethod" );
//    QCOMPARE( items.at(41)->value.toString(), "4" );
//    QCOMPARE( items.at(41)->text, "   lpmethod 4" );
//    QCOMPARE( items.at(41)->error, No_Error );

//    QCOMPARE( items.at(64)->optionId, 65 );
//    QCOMPARE( items.at(64)->key, "epgap" );
//    QCOMPARE( items.at(64)->value.toString(), "0.00001" );
//    QCOMPARE( items.at(64)->text, " epgap  0.00001" );
//    QCOMPARE( items.at(64)->error, No_Error );

//    QCOMPARE( items.at(70)->optionId, 204 );
//    QCOMPARE( items.at(70)->key, "startalg" );
//    QCOMPARE( items.at(70)->value.toString(), "4" );
//    QCOMPARE( items.at(70)->text, "  startalg 4" );
//    QCOMPARE( items.at(70)->error, No_Error );
//}

void TestCPLEXOption::testNonExistReadOptionFile()
{
    // when
    QString optFile = QDir(".").absoluteFilePath("cplex.op012345");
    QList<SolverOptionItem *> items = optionTokenizer->readOptionFile(optFile, QTextCodec::codecForLocale());

    // then
    QCOMPARE( items.size(), 0);
}

void TestCPLEXOption::testWriteOptionFile_data()
{
    // given
    QList<SolverOptionItem *> items;
    items.append(new SolverOptionItem(-1, "*----------------------- ", "", "", true));
    items.append(new SolverOptionItem(-1, "*  this is a comment  line", "", "", true));
    items.append(new SolverOptionItem(-1, "*", "", "", true));
    items.append(new SolverOptionItem(-1, "", "", "", true));
    items.append(new SolverOptionItem(6, "advind", "1", "", false));
    items.append(new SolverOptionItem(45, "cuts", "2", "", false));
    items.append(new SolverOptionItem(-1, "* -----------------------]", "", "", true));
    items.append(new SolverOptionItem(74, "cost.feaspref", "0.9", "", false));
    items.append(new SolverOptionItem(74, "x.feaspref", "1.0", "", false));
    items.append(new SolverOptionItem(-1, "* -----------------------]", "", "", true));
    items.append(new SolverOptionItem(7, "aggcutlim", "1000000000", "", false));
    items.append(new SolverOptionItem(71, "*  eprhs=0.001", "", "", true));
    items.append(new SolverOptionItem(-1, "", "", "", true));
    items.append(new SolverOptionItem(27, "benderspartitioninstage", "1", "", false));
    items.append(new SolverOptionItem(50, "dettilim", "1e+075", "", false));
    items.append(new SolverOptionItem(176, "* rerun auto", "", "", true));
    items.append(new SolverOptionItem(193, "solnpoolcapacity", "1100000000", "solnpoolcapacity=1100000000", false));
    items.append(new SolverOptionItem(195, "solnpoolintensity", "3", "", false));
    items.append(new SolverOptionItem(216, "tuning", "str1, str2, str3", "tuning str1, str2, str3", false));
    items.append(new SolverOptionItem(216, "tuning", "str 4, str 5", "tuning str 4, str 5", false));
    items.append(new SolverOptionItem(-1, "feasoptmode=4", "", "", false));

    int size = items.size();

    // when
    QVERIFY( optionTokenizer->writeOptionFile(items, QDir(".").absoluteFilePath("cplex.op4"), QTextCodec::codecForLocale()) );

    // clean up
    qDeleteAll(items);
    items.clear();

    // then
    QFile inputFile(QDir(".").absoluteFilePath("cplex.op4"));
    int i = 0;
    QStringList optionItems;

    if (inputFile.open(QIODevice::ReadOnly)) {
       QTextStream in(&inputFile);
       while (!in.atEnd()) {
           optionItems << in.readLine();
           i++ ;
       }
       inputFile.close();
    }

    QCOMPARE( optionItems.size(), size );
    QCOMPARE( i, size );

    QTest::addColumn<QString>("optionString");
    QTest::addColumn<QString>("line");

    QTest::newRow("line0") << optionItems.at(0) <<  "*-----------------------";
    QTest::newRow("line1") << optionItems.at(1) << "* this is a comment line";
    QTest::newRow("line2") << optionItems.at(2) << "";
    QTest::newRow("line3") << optionItems.at(3) << "";
    QTest::newRow("line4") << optionItems.at(4) << "advind=1";
    QTest::newRow("line5") << optionItems.at(5) << "cuts=2";
    QTest::newRow("line6") << optionItems.at(6) << "* -----------------------]";
    QTest::newRow("line7") << optionItems.at(7) << "cost.feaspref=0.9";
    QTest::newRow("line8") << optionItems.at(8) << "x.feaspref=1.0";
    QTest::newRow("line9") << optionItems.at(9) << "* -----------------------]";
    QTest::newRow("line10") << optionItems.at(10) << "aggcutlim=1000000000";
    QTest::newRow("line11") << optionItems.at(11) << "* eprhs=0.001";
    QTest::newRow("line12") << optionItems.at(12) << "";
    QTest::newRow("line13") << optionItems.at(13) << "benderspartitioninstage=1";
    QTest::newRow("line14") << optionItems.at(14) << "dettilim=1e+075";
    QTest::newRow("line15") << optionItems.at(15) << "* rerun auto";
    QTest::newRow("line16") << optionItems.at(16) << "solnpoolcapacity=1100000000 ! solnpoolcapacity=1100000000";
    QTest::newRow("line17") << optionItems.at(17) << "solnpoolintensity=3";
    QTest::newRow("line18") << optionItems.at(18) << "tuning=\"str1, str2, str3\" ! tuning str1, str2, str3";
    QTest::newRow("line19") << optionItems.at(19) << "tuning=\"str 4, str 5\" ! tuning str 4, str 5";
    QTest::newRow("line20") << optionItems.at(20) << "feasoptmode=4";
}

void TestCPLEXOption::testWriteOptionFile()
{
    QFETCH(QString, optionString);
    QFETCH(QString, line);

    QCOMPARE( optionString, line );
}

void TestCPLEXOption::testReadFromStr_data()
{
    QVERIFY( Dcreated && optdefRead );

    QTest::addColumn<QString>("optionStr");
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("defined");
    QTest::addColumn<bool>("recentlyDefined");
    QTest::addColumn<QString>("optionValue");
    QTest::addColumn<bool>("errorRead");
    QTest::addColumn<int>("errorCode");

    // comment
    QTest::newRow("*---------------------------------------------------------------------------*")
            << "*---------------------------------------------------------------------------*"
            << "" << false << false << "" << true << getErrorCode(optMsgUserError);

    QTest::newRow("*iterationlim 400000")  << "*iterationlim 400000"  << ""                     << false << false << "n/a"        << true    << getErrorCode(optMsgUserError);
    QTest::newRow("*  lpmethod 1" )        << "*  lpmethod 1"         << ""                     << false << false << "n/a"        << true    << getErrorCode(optMsgUserError);

    // empty string
    QTest::newRow("                    ")  << "                    "  << "                    " << false << false << "n/a"        << false   << -1;
    QTest::newRow(" ")                     << " "                     << " "                    << false << false << "n/a"        << false   << -1;
    QTest::newRow("" )                     << ""                      << ""                     << false << false << "n/a"        << true    << getErrorCode(optMsgValueWarning);

    QTest::newRow("=" )                    << "="                     << ""                     << false << false << "n/a"        << true    << getErrorCode(optMsgUserError);

    // indicator
    QTest::newRow("indic constr01$y 0")            << "indic constr01$y 0"             << ""    << false  << false  << "n/a"      << false   << -1;
    QTest::newRow("indic equ1(i,j,k)$bin1(i,k) 1") << "indic equ1(i,j,k)$bin1(i,k) 1"  << ""    << false  << false  << "n/a"      << false   << -1;

    // strlist option
    QTest::newRow("secret abc 1 34")       << "secret abc 1 34"       << ""                     << true  << true << "abc 1 34"    << false   << -1;
    QTest::newRow("secret def 1 34")       << "secret def 1 34"       << ""                     << true  << true << "def 1 34"    << false   << -1;

    QTest::newRow("tuning str1 str2 str3")   << "tuning str1 str2 str3"     << "tuning"         << true  << true  << "str1 str2 str3"  << false   << -1;
    QTest::newRow("tuning str4 str5")        << "tuning str4 str5"          << "tuning"         << true  << true  << "str4 str5"       << false   << -1;

    // deprecated option
    QTest::newRow("SCALE -1")              << "SCALE -1"              << "SCALE"                << true  << true << "-1"          << true    << getErrorCode(optMsgDeprecated);
    QTest::newRow("iterationlim 400000")   << "iterationlim 400000"   << "iterationlim"         << true  << true << "400000"      << true    << getErrorCode(optMsgDeprecated);

    // unknown option
    QTest::newRow("unknown -1")            << "unknown -1"            << "unknown"              << false  << false << "n/a"       << true    << getErrorCode(optMsgUserError);
    QTest::newRow("what 0.1234")           << "what 0.1234"           << "what"                 << false  << false << "n/a"       << true    << getErrorCode(optMsgUserError);
    QTest::newRow("indicx e(i,j,k)$bin1(i,k) 2")   << "indicx e(i,j,k)$bin1(i,k) 2"    << ""    << false  << false  << "n/a"      << true   << getErrorCode(optMsgUserError);

    // mssing value
    QTest::newRow("mipinterval")              << "mipinterval"         << "mipinterval"    << false   << false   << "n/a"        << true       << getErrorCode(optMsgUserError);
    QTest::newRow("advind=")                  << "advind="             << "advind"         << false   << false   << "n/a"        << true       << getErrorCode(optMsgUserError);

    // integer option
    QTest::newRow("mipinterval 2")            << "mipinterval 2"          << "mipinterval"       << true    << true     << "2"        << false       << -1;
    QTest::newRow("solnpoolcapacity=11000")   << "solnpoolcapacity=11000" << "solnpoolcapacity"  << true    << true     << "11000"    << false       << -1;
    QTest::newRow("mipinterval -2")           << "mipinterval -2"         << "mipinterval"       << true    << true     << "-2"       << false       << -1;
    QTest::newRow("aggind -3")                << "aggind -3"              << "aggind"            << true    << true     << "-1"       << true        << getErrorCode(optMsgValueError);

    // enumint option
    QTest::newRow("advind=1")                 << "advind=1"               << "advind"            << true    << true     << "1"        << false       << -1;
    QTest::newRow("advind=-1")                << "advind=-1"              << "advind"            << false   << false    << "n/a"      << true        << getErrorCode(optMsgValueError);
    QTest::newRow("advind 0.2")               << "advind 0.2"             << "advind"            << false   << false    << "n/a"      << true        << getErrorCode(optMsgValueError);

    QTest::newRow("  lpmethod 4")             << "  lpmethod 4"          << "lpmethod"           << true    << true     << "4"        << false       << -1;
    QTest::newRow("lpmethod 19")              << "lpmethod 19"           << "lpmethod"           << false   << false    << "n/a"      << true        << getErrorCode(optMsgValueError);

    QTest::newRow("localimplied -1")          << "localimplied -1"       << "localimplied"       << true    << true     << "-1"       << false       << -1;
    QTest::newRow("localimplied 55")          << "localimplied 55"       << "localimplied"       << false   << false     << "n/a"     << true        << getErrorCode(optMsgValueError);

    // boolean option
    QTest::newRow("  iis yes" )                 << "  iis yes"                   << "iis"                      << true    << true     << "1"       << false   << -1;
    QTest::newRow("iis yes")                    << "iis yes"                     << "iis"                      << true    << true     << "1"       << false   << -1;
    QTest::newRow("benderspartitioninstage no") << "benderspartitioninstage no"  << "benderspartitioninstage"  << true    << true     << "0"       << false   << -1;

    // double option
    QTest::newRow("divfltlo 1e-5")         << "divfltlo 1e-5"      << "divfltlo"  << true    << true    << "1e-5"       << false      << -1;
    QTest::newRow(" epgap  0.00001")       << " epgap  0.00001"    << "epgap"     << true    << true    << "0.00001"    << false      << -1;
    QTest::newRow("tilim 2.1")             << "tilim 2.1"          << "epgap"     << true    << true    << "2.1"        << false      << -1;

    QTest::newRow("divfltup abc12-345")   << "divfltup abc12-345"  << "divfltup"  << false   << false   << "n/a"        << true      << getErrorCode(optMsgValueError);
    QTest::newRow("epgap -0.1")           << "epgap -0.1"          << "epgap"     << true    << true    << "0"          << true      << getErrorCode(optMsgValueError);
    QTest::newRow("epint=-0.9")           << "epint=-0.9"          << "epint"     << true    << true    << "0"          << true      << getErrorCode(optMsgValueError);

    // string option
    QTest::newRow("RERUN=YES")             << "RERUN=YES"             << "RERUN"              << true    << true     << "YES"       << false      << -1;
    QTest::newRow("cuts 2")                << "cuts 2"                << "cuts"               << true    << true     << "2"         << false      << -1;

    // dot option
    QTest::newRow("x.feaspref 0.0006")            << "x.feaspref 0.0006"       << ".feaspref"          << true   << true    << "0.0006"    << false      << -1;
    QTest::newRow("x.feaspref 0.0007")            << "x.feaspref 0.0007"       << ".feaspref"          << true   << true    << "0.0007"    << false      << -1;
    QTest::newRow("x.feaspref xxxxx")             << "x.feaspref xxxxx"        << ".feaspref"          << false  << false   << "n/a"       << true       << getErrorCode(optMsgValueError);
    QTest::newRow("y.feaspref(*) 0.0008")         << "y.feaspref(*) 0.0008"    << ".feaspref"          << true   << true    << "0.0008"    << false      << -1;
    QTest::newRow("z.feaspref(*,*) 4")            << "z.feaspref(*,*) 4"        << ".feaspref"         << true   << true    << "4"         << false      << -1;
    QTest::newRow("xyz.benderspartition 3")       << "xyz.benderspartition 3"  << ".benderspartition"  << true   << true    << "3"         << false      << -1;

    QTest::newRow("y.feaspref(i) 0.0008")         << "y.feaspref(i) 0.0008"    << ".feaspref"          << false   << false    << "n/a"      << true       << getErrorCode(optMsgUserError);
    QTest::newRow("*z.feaspref(*,*) 4")           << "*z.feaspref(*,*) 4"      << ".feaspref"          << false   << false   << "n/a"       << true       << getErrorCode(optMsgUserError);
    QTest::newRow("* z.feaspref(*,*) 4")          << "* z.feaspref(*,*) 4"     << ".feaspref"          << false   << false   << "n/a"       << true       << getErrorCode(optMsgUserError);
    QTest::newRow("z.feasssopt 0.0001")           << "z.feasssopt 0.0009"      << ".feasssopt"         << false   << false   << "n/a"       << true       << getErrorCode(optMsgUserError);

    // too many value
    QTest::newRow("feasoptmode 4 0.1")            << "feasoptmode 4 0.1"       << "feasoptmode"        << false    << false   << "n/a"          << true       << getErrorCode(optMsgValueError);
    QTest::newRow("feasoptmode 1 X")              << "feasoptmode 1 X"         << "feasoptmode"        << false    << false   << "n/a"          << true       << getErrorCode(optMsgValueError);

}

void TestCPLEXOption::testReadFromStr()
{
    QFETCH(QString, optionStr);
    QFETCH(QString, optionName);
    QFETCH(bool, defined);
    QFETCH(bool, recentlyDefined);
    QFETCH(QString, optionValue);
    QFETCH(bool, errorRead);
    QFETCH(int, errorCode);

    // given
    optResetAllRecent( mOPTHandle );

    // when
    optReadFromStr(mOPTHandle, optionStr.toLatin1());
    int messageType = logAndClearMessage( mOPTHandle );
    QCOMPARE( messageType, errorCode );
    QCOMPARE( messageType != -1, errorRead );

    // then
    bool readValue = false;
    for (int i = 1; i <= optCount(mOPTHandle); ++i) {
        int idefined, idefinedR, irefnr, itype, iopttype, ioptsubtype;
        optGetInfoNr(mOPTHandle, i, &idefined, &idefinedR, &irefnr, &itype, &iopttype, &ioptsubtype);

        if (idefined || idefinedR) {
            char name[GMS_SSSIZE];
            int group = 0;
            int helpContextNr;
            optGetOptHelpNr(mOPTHandle, i, name, &helpContextNr, &group);

            qDebug() << QString("%1: %2: %3 %4 %5 [%6 %7 %8]").arg(name).arg(i)
                     .arg(idefined).arg(idefinedR).arg(irefnr).arg(itype).arg(iopttype).arg(ioptsubtype);
            QCOMPARE( idefined == 1, defined );
            QCOMPARE( idefinedR == 1, recentlyDefined );

            int ivalue;
            double dvalue;
            char svalue[GMS_SSSIZE];

            optGetValuesNr(mOPTHandle, i, name, &ivalue, &dvalue, svalue);
            bool ok = false;
            switch(itype) {
            case optDataInteger: {  // 1
                qDebug() << QString("%1: %2: dInt %3 %4 %5").arg(name).arg(i).arg(ivalue).arg(dvalue).arg(svalue);
                int i = optionValue.toInt(&ok);
                QVERIFY( ok && i==ivalue );
                readValue = true;
                break;
            }
            case optDataDouble: {  // 2
                qDebug() << QString("%1: %2: dDouble %3 %4 %5").arg(name).arg(i).arg(ivalue).arg(dvalue).arg(svalue);
                double d = optionValue.toDouble(&ok);
                QVERIFY( ok && qFabs(d - dvalue) < 0.000000001 );
                readValue = true;
                break;
            }
            case optDataString: {  // 3
                qDebug() << QString("%1: %2: dString %3 %4 %5").arg(name).arg(i).arg(ivalue).arg(dvalue).arg(svalue);
                QVERIFY( optionValue.compare( QString::fromLatin1(svalue), Qt::CaseInsensitive) == 0);
                readValue = true;
                break;
            }
            case optDataStrList: {  // 4
                QStringList strList;
                for (int j = 1; j <= optListCountStr(mOPTHandle, name ); ++j) {
                   optReadFromListStr( mOPTHandle, name, j, svalue );
                   qDebug() << QString("%1: %2: dStrList #%4 %5").arg(name).arg(i).arg(j).arg(svalue);
                   strList << QString::fromLatin1(svalue);
                }
                QVERIFY( strList.contains(optionValue, Qt::CaseInsensitive) );
                readValue = true;
                break;
            }
            case optDataNone:
            default: break;
            }
            break;
        }

    }
    if (!readValue) {
        QCOMPARE( defined , false );
        QCOMPARE( recentlyDefined, false );
    }
    logAndClearMessage(  mOPTHandle );

    int optcount = -1;
    optDotOptCount(mOPTHandle, &optcount);
    qDebug() << "DOTOptCount:" << optcount;
    char msg[GMS_SSSIZE];
    int ival;
    for (int i = 1; i <= optMessageCount(mOPTHandle); i++ ) {
        optGetMessage( mOPTHandle, i, msg, &ival );
         qDebug() << QString("#DOTMessage: %1 : %2 : %3").arg(i).arg(msg).arg(ival);
    }

    // cleanup
    logAndClearMessage(  mOPTHandle );
    optResetAll(mOPTHandle);
}

void TestCPLEXOption::testEOLChars()
{
    char eolchars[256];
    int numchar = optEOLChars( mOPTHandle, eolchars);

    QCOMPARE( 1, numchar );
    QCOMPARE( "!", QString::fromLatin1(eolchars) );
}

void TestCPLEXOption::cleanupTestCase()
{
    if (mOPTHandle)
        optFree(&mOPTHandle);

    if (optionTokenizer)
        delete optionTokenizer;
}

bool TestCPLEXOption::containKey(QList<SolverOptionItem> &items, const QString &key) const
{
    for(SolverOptionItem item : items) {
        if (QString::compare(item.key, key, Qt::CaseInsensitive)==0)
            return true;
    }
    return false;
}

bool TestCPLEXOption::containKey(QList<OptionItem> &items, const QString &key) const
{
    for(OptionItem item : items) {
        if (QString::compare(item.key, key, Qt::CaseInsensitive)==0)
            return true;
    }
    return false;
}

QVariant TestCPLEXOption::getValue(QList<SolverOptionItem> &items, const QString &key) const
{
    QVariant value;
    for(SolverOptionItem item : items) {
        if (QString::compare(item.key, key, Qt::CaseInsensitive)==0)
            return QVariant(item.value);
    }
    return value;
}

QVariant TestCPLEXOption::getValue(QList<OptionItem> &items, const QString &key) const
{
    QVariant value;
    for(OptionItem item : items) {
        if (QString::compare(item.key, key, Qt::CaseInsensitive)==0)
            return QVariant(item.value);
    }
    return value;
}

int TestCPLEXOption::logAndClearMessage(optHandle_t &OPTHandle)
{
    int messageType = -1;
    int ival;
    char msg[GMS_SSSIZE];
    int count = optMessageCount(OPTHandle);
    for (int i = 1; i <= count; i++ ) {
        optGetMessage( OPTHandle, i, msg, &ival );
        qDebug() << QString("#Message: %1 : %2 : %3").arg(i).arg(msg).arg(ival);
        if (ival !=6 && ival != 7)
            messageType = ival;
    }
    optClearMessages(OPTHandle);
    return messageType;
}

int TestCPLEXOption::getErrorCode(optMsgType type)
{
    return type;
}

QTEST_MAIN(TestCPLEXOption)
