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
#include "testgurobioption.h"

using gams::studio::CommonPaths;

void TestGUROBIOption::initTestCase()
{
    // given
    const QString expected = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    CommonPaths::setSystemDir(expected.toLatin1());
    // when
    optionTokenizer = new OptionTokenizer(QString("optgurobi.def"));
    if  ( !optionTokenizer->getOption()->available() ) {
       QFAIL("expected successful read of optgurobi.def, but failed");
    }
}

void TestGUROBIOption::testOptionBooleanType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("defaultValue");
    QTest::addColumn<QString>("description");

    QTest::newRow("feasopt")     << "feasopt"      << true  << 0  << "Computes a minimum-cost relaxation to make an infeasible model feasible";
    QTest::newRow("iis")         << "iis"          << true  << 0  << "Run the Irreducible Inconsistent Subsystem (IIS) finder if the problem is infeasible";
    QTest::newRow("kappa")       << "kappa"        << true  << 0  << "Display approximate condition number estimates for the optimal simplex basis";
    QTest::newRow("lazyconstraints")  << "lazyconstraints"  << true  << 0  << "Indicator to use lazy constraints";
    QTest::newRow("mipstart")    << "mipstart"     << true  << 0  << "Use mip starting values";
    QTest::newRow("names")       << "names"        << true  << 1  << "Indicator for loading names";
    QTest::newRow("presparsify") << "presparsify"  << true  << 0  << "Enables the presolve sparsify reduction for MIP models";
    QTest::newRow("qcpdual")     << "qcpdual"      << true  << 1  << "Determines whether dual variable values are computed for QCP models";
    QTest::newRow("sensitivity") << "sensitivity"  << true  << 0  << "Provide sensitivity information";
    QTest::newRow("varhint")     << "varhint"      << true  << 0  << "Guide heuristics and branching through variable hints";
}

void TestGUROBIOption::testOptionBooleanType()
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

void TestGUROBIOption::testOptionStringType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QString>("description");

    QTest::newRow("computeserver")  << "computeserver"   << true  << "List of Gurobi compute servers";
    QTest::newRow("fixoptfile")     << "fixoptfile"      << true  << "Option file for fixed problem optimization";
    QTest::newRow("icsecretkey")    << "icsecretkey"     << true  << "The secret key for your Gurobi Instant Cloud license";
    QTest::newRow("miptrace")       << "miptrace"        << false << "Filename of MIP trace file";
    QTest::newRow("nodefiledir")    << "nodefiledir"     << true  << "Nodefile directory";
    QTest::newRow("probread")       << "probread"        << false << "Supply a problem via a Gurobi input file";
    QTest::newRow("readparams")     << "readparams"      << true  << "Read Gurobi parameter file";
    QTest::newRow("solnpool")       << "solnpool"        << true  << "Controls export of alternate MIP solutions";
    QTest::newRow("tuning")         << "tuning"          << true  << "Parameter Tuning";
    QTest::newRow("workerpassword") << "workerpassword"  << true  << "Compute server password Pool of compute servers to use for distributed algorithms";
}

void TestGUROBIOption::testOptionStringType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(QString, description);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeString);
    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).description, description);
}

