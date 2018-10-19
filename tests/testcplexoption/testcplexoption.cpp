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
#include <QStandardPaths>

#include "commonpaths.h"
#include "testcplexoption.h"

using gams::studio::CommonPaths;

void TestCPLEXOption::initTestCase()
{
    // given
    const QString expected = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    CommonPaths::setSystemDir(expected.toLatin1());
    // when
    optionTokenizer = new OptionTokenizer(QString("optcplex.def"));
    if  ( !optionTokenizer->getOption()->available() ) {
       QFAIL("expected successful read of optcplex.def, but failed");
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
    QTest::newRow("bttol")       << "bttol"         << true  << 0.0  << 1.0    << 0.9999;
    QTest::newRow("cutsfactor")  << "cutsfactor"    << true  << -1.0 << 1e+075 << -1.0;
    QTest::newRow("divfltlo")    << "divfltlo"      << true  << gams::studio::option::OPTION_VALUE_MINDOUBLE << gams::studio::option::OPTION_VALUE_MAXDOUBLE << gams::studio::option::OPTION_VALUE_MINDOUBLE;
    QTest::newRow("epgap")       << "epgap"         << true  << 0.0  << 1.0    << 0.0001;
    QTest::newRow(".feaspref")   << ".feaspref"     << true  << 0.0  << 1e+020 << 1.0;
    QTest::newRow("miptracetime") << "miptracetime" << true  << 0.0  << gams::studio::option::OPTION_VALUE_MAXDOUBLE  << 1.0;
    QTest::newRow("neteprhs")    << "neteprhs"    << true  << 1e-011 << 0.1 <<  1e-006;
    QTest::newRow("objllim")     << "objllim"     << true  << gams::studio::option::OPTION_VALUE_MINDOUBLE << gams::studio::option::OPTION_VALUE_MAXDOUBLE << -1e+075;
    QTest::newRow("polishafterepgap")  << "polishafterepgap" << true << 0.0 << 1.0    << 0.0;
    QTest::newRow("rampuptimelimit")   << "rampuptimelimit"  << true << 0.0 << 1e+075 << 1e+075;
    QTest::newRow("solnpoolgap")       << "solnpoolgap"      << true << 0.0 << 1e+075 << 1e+075;
    QTest::newRow("tuningdettilim")    << "tuningdettilim"   << true << 1.0 << 1e+075 << 1e+007;
    QTest::newRow("workmem")           << "workmem"          << true << 0.0 << 1e+075 << 128.0;
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
    QTest::newRow("miptracenode")      << "miptracenode"      << true  << 0  << gams::studio::option::OPTION_VALUE_MAXINT << 100;
    QTest::newRow("perlim")            << "perlim"            << true  << 0  << 2100000000                        << 0;
    QTest::newRow("polishafterintsol") << "polishafterintsol" << true  << 0  << 2100000000                        << 2100000000;
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

    QTest::newRow("fixoptfile")    << "fixoptfile"   << true  << "name of option file which is read just before solving the fixed problem"  << "string";
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

    QTest::newRow("writepremps") << "writepremps"   << true  << "string"  << "produce a Cplex LP/MPS/SAV file of the presolved problem";

    QTest::newRow("polishtime")  << "polishtime"    << false << "double"  << "time spent polishing a solution" ;

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

void TestCPLEXOption::testReadOptionFile()
{
    // given
    QFile outputFile(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("cplex.op2"));
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
        QFAIL("expected to open cplex.op2 to write, but failed");

    QTextStream out(&outputFile);
    out << "advind 0" << endl;
    out << "advind 1" << endl;
    out << "* this is a comment line" << endl;
    out << "" << endl;
    out << "cuts 2" << endl;
    out << "aggcutlim=2000000000" << endl;
    out << "benderspartitioninstage 1" << endl;
    out << "dettilim 1e+075" << endl;
    out << "miptrace /This/Is/The/File Name/Of/MIPTrace.File" << endl;
    out << "computeserver https://somewhere.org/" << endl;
    out << "rerun=auto" << endl;
    out << "solnpoolcapacity=1100000000" << endl;
    out << "solnpoolintensity 3" << endl;
    out << "tuning str1, str2, str3";
    out << ", str 4";
    out << ", str 5";
    out << "" << endl;
    outputFile.close();

    // when
    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("cplex.op2");
    QList<OptionItem> items = optionTokenizer->readOptionParameterFile(optFile);

    // then
    QCOMPARE( items.size(), 11 );

    QVERIFY( containKey (items,"advind") );
    QCOMPARE( getValue(items,"advind").toInt(),  QVariant("1").toInt() );

    QVERIFY( containKey (items,"aggcutlim") );
    QCOMPARE( getValue(items,"aggcutlim").toInt(), QVariant("2000000000").toInt() );

    QVERIFY( containKey (items,"benderspartitioninstage") );
    QCOMPARE( getValue(items,"benderspartitioninstage").toInt(), QVariant("1").toInt() );

    QVERIFY( containKey (items,"dettilim") );
    QCOMPARE( getValue(items,"dettilim").toDouble(), QVariant("1e+075").toDouble() );

    QVERIFY( containKey (items,"solnpoolcapacity") );
    QCOMPARE( getValue(items,"solnpoolcapacity").toInt(), QVariant("1100000000").toInt() );

    QVERIFY( containKey (items,"solnpoolintensity") );
    QCOMPARE( getValue(items,"solnpoolintensity").toInt(), QVariant("3").toInt() );

    QVERIFY( containKey (items,"cuts") );
    QCOMPARE( getValue(items,"cuts").toString(),  QVariant("2").toString() );

    QVERIFY( containKey (items,"miptrace") );
    QCOMPARE( getValue(items,"miptrace").toString(), QVariant("/This/Is/The/File Name/Of/MIPTrace.File").toString() );

    QVERIFY( containKey (items,"rerun") );
    QCOMPARE( getValue(items,"rerun").toString(), QVariant("auto").toString() );

    QVERIFY( containKey (items,"computeserver") );
    QCOMPARE( getValue(items,"computeserver").toString(), QVariant("https://somewhere.org/").toString() );

    QVERIFY( containKey (items,"tuning") );
    QCOMPARE( getValue(items,"tuning").toString(), QVariant("str1, str2, str3, str 4, str 5").toString() );
}

void TestCPLEXOption::testNonExistReadOptionFile()
{
    // when
    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("cplex.op012345");
    QList<OptionItem> items = optionTokenizer->readOptionParameterFile(optFile);

    // then
    QCOMPARE( items.size(), 0);
}

void TestCPLEXOption::testWriteOptionFile()
{
    // given
    QList<OptionItem> items;
    items.append(OptionItem("advind", "1"));
    items.append(OptionItem("barepcomp", "1e-008"));
    items.append(OptionItem("dettilim","1e+075"));
    items.append(OptionItem("computeserver", "https://somewhere.org/"));
    items.append(OptionItem("covers", "2"));
    items.append(OptionItem("feasopt", "0"));
    items.append(OptionItem("rerun", "nono"));
    items.append(OptionItem("tuning", "str1, str2, str3"));
    items.append(OptionItem("usergdxin", "Antoni Pumi Rufi"));

    // when
    QVERIFY( optionTokenizer->writeOptionParameterFile(items, QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("cplex.opt") ));

    // then
    QFile inputFile(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("cplex.opt"));
    int i = 0;
    if (inputFile.open(QIODevice::ReadOnly)) {
       QTextStream in(&inputFile);
       while (!in.atEnd()) {
          QStringList strList = in.readLine().split( "=" );

          QVERIFY( containKey (items, strList.at(0)) );
          if ((QString::compare(strList.at(0), "advind", Qt::CaseInsensitive)==0) ||
              (QString::compare(strList.at(0), "covers", Qt::CaseInsensitive)==0) ||
              (QString::compare(strList.at(0), "feasopt", Qt::CaseInsensitive)==0)
             ) {
             QCOMPARE( getValue(items, strList.at(0)).toInt(), strList.at(1).toInt() );
          } else if ((QString::compare(strList.at(0), "dettilim", Qt::CaseInsensitive)==0) ||
                     (QString::compare(strList.at(0), "barepcomp", Qt::CaseInsensitive)==0)) {
              QCOMPARE( getValue(items, strList.at(0)).toDouble(), strList.at(1).toDouble() );
          } else {
              QString value = strList.at(1);
              if (value.startsWith("\""))
                 value = value.right(value.length()-1);
              if (value.endsWith("\""))
                 value = value.left( value.length()-1);
              QCOMPARE( getValue(items, strList.at(0)).toString(), value );
          }
          i++;
       }
       inputFile.close();
    }
    QCOMPARE(i, items.size());
}

void TestCPLEXOption::cleanupTestCase()
{
    if (optionTokenizer)
        delete optionTokenizer;
}

bool TestCPLEXOption::containKey(QList<OptionItem> &items, const QString &key) const
{
    for(OptionItem item : items) {
        if (QString::compare(item.key, key, Qt::CaseInsensitive)==0)
            return true;
    }
    return false;
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

QTEST_MAIN(TestCPLEXOption)
