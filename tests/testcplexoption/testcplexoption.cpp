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

using gams::studio::Option;
using gams::studio::CommonPaths;

void TestCPLEXOption::initTestCase()
{
    // given
    const QString expected = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    CommonPaths::setSystemDir(expected.toLatin1());
    // when
    cplexOption = new Option(CommonPaths::systemDir(), "optcplex.def");
    if  ( !cplexOption->available() ) {
       QFAIL("expected successful read of optcplex.def, but failed");
    }
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
}

void TestCPLEXOption::testOptionEnumStrValue()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, valueIndex);
    QFETCH(bool, hidden);
    QFETCH(QString, value);
    QFETCH(QString, description);

    QCOMPARE( cplexOption-> getOptionDefinition(optionName).valid, valid );
    QCOMPARE( cplexOption->getOptionType(optionName),  optTypeEnumStr );
    QCOMPARE( cplexOption->getValueList(optionName).at(valueIndex).hidden, hidden );
    QCOMPARE( cplexOption->getValueList(optionName).at(valueIndex).value.toString().toLower(), value.toLower() );
    QCOMPARE( cplexOption->getValueList(optionName).at(valueIndex).description.toLower(), description.toLower() );
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

    QCOMPARE( cplexOption->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( cplexOption->getOptionType(optionName),  optTypeEnumInt);
    QCOMPARE( cplexOption->getValueList(optionName).size() , numberOfEnumint);
    QCOMPARE( cplexOption->getDefaultValue(optionName).toInt(), defaultValue );
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

// mipstart of optTypeInteger
//    QTest::newRow("mipstart_0")  << "mipstart"  << true  << 0 << false << 0  << "do not use the values";
//    QTest::newRow("mipstart_1")  << "mipstart"  << true  << 1 << false << 1  << "set discrete variable values and use auto mipstart level";
//    QTest::newRow("mipstart_2")  << "mipstart"  << true  << 2 << false << 2  << "set all variable values and use check feasibility mipstart level";
//    QTest::newRow("mipstart_3")  << "mipstart"  << true  << 3 << false << 3  << "set discrete variable values and use solve fixed mipstart level" ;
//    QTest::newRow("mipstart_4")  << "mipstart"  << true  << 4 << false << 4  << "set discrete variable values and use solve sub-MIP mipstart level" ;
//    QTest::newRow("mipstart_5")  << "mipstart"  << true  << 5 << false << 5  << "set discrete variable values and use solve repair-MIP mipstart level" ;
//    QTest::newRow("mipstart_6")  << "mipstart"  << true  << 6 << false << 6  << "set discrete variable values and use no checks at all" ;
}

void TestCPLEXOption::testOptionEnumIntValue()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, valueIndex);
    QFETCH(bool, hidden);
    QFETCH(int, value);
    QFETCH(QString, description);

    QCOMPARE( cplexOption-> getOptionDefinition(optionName).valid, valid );
    QCOMPARE( cplexOption->getOptionType(optionName),  optTypeEnumInt );
    QCOMPARE( cplexOption->getValueList(optionName).at(valueIndex).hidden, hidden );
    QCOMPARE( cplexOption->getValueList(optionName).at(valueIndex).value.toInt(), value );
    QCOMPARE( cplexOption->getValueList(optionName).at(valueIndex).description.toLower(), description.toLower() );

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
    QTest::newRow("divfltlo")    << "divfltlo"      << true  << gams::studio::OPTION_VALUE_MINDOUBLE << gams::studio::OPTION_VALUE_MAXDOUBLE << gams::studio::OPTION_VALUE_MINDOUBLE;
    QTest::newRow("epgap")       << "epgap"         << true  << 0.0  << 1.0    << 0.0001;
    QTest::newRow(".feaspref")   << ".feaspref"     << true  << 0.0  << 1e+020 << 1.0;
    QTest::newRow("miptracetime") << "miptracetime" << true  << 0.0  << gams::studio::OPTION_VALUE_MAXDOUBLE  << 1.0;
    QTest::newRow("neteprhs")    << "neteprhs"    << true  << 1e-011 << 0.1 <<  1e-006;
    QTest::newRow("objllim")     << "objllim"     << true  << gams::studio::OPTION_VALUE_MINDOUBLE << gams::studio::OPTION_VALUE_MAXDOUBLE << -1e+075;
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

    QCOMPARE( cplexOption->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( cplexOption->getOptionType(optionName),  optTypeDouble);
    QCOMPARE( cplexOption->getLowerBound(optionName).toDouble(), lowerBound );
    QCOMPARE( cplexOption->getUpperBound(optionName).toDouble(), upperBound );
    QCOMPARE( cplexOption->getDefaultValue(optionName).toDouble(), defaultValue );
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
        QVERIFY( cplexOption->getNameFromSynonym(optionSynonym).toUpper().isEmpty() );
        QVERIFY( !cplexOption->isASynonym(optionName) );
    } else {
       QVERIFY( cplexOption->isASynonym(optionSynonym) );
       QCOMPARE( cplexOption->getNameFromSynonym(optionSynonym).toUpper(), optionName.toUpper() );
    }
}

void TestCPLEXOption::testDeprecatedOption_data()
{
    QTest::addColumn<QString>("deprecatedOption");
    QTest::addColumn<QString>("optionDescription");

    QTest::newRow("crossoveralg") << "crossoveralg" << "Use option barcrossalg to specify barrier crossover method";
    QTest::newRow("lpalg")        << "lpalg"        << "Use option lpmethod to specify algorithm used for LP problems";

    QTest::newRow("writepremps") << "writepremps" << "Use option writepre to specify file name. File extension determines problem format";

    QTest::newRow("polishtime")  << "polishtime" << "Use option polishafter... for finer solution polishing control.";

}

void TestCPLEXOption::testDeprecatedOption()
{
    QFETCH(QString, deprecatedOption);
    QFETCH(QString, optionDescription);

    QVERIFY( cplexOption->isValid(deprecatedOption) );
    QVERIFY( cplexOption->isDeprecated(deprecatedOption) );
    QCOMPARE( cplexOption->getDescription(deprecatedOption).toLower(), optionDescription.trimmed().toLower());

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

    QCOMPARE( cplexOption->getGroupNumber(optionName), groupNumber );
    QCOMPARE( cplexOption->getGroupName(optionName), optionGroupName );
    QCOMPARE( cplexOption->getGroupDescription(optionName), optionGroupDescription );
    QCOMPARE( cplexOption->getOptionTypeName(cplexOption->getOptionType(optionName)), optionType );

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
    QTest::newRow("reslim_valid")    << "reslim"   << false    << true;

    QTest::newRow("iiis_invalid")    << "iiis"     << false    << false;
    QTest::newRow("mipstar_invalid") << "mipstar"  << false    << false;
    QTest::newRow("nitlim_invalid")  << "nitlim"   << false    << false;

}

void TestCPLEXOption::testInvalidOption()
{
    QFETCH(QString, optionName);
    QFETCH(bool, nameValid);
    QFETCH(bool, synonymValid);

    QCOMPARE( cplexOption->isValid(optionName), nameValid);
    QCOMPARE( cplexOption->isASynonym(optionName), synonymValid);
}

void TestCPLEXOption::cleanupTestCase()
{
    if (cplexOption)
       delete cplexOption;
}

QTEST_MAIN(TestCPLEXOption)