void TestGUROBIOption::testOptionEnumIntType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("valueIndex");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<int>("value");
    QTest::addColumn<QString>("description");

    QTest::newRow("barhomogeneous_-1") << "barhomogeneous"  << true  << 0 << false << -1  << "Auto";
    QTest::newRow("barhomogeneous_0")  << "barhomogeneous"  << true  << 1 << false << 0   << "Homogeneous Barrier off";
    QTest::newRow("barhomogeneous_1")  << "barhomogeneous"  << true  << 2 << false << 1   << "Force Homogeneous Barrier on";

    QTest::newRow("cliquecuts_-1")  << "cliquecuts"  << true  << 0 << false << -1  << "Auto";
    QTest::newRow("cliquecuts_0")   << "cliquecuts"  << true  << 1 << false << 0   << "Off";
    QTest::newRow("cliquecuts_1")   << "cliquecuts"  << true  << 2 << false << 1   << "Conservative";
    QTest::newRow("cliquecuts_2")   << "cliquecuts"  << true  << 3 << false << 2   << "Aggressive";

    QTest::newRow("disconnected_-1")  << "disconnected"  << true  << 0 << false << -1  << "Auto";
    QTest::newRow("disconnected_0")   << "disconnected"  << true  << 1 << false << 0   << "Ignores structure entirely";
    QTest::newRow("disconnected_1")   << "disconnected"  << true  << 2 << false << 1   << "Conservative";
    QTest::newRow("disconnected_2")   << "disconnected"  << true  << 3 << false << 2   << "Aggressive";

    QTest::newRow("feasoptmode_0")   << "feasoptmode"  << true  << 0 << false << 0   << "Minimize sum of relaxations";
    QTest::newRow("feasoptmode_1")   << "feasoptmode"  << true  << 1 << false << 1   << "Minimize sum of relaxations and optimize";
    QTest::newRow("feasoptmode_2")   << "feasoptmode"  << true  << 2 << false << 2   << "Minimize number of relaxations";
    QTest::newRow("feasoptmode_3")   << "feasoptmode"  << true  << 3 << false << 3   << "Minimize number of relaxations and optimize";
    QTest::newRow("feasoptmode_4")   << "feasoptmode"  << true  << 4 << false << 4   << "Minimize sum of squares of relaxations";
    QTest::newRow("feasoptmode_5")   << "feasoptmode"  << true  << 5 << false << 5   << "Minimize sum of squares of relaxations and optimize";

    QTest::newRow("gubcovercuts_-1")  << "gubcovercuts"  << true  << 0 << false << -1  << "Auto";
    QTest::newRow("gubcovercuts_0")   << "gubcovercuts"  << true  << 1 << false << 0   << "Off";
    QTest::newRow("gubcovercuts_1")   << "gubcovercuts"  << true  << 2 << false << 1   << "Conservative";
    QTest::newRow("gubcovercuts_2")   << "gubcovercuts"  << true  << 3 << false << 2   << "Aggressive";

    QTest::newRow("impliedcuts_-1")  << "impliedcuts"  << true  << 0 << false << -1  << "Auto";
    QTest::newRow("impliedcuts_0")   << "impliedcuts"  << true  << 1 << false << 0   << "Off";
    QTest::newRow("impliedcuts_1")   << "impliedcuts"  << true  << 2 << false << 1   << "Conservative";
    QTest::newRow("impliedcuts_2")   << "impliedcuts"  << true  << 3 << false << 2   << "Aggressive";

    QTest::newRow("mipfocus_0")   << "mipfocus"  << true  << 0 << false << 0   << "Balance between finding good feasible solutions and proving optimality";
    QTest::newRow("mipfocus_1")   << "mipfocus"  << true  << 1 << false << 1   << "Focus towards finding feasible solutions";
    QTest::newRow("mipfocus_2")   << "mipfocus"  << true  << 2 << false << 2   << "Focus towards proving optimality";
    QTest::newRow("mipfocus_3")   << "mipfocus"  << true  << 3 << false << 3   << "Focus on moving the best objective bound";

    QTest::newRow("rerun_-1")  << "rerun"  << true  << 0 << false << -1  << "No";
    QTest::newRow("rerun_0")   << "rerun"  << true  << 1 << false << 0   << "Auto";
    QTest::newRow("rerun_1")   << "rerun"  << true  << 2 << false << 1   << "Yes";

    QTest::newRow("sifting_-1")  << "sifting"  << true  << 0 << false << -1  << "Auto";
    QTest::newRow("sifting_0")   << "sifting"  << true  << 1 << false << 0   << "Off";
    QTest::newRow("sifting_1")   << "sifting"  << true  << 2 << false << 1   << "Moderate";
    QTest::newRow("sifting_2")   << "sifting"  << true  << 3 << false << 2   << "Agressive";

    QTest::newRow("tuneoutput_0")   << "tuneoutput"  << true  << 0 << false << 0   << "No output";
    QTest::newRow("tuneoutput_1")   << "tuneoutput"  << true  << 1 << false << 1   << "Summary output only when a new best parameter set is found";
    QTest::newRow("tuneoutput_2")   << "tuneoutput"  << true  << 2 << false << 2   << "Summary output for each parameter set that is tried";
    QTest::newRow("tuneoutput_3")   << "tuneoutput"  << true  << 3 << false << 3   << "Summary output, plus detailed solver output, for each parameter set tried";

    QTest::newRow("usebasis_0")   << "usebasis"  << true  << 0 << false << 0   << "No basis";
    QTest::newRow("usebasis_1")   << "usebasis"  << true  << 1 << false << 1   << "Supply basis if basis is full otherwise provide primal dual solution";
    QTest::newRow("usebasis_2")   << "usebasis"  << true  << 2 << false << 2   << "Supply basis iff basis is full";
    QTest::newRow("usebasis_3")   << "usebasis"  << true  << 3 << false << 3   << "Supply primal dual solution";
}

void TestGUROBIOption::testOptionEnumIntType()
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

void TestGUROBIOption::testOptionDoubleType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<double>("lowerBound");
    QTest::addColumn<double>("upperBound");
    QTest::addColumn<double>("defaultValue");

    QTest::newRow("barconvtol")     <<  "barconvtol"      << true  << 1e-10  << 1.0                                            << 1e-8;
    QTest::newRow("cstimeoutfrac")  <<  "cstimeoutfrac"   << true  << 0.0    << gams::studio::option::OPTION_VALUE_MAXDOUBLE   << 0.1;
    QTest::newRow("feasibilitytol") <<  "feasibilitytol"  << true  << 1e-9   << 1e-2                                           << 1e-6;
    QTest::newRow(".feaspref")      <<  ".feaspref"       << true  << 0.0    << 1e+020                                         << 1.0;
    QTest::newRow("heuristics")     <<  "heuristics"      << true  << 0.0    << 1.0                                            << 0.05;
    QTest::newRow("improvestartgap")  <<  "improvestartgap"   << true  << 0.0    << gams::studio::option::OPTION_VALUE_MAXDOUBLE   << gams::studio::option::OPTION_VALUE_MAXDOUBLE;
    QTest::newRow("markowitztol")     <<  "markowitztol"      << true  << 1e-4   << 0.999                                          << 0.0078125;
    QTest::newRow("nodefilestart")    <<  "nodefilestart"     << true  << 0.0    << gams::studio::option::OPTION_VALUE_MAXDOUBLE   << gams::studio::option::OPTION_VALUE_MAXDOUBLE;
    QTest::newRow("objscale")         <<  "objscale"          << true  << -1.0   << gams::studio::option::OPTION_VALUE_MAXDOUBLE   << 0.0;
    QTest::newRow("perturbvalue")     <<  "perturbvalue"      << true  << 0.0     << 0.01                                          << 0.0002;
    QTest::newRow(".prior")           <<  ".prior"            << true  << 1.0    << gams::studio::option::OPTION_VALUE_MAXDOUBLE   << 1.0;
    QTest::newRow("timelimit")        <<  "timelimit"         << true  << 0.0    << gams::studio::option::OPTION_VALUE_MAXDOUBLE   << gams::studio::option::OPTION_VALUE_MAXDOUBLE;

}

void TestGUROBIOption::testOptionDoubleType()
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

void TestGUROBIOption::testOptionIntegerType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("lowerBound");
    QTest::addColumn<int>("upperBound");
    QTest::addColumn<int>("defaultValue");

    QTest::newRow("aggregate")      << "aggregate"      << true  << 0    << 1                                         << 1;
    QTest::newRow("bariterlimit")   << "bariterlimit"   << true  << 0    << gams::studio::option::OPTION_VALUE_MAXINT << gams::studio::option::OPTION_VALUE_MAXINT;
    QTest::newRow("concurrentmip")  << "concurrentmip"  << true  << 1    << gams::studio::option::OPTION_VALUE_MAXINT << 1;
    QTest::newRow("degenmoves")     << "degenmoves"     << true  << -1   << gams::studio::option::OPTION_VALUE_MAXINT << -1;
    QTest::newRow(".genconstrtype") << ".genconstrtype" << true  << 0    << 5                                         << 0;
    QTest::newRow("icpriority")     << "icpriority"     << true  << -100 << 100                                       << 0;
    QTest::newRow(".lazy")          << ".lazy"          << true  << 0    << 3                                         << 0;
    QTest::newRow("minrelnodes")    << "minrelnodes"    << true  << 0    << gams::studio::option::OPTION_VALUE_MAXINT << 0;
    QTest::newRow("normadjust")     << "normadjust"     << true  << -1   << 3                                         << -1;
    QTest::newRow("outputflag")     << "outputflag"     << false << 0    << 1                                         << 1;
    QTest::newRow(".partition")     << ".partition"     << true  << -1   << gams::studio::option::OPTION_VALUE_MAXINT << 0;
    QTest::newRow("quad")           << "quad"           << true  << -1   << 1                                         << -1;
    QTest::newRow("rins")           << "rins"           << true  << -1   << gams::studio::option::OPTION_VALUE_MAXINT << -1;
    QTest::newRow("scaleflag")      << "scaleflag"      << true  << 0    << 2                                         << 1;
    QTest::newRow("threads")        << "threads"        << true  << -128 << gams::studio::option::OPTION_VALUE_MAXINT << 0;
    QTest::newRow("workerport")     << "workerport"     << true  << -1   << gams::studio::option::OPTION_VALUE_MAXINT << -1;
    QTest::newRow("zeroobjnodes")   << "zeroobjnodes"   << true  << 0    << gams::studio::option::OPTION_VALUE_MAXINT << 0;
}

void TestGUROBIOption::testOptionIntegerType()
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

void TestGUROBIOption::testOptionGroup_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<int>("groupNumber");
    QTest::addColumn<QString>("optionGroupName");
    QTest::addColumn<QString>("optionGroupDescription");
    QTest::addColumn<QString>("optionType");

    QTest::newRow("bariterlimit_1")         << "bariterlimit"         << 1 << "Termination" << "Termination options" << "integer";
    QTest::newRow("cutoff_1")               << "cutoff"               << 1 << "Termination" << "Termination options" << "double";
    QTest::newRow("iterationlimit_1")       << "iterationlimit"       << 1 << "Termination" << "Termination options" << "double";
    QTest::newRow("nodelimit_1")            << "nodelimit"            << 1 << "Termination" << "Termination options" << "double";
    QTest::newRow("solutionlimit_1")        << "solutionlimit"        << 1 << "Termination" << "Termination options" << "integer";
    QTest::newRow("timelimit_1")            << "timelimit"            << 1 << "Termination" << "Termination options" << "double";

    QTest::newRow("barconvtol_2")      << "barconvtol"      << 2 << "Tolerances" << "Tolerance options"  << "double";
    QTest::newRow("feasibilitytol_2")  << "feasibilitytol"  << 2 << "Tolerances" << "Tolerance options"  << "double";
    QTest::newRow("intfeastol_2")      << "intfeastol"      << 2 << "Tolerances" << "Tolerance options"  << "double";
    QTest::newRow("markowitztol_2")    << "markowitztol"    << 2 << "Tolerances" << "Tolerance options"  << "double";
    QTest::newRow("optimalitytol_2")   << "optimalitytol"   << 2 << "Tolerances" << "Tolerance options"  << "double";
    QTest::newRow("psdtol_2")          << "psdtol"          << 2 << "Tolerances" << "Tolerance options"  << "double";

    QTest::newRow("normadjust_3")      << "normadjust"        << 3 << "Simplex" << "Simplex options"  << "integer";
    QTest::newRow("objscale_3")        << "objscale"          << 3 << "Simplex" << "Simplex options"  << "double";
    QTest::newRow("perturbvalue_3")    << "perturbvalue"      << 3 << "Simplex" << "Simplex options"  << "double";
    QTest::newRow("quad_3")            << "quad"              << 3 << "Simplex" << "Simplex options"  << "integer";
    QTest::newRow("scaleflag_3")       << "scaleflag"         << 3 << "Simplex" << "Simplex options"  << "integer";

    QTest::newRow("barcorrectors_4")   << "barcorrectors"     << 4 << "Barrier" << "Barrier options"  << "integer";
    QTest::newRow("crossover_4")       << "crossover"         << 4 << "Barrier" << "Barrier options"  << "integer";
    QTest::newRow("qcpdual_4")         << "qcpdual"           << 4 << "Barrier" << "Barrier options"  << "boolean";

    QTest::newRow("bestbdstop_5")      << "bestbdstop"        << 5 << "MIPoptions" << "MIP options"  << "double";
    QTest::newRow("cliquecuts_5")      << "cliquecuts"        << 5 << "MIPoptions" << "MIP options"  << "enumint";
    QTest::newRow("degenmoves_5")      << "degenmoves"        << 5 << "MIPoptions" << "MIP options"  << "integer";
    QTest::newRow("flowpathcuts_5")    << "flowpathcuts"      << 5 << "MIPoptions" << "MIP options"  << "enumint";
    QTest::newRow("gomorypasses_5")    << "gomorypasses"      << 5 << "MIPoptions" << "MIP options"  << "integer";
    QTest::newRow("heuristics_5")      << "heuristics"        << 5 << "MIPoptions" << "MIP options"  << "double";
    QTest::newRow("infproofcuts_5")    << "infproofcuts"      << 5 << "MIPoptions" << "MIP options"  << "integer";
    QTest::newRow("lazyconstraints_5") << "lazyconstraints"   << 5 << "MIPoptions" << "MIP options"  << "boolean";
    QTest::newRow("modkcuts_5")        << "modkcuts"          << 5 << "MIPoptions" << "MIP options"  << "integer";
    QTest::newRow("nodemethod_5")      << "nodemethod"        << 5 << "MIPoptions" << "MIP options"  << "enumint";
    QTest::newRow("pumppasses_5")      << "pumppasses"        << 5 << "MIPoptions" << "MIP options"  << "integer";
    QTest::newRow("rins_5")            << "rins"              << 5 << "MIPoptions" << "MIP options"  << "integer";
    QTest::newRow("symmetry_5")        << "symmetry"          << 5 << "MIPoptions" << "MIP options"  << "enumint";
    QTest::newRow("varbranch_5")       << "varbranch"         << 5 << "MIPoptions" << "MIP options"  << "enumint";
    QTest::newRow("zerohalfcuts_5")    << "zerohalfcuts"      << 5 << "MIPoptions" << "MIP options"  << "enumint";

    QTest::newRow("aggfill_6")         << "aggfill"           << 6 << "Other" << "Other options"  << "integer";
    QTest::newRow("computeserver_6")   << "computeserver"     << 6 << "Other" << "Other options"  << "string";
    QTest::newRow("displayinterval_6") << "displayinterval"   << 6 << "Other" << "Other options"  << "integer";
    QTest::newRow("feasopt_6")         << "feasopt"           << 6 << "Other" << "Other options"  << "boolean";
    QTest::newRow("icpool_6")          << "icpool"            << 6 << "Other" << "Other options"  << "string";
    QTest::newRow("kappa_6")           << "kappa"             << 6 << "Other" << "Other options"  << "boolean";
    QTest::newRow("multobj_6")         << "multobj"           << 6 << "Other" << "Other options"  << "boolean";
    QTest::newRow("objnreltol_6")      << "objnreltol"        << 6 << "Other" << "Other options"  << "strlist";
    QTest::newRow("precrush_6")        << "precrush"          << 6 << "Other" << "Other options"  << "integer";
    QTest::newRow("readparams_6")      << "readparams"        << 6 << "Other" << "Other options"  << "string";
    QTest::newRow("tunetrials_6")      << "tunetrials"        << 6 << "Other" << "Other options"  << "integer";
    QTest::newRow("usebasis_6")        << "usebasis"          << 6 << "Other" << "Other options"  << "enumint";
    QTest::newRow("varhint_6")         << "varhint"           << 6 << "Other" << "Other options"  << "boolean";
    QTest::newRow("writeparams_6")     << "writeparams"       << 6 << "Other" << "Other options"  << "string";
}

void TestGUROBIOption::testOptionGroup()
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

void TestGUROBIOption::testReadOptionFile()
{
    // given
    QFile outputFile(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("gurobi.op2"));
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
        QFAIL("expected to open gurobi.op2 to write, but failed");

    QTextStream out(&outputFile);
    out << "cutoff 0.12345" << endl;
    out << "cliquecuts=-1" << endl;
    out << "iterationlimit=120000" << endl;
    out << "barconvtol 1e-9" << endl;
    out << "mipgap 0.10" << endl;
    out << "fixoptfile /This/Is/a/Fix/opt/file" << endl;
    out << "objnreltol str1, str2, str3" << endl;
    outputFile.close();

    // when
    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("gurobi.op2");
    QList<OptionItem> items = optionTokenizer->readOptionParameterFile(optFile);

    // then
    QCOMPARE( items.size(), 7 );

    QVERIFY( containKey (items,"cutoff") );
    QCOMPARE( getValue(items,"cutoff").toDouble(),  QVariant("0.12345").toDouble() );

    QVERIFY( containKey (items,"iterationlimit") );
    QCOMPARE( getValue(items,"iterationlimit").toInt(),  QVariant("120000").toInt() );

    QVERIFY( containKey (items,"cliquecuts") );
    QCOMPARE( getValue(items,"cliquecuts").toInt(),  QVariant("-1").toInt() );

    QVERIFY( containKey (items,"barconvtol") );
    QCOMPARE( getValue(items,"barconvtol").toDouble(),  QVariant("1e-9").toDouble() );

    QVERIFY( containKey (items,"mipgap") );
    QCOMPARE( getValue(items,"mipgap").toDouble(),  QVariant("0.10").toDouble() );

    QVERIFY( containKey (items,"fixoptfile") );
    QCOMPARE( getValue(items,"fixoptfile").toString(),  QVariant("/This/Is/a/Fix/opt/file").toString() );

    QVERIFY( containKey (items,"objnreltol") );
    QCOMPARE( getValue(items,"objnreltol").toString(),  QVariant("str1, str2, str3").toString() );
}

void TestGUROBIOption::testNonExistReadOptionFile()
{
    // when
    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("gurobi.op012345");
    QList<OptionItem> items = optionTokenizer->readOptionParameterFile(optFile);

    // then
    QCOMPARE( items.size(), 0);
}

void TestGUROBIOption::testWriteOptionFile()
{
    // given
    QList<OptionItem> items;
    items.append(OptionItem("cliquecuts", "1"));
    items.append(OptionItem("computeserver", "https://server1/ https://server2/"));
    items.append(OptionItem("intfeastol", "1e-3"));
    items.append(OptionItem("method", "3"));
    items.append(OptionItem("perturbvalue", "0.0012345"));

    // when
    QVERIFY( optionTokenizer->writeOptionParameterFile(items, CommonPaths::defaultWorkingDir(), "gurobi.opt") );

    // then
    QFile inputFile(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("gurobi.opt"));
    int i = 0;
    if (inputFile.open(QIODevice::ReadOnly)) {
       QTextStream in(&inputFile);
       while (!in.atEnd()) {
          QStringList strList = in.readLine().split( "=" );

          QVERIFY( containKey (items, strList.at(0)) );
          if ((QString::compare(strList.at(0), "cliquecuts", Qt::CaseInsensitive)==0) ||
              (QString::compare(strList.at(0), "kappaexact", Qt::CaseInsensitive)==0) ||
              (QString::compare(strList.at(0), "method", Qt::CaseInsensitive)==0)
             ) {
             QCOMPARE( getValue(items, strList.at(0)).toInt(), strList.at(1).toInt() );
          } else if ((QString::compare(strList.at(0), "intfeastol", Qt::CaseInsensitive)==0) ||
                     (QString::compare(strList.at(0), "perturbvalue", Qt::CaseInsensitive)==0)) {
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

void TestGUROBIOption::cleanupTestCase()
{
    if (optionTokenizer)
        delete optionTokenizer;
}

bool TestGUROBIOption::containKey(QList<OptionItem> &items, const QString &key) const
{
    for(OptionItem item : items) {
        if (QString::compare(item.key, key, Qt::CaseInsensitive)==0)
            return true;
    }
    return false;
}

QVariant TestGUROBIOption::getValue(QList<OptionItem> &items, const QString &key) const
{
    QVariant value;
    for(OptionItem item : items) {
        if (QString::compare(item.key, key, Qt::CaseInsensitive)==0)
            return QVariant(item.value);
    }
    return value;
}

QTEST_MAIN(TestGUROBIOption)
