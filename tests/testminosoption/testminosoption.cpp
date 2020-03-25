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
#include <QTextCodec>

#include "commonpaths.h"
#include "testminosoption.h"
#include "gclgms.h"
#include "optcc.h"
using gams::studio::CommonPaths;

void TestMINOSOption::initTestCase()
{
    // given
    const QString expected = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    CommonPaths::setSystemDir(expected.toLatin1());
    // when
    QString optdef = "optminos.def";
    optionTokenizer = new OptionTokenizer(optdef);
    if  ( !optionTokenizer->getOption()->available() ) {
       QFAIL("expected successful read of optminos.def, but failed");
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

void TestMINOSOption::testOptionStringType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QString>("defvalue");
    QTest::addColumn<QString>("description");

    QTest::newRow("scale print nv")                << "scale print"                << true << "" << "Print scaling factors";
    QTest::newRow("LU partial pivoting nv")        << "LU partial pivoting"        << true << "" << "LUSOL pivoting strategy";
    QTest::newRow("scale no nv")                   << "scale no"                   << true << "" << "Synonym to scale option 0";
    QTest::newRow("Verify_objective_gradients nv") << "Verify objective gradients" << true << "" << "Synonym to verify level 1";
}

void TestMINOSOption::testOptionStringType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(QString, defvalue);
    QFETCH(QString, description);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeString);
    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).description, description);
    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).defaultValue, defvalue);
}

void TestMINOSOption::testOptionEnumStrValue_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("valueIndex");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<QString>("value");
    QTest::addColumn<QString>("defValue");
    QTest::addColumn<QString>("description");

    QTest::newRow("completion FULL")      << "completion"  << true  << 0 << false << "FULL"    << "FULL" << "Solve subproblems to full accuracy";
    QTest::newRow("completion PARTIAL")   << "completion"  << true  << 1 << false << "PARTIAL" << "FULL" << "Solve subproblems to moderate accuracy";

    QTest::newRow("lagrangian NO")    << "lagrangian"  << true  << 0 << false << "NO"      << "YES"  << "Nondefault value (not recommended)";
    QTest::newRow("lagrangian YES")   << "lagrangian"  << true  << 1 << false << "YES"     << "YES"  << "Default value (recommended)";

    QTest::newRow("solution NO")    << "solution"  << true  << 0 << false << "NO"    << "NO"  << "Turn off printing of solution";
    QTest::newRow("solution YES")   << "solution"  << true  << 1 << false << "YES"   << "NO"  << "Turn on printing of solution";

    QTest::newRow("start assigned nonlinears SUPERBASIC")         << "start assigned nonlinears"  << true  << 0 << false << "SUPERBASIC"          << "SUPERBASIC" << "Default";
    QTest::newRow("start assigned nonlinears BASIC")              << "start assigned nonlinears"  << true  << 1 << false << "BASIC"               << "SUPERBASIC" << "Good for square systems";
    QTest::newRow("start assigned nonlinears NONBASIC")           << "start assigned nonlinears"  << true  << 2 << false << "NONBASIC"            << "SUPERBASIC" << "";
    QTest::newRow("start assigned nonlinears ELIGIBLE FOR CRASH") << "start assigned nonlinears"  << true  << 3 << false << "ELIGIBLE FOR CRASH"  << "SUPERBASIC" << "";

}

void TestMINOSOption::testOptionEnumStrValue()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, valueIndex);
    QFETCH(bool, hidden);
    QFETCH(QString, value);
    QFETCH(QString, defValue);
    QFETCH(QString, description);

    QCOMPARE( optionTokenizer->getOption()-> getOptionDefinition(optionName).valid, valid );
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeEnumStr );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).hidden, hidden );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).value.toString().toLower(), value.toLower() );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).description.toLower(), description.toLower() );
    QCOMPARE( optionTokenizer->getOption()-> getOptionDefinition(optionName).defaultValue.toString(), defValue );
}

void TestMINOSOption::testOptionEnumIntValue_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("valueIndex");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<int>("value");
    QTest::addColumn<int>("defValue");
    QTest::addColumn<QString>("description");

    QTest::newRow("crash option 0")  << "crash option"  << true  << 0 << false << 0  << 3 << "Initial basis will be a slack basis"  ;
    QTest::newRow("crash option 1")  << "crash option"  << true  << 1 << false << 1  << 3 << "All columns are eligible";
    QTest::newRow("crash option 2")  << "crash option"  << true  << 2 << false << 2  << 3 << "Only linear columns are eligible";
    QTest::newRow("crash option 3")  << "crash option"  << true  << 3 << false << 3  << 3 << "Columns appearing nonlinearly in the objective are not eligible";
    QTest::newRow("crash option 4")  << "crash option"  << true  << 4 << false << 4  << 3 << "Columns appearing nonlinearly in the constraints are not eligible";

    QTest::newRow("scale option 0")  << "scale option"  << true  << 0 << false << 0  << 1 << "No scaling";
    QTest::newRow("scale option 1")  << "scale option"  << true  << 1 << false << 1  << 1 << "Scale linear variables";
    QTest::newRow("scale option 2")  << "scale option"  << true  << 2 << false << 2  << 1 << "Scale linear + nonlinear variables";

    QTest::newRow("verify level 0")  << "verify level"  << true  << 0 << false << 0  << 0 << "Cheap test";
    QTest::newRow("verify level 1")  << "verify level"  << true  << 1 << false << 1  << 0 << "Check objective";
    QTest::newRow("verify level 2")  << "verify level"  << true  << 2 << false << 2  << 0 << "Check Jacobian";
    QTest::newRow("verify level 3")  << "verify level"  << true  << 3 << false << 3  << 0 << "Check objective and Jacobian";
    QTest::newRow("verify level -1") << "verify level"  << true  << 4 << false << -1 << 0 << "No check";
}

void TestMINOSOption::testOptionEnumIntValue()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, valueIndex);
    QFETCH(bool, hidden);
    QFETCH(int, value);
    QFETCH(QString, description);
    QFETCH(int, defValue);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, valid );
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeEnumInt );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).hidden, hidden );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).value.toInt(), value );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).description.toLower(), description.toLower() );
    QCOMPARE( optionTokenizer->getOption()-> getOptionDefinition(optionName).defaultValue.toInt(), defValue );
}

void TestMINOSOption::testOptionDoubleType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<double>("lowerBound");
    QTest::addColumn<double>("upperBound");
    QTest::addColumn<double>("defaultValue");

    QTest::newRow("crash_tolerance")         << "crash tolerance"          << true  << 0.0 << 1.0 << 0.1;
    QTest::newRow("feasibility_tolerance")   << "feasibility tolerance"    << true  << 0.0 << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 1.0e-6;
    QTest::newRow("linesearch_tolerance")    << "linesearch tolerance"     << true  << 0.0 << 1.0 << 0.1;
    QTest::newRow("scale_print_tolerance")   << "scale print tolerance"    << true  << 0.0 << 1.0 << 0.9;
    QTest::newRow("unbounded_step_size")     << "unbounded step size"      << true  << 0.0 << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 1.0e10;
    QTest::newRow("unbounded_step_size")     << "unbounded step size"      << true  << 0.0 << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 1.0e10;
    QTest::newRow("major_damping_parameter")      << "major damping parameter"    << true  << 0.0 << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 2.0;
    QTest::newRow("radius_of_convergence")        << "radius of convergence"      << true  << 0.0 << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 0.01;
    QTest::newRow("weight_on_linear_objective")   << "weight on linear objective" << true  << 0.0 << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 0.0;
}

void TestMINOSOption::testOptionDoubleType()
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

void TestMINOSOption::testOptionIntegerType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("lowerBound");
    QTest::addColumn<int>("upperBound");
    QTest::addColumn<int>("defaultValue");

    QTest::newRow("debug level")               << "debug level"              << true   << 0  << gams::studio::option::OPTION_VALUE_MAXINT << 0;
    QTest::newRow("factorization frequency")   << "factorization frequency"  << true   << 1  << gams::studio::option::OPTION_VALUE_MAXINT << 100;
    QTest::newRow("hessian dimension")         << "hessian dimension"        << true   << 1  << gams::studio::option::OPTION_VALUE_MAXINT << 1;
    QTest::newRow("log frequency")             << "log frequency"            << true   << 1  << gams::studio::option::OPTION_VALUE_MAXINT << 100;
    QTest::newRow("major iterations")          << "major iterations"         << true   << 0  << gams::studio::option::OPTION_VALUE_MAXINT << 50;
    QTest::newRow("solution file")             << "solution file"            << false  << 0  << gams::studio::option::OPTION_VALUE_MAXINT << 0;
    QTest::newRow("print level")               << "print level"              << true   << 0  << gams::studio::option::OPTION_VALUE_MAXINT << 0;
}

void TestMINOSOption::testOptionIntegerType()
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

void TestMINOSOption::testHiddenOption_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<QString>("description");
    QTest::addColumn<QString>("optionType");

    QTest::newRow("secret")  << "secret"  << true  << "back door for secret or undocumented MINOS options"  << "strlist";
    QTest::newRow("begin")   << "begin"   << true  << "For compatibility with old MINOS spec files"         << "string";
    QTest::newRow("end")     << "end"     << true  << "For compatibility with old MINOS spec files"         << "string";
}

void TestMINOSOption::testHiddenOption()
{
    QFETCH(QString, optionName);
    QFETCH(bool, hidden);
    QFETCH(QString, description);
    QFETCH(QString, optionType);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, !hidden);
    QCOMPARE( optionTokenizer->getOption()->getOptionTypeName(optionTokenizer->getOption()->getOptionType(optionName)), optionType );
}

void TestMINOSOption::testOptionGroup_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<int>("groupNumber");
    QTest::addColumn<QString>("optionGroupName");
    QTest::addColumn<QString>("optionGroupDescription");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<QString>("optionType");

    QTest::newRow("debug level 1")         << "debug level"         << 1 << "output" << "Output related options" << false << "integer" ;
    QTest::newRow("log frequency 1")       << "log frequency"       << 1 << "output" << "Output related options" << false << "integer";
    QTest::newRow("print level 1")         << "print level"         << 1 << "output" << "Output related options" << false << "integer";
    QTest::newRow("solution 1")            << "solution"            << 1 << "output" << "Output related options" << false << "enumstr";
    QTest::newRow("summary frequency 1")   << "summary frequency"   << 1 << "output" << "Output related options" << false << "integer";

    QTest::newRow("crash tolerance 2")         << "crash tolerance"        << 2 << "tolerances" << "Tolerances"  << false << "double";
    QTest::newRow("feasibility tolerance 2")   << "feasibility tolerance"  << 2 << "tolerances" << "Tolerances"  << false << "double";
    QTest::newRow("linesearch tolerance 2")    << "linesearch tolerance"   << 2 << "tolerances" << "Tolerances"  << false << "double";
    QTest::newRow("LU density tolerance 2")    << "LU density tolerance"   << 2 << "tolerances" << "Tolerances"  << false << "double";
    QTest::newRow("optimality tolerance 2")    << "optimality tolerance"   << 2 << "tolerances" << "Tolerances"  << false << "double";
    QTest::newRow("row tolerance 2")           << "row tolerance"          << 2 << "tolerances" << "Tolerances"  << false << "double";
    QTest::newRow("scale print tolerance 2")   << "scale print tolerance"  << 2 << "tolerances" << "Tolerances"  << false << "double";

    QTest::newRow("hessian dimension 3")         << "hessian dimension"         << 3 << "limits" << "Limits" << false << "integer";
    QTest::newRow("iterations limit 3")          << "iterations limit"          << 3 << "limits" << "Limits" << false << "integer";
    QTest::newRow("major iterations 3")          << "major iterations"          << 3 << "limits" << "Limits" << false << "integer";
    QTest::newRow("superbasics limit 3")         << "superbasics limit"         << 3 << "limits" << "Limits" << false << "integer";
    QTest::newRow("unbounded objective value 3") << "unbounded objective value" << 3 << "limits" << "Limits" << false << "double";

    QTest::newRow("check frequency 4")          << "check frequency"          << 4 << "other" << "Other algorithmic options" << false << "integer";
    QTest::newRow("expand frequency 4")         << "expand frequency"         << 4 << "other" << "Other algorithmic options" << false << "integer";
    QTest::newRow("factorization frequency 4")  << "factorization frequency"  << 4 << "other" << "Other algorithmic options" << false << "integer";
    QTest::newRow("lagrangian 4")               << "lagrangian"               << 4 << "other" << "Other algorithmic options" << false << "enumstr";
    QTest::newRow("major damping parameter 4")  << "major damping parameter"  << 4 << "other" << "Other algorithmic options" << false << "double";
    QTest::newRow("partial price 4")               << "partial price"               << 4 << "other" << "Other algorithmic options" << false << "integer";
    QTest::newRow("radius of convergence 4")       << "radius of convergence"       << 4 << "other" << "Other algorithmic options" << false << "double";
    QTest::newRow("scale all variables 4")         << "scale all variables"         << 4 << "other" << "Other algorithmic options" << false << "string";
    QTest::newRow("start assigned nonlinears 4")   << "start assigned nonlinears"   << 4 << "other" << "Other algorithmic options" << false << "enumstr";
    QTest::newRow("verify constraint gradients 4") << "verify constraint gradients" << 4 << "other" << "Other algorithmic options" << false << "string";
    QTest::newRow("weight on linear objective 4")  << "weight on linear objective"  << 4 << "other" << "Other algorithmic options" << false << "double";

    QTest::newRow("begin 5")        << "begin"        << 5 << "compatibility" << "For compatibility with older option files" << true << "string";
    QTest::newRow("end 5")          << "end"          << 5 << "compatibility" << "For compatibility with older option files" << true << "string";

}

void TestMINOSOption::testOptionGroup()
{
    QFETCH(QString, optionName);
    QFETCH(int, groupNumber);
    QFETCH(QString, optionGroupName);
    QFETCH(QString, optionGroupDescription);
    QFETCH(QString, optionType);
    QFETCH(bool, hidden);

    QCOMPARE( optionTokenizer->getOption()->getGroupNumber(optionName), groupNumber );
    QCOMPARE( optionTokenizer->getOption()->getGroupName(optionName), optionGroupName );
    QCOMPARE( optionTokenizer->getOption()->getGroupDescription(optionName), optionGroupDescription );
    QCOMPARE( optionTokenizer->getOption()->getOptionTypeName(optionTokenizer->getOption()->getOptionType(optionName)), optionType );
    QCOMPARE( optionTokenizer->getOption()->isGroupHidden(groupNumber), hidden );

}

void TestMINOSOption::testInvalidOption_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("nameValid");

    QTest::newRow("iiis_invalid")                 << "iiis"                  << false;
    QTest::newRow("what?_invalid")                << "what?"                 << false;

    QTest::newRow("row_tolerance_invalid")        << "row_tolerance"         << false;
    QTest::newRow("row tolerance_valid")          << "row tolerance"         << true;

    QTest::newRow("scale_print_invalid")          << "scale_print"           << false;
    QTest::newRow("scale print_valid")            << "scale print"           << true;

    QTest::newRow("hessian_dimension_invalid")    << "hessian_dimension"     << false;
    QTest::newRow("hessian dimension_valid")      << "hessian dimension"     << true;

    QTest::newRow("major_iterations_invalid")     << "major_iterations"      << false;
    QTest::newRow("major iterations_valid")       << "major iterations"      << true;

    QTest::newRow("unbounded_objective_value_invalid")  << "unbounded_objective_value"   << false;
    QTest::newRow("unbounded objective value valid")    << "unbounded objective value"   << true;

    QTest::newRow("LU_complete_pivoting_invalid") << "LU_complete_pivoting"  << false;
    QTest::newRow("LU complete pivoting_valid")   << "LU complete pivoting"  << true;
}

void TestMINOSOption::testInvalidOption()
{
    QFETCH(QString, optionName);
    QFETCH(bool, nameValid);

    QCOMPARE( optionTokenizer->getOption()->isValid(optionName), nameValid );
}

void TestMINOSOption::testReadOptionFile_data()
{
    // given
    QFile outputFile(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("minos.op2"));
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
        QFAIL("expected to open cplex.op2 to write, but failed");

    QTextStream out(&outputFile);
    out << "print level 1" << endl;
    out << "* this is a comment line" << endl;
    out << "" << endl;
    out << "hessian dimension=8" << endl;
    out << "crash option 2" << endl;;
    out << "major damping parameter 4.2" << endl;
    out << "radius of convergence = 0.1" << endl;
    out << "lagrangian YES      * Default value (recommended) " << endl;
    out << "start assigned nonlinears BASIC" << endl;
    out << "scale nonlinear variables  * No value" << endl;
    out << "scale print" << endl;
    out << "secret strlist - back door for secret or undocumented MINOS options" << endl;
    out << "" << endl;
    out << "unbounded step size * ub" << endl;
    out << "unbounded_step_size 1.2345" << endl;
    out << "scale option 2 ! Scale linear + nonlinear variables" << endl;
    out << "start_assigned_nonlinears ELIGIBLE_FOR_CRASH" << endl;
    outputFile.close();

    // when
    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("minos.op2");
    QList<SolverOptionItem *> items = optionTokenizer->readOptionFile(optFile, QTextCodec::codecForLocale());

    // then
    QCOMPARE( items.size(), 17 );

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
    QTest::newRow("* this is a comment line")  << items.at(1)->disabled <<  true
                           << items.at(1)->key      << "* this is a comment line"
                           << items.at(1)->value    << QVariant("")
                           << items.at(1)->text     << ""
                           << items.at(1)->optionId << -1
                           << static_cast<int>(items.at(1)->error)    << static_cast<int>(OptionErrorType::No_Error);
    QTest::newRow("[empty line1]")  << items.at(2)->disabled <<  true
                           << items.at(2)->key      << ""
                           << items.at(2)->value    << QVariant("")
                           << items.at(2)->text     << ""
                           << items.at(2)->optionId << -1
                           << static_cast<int>(items.at(2)->error)    << static_cast<int>(OptionErrorType::No_Error);

    QTest::newRow("[empty line2]")  << items.at(12)->disabled <<  true
                           << items.at(12)->key      << ""
                           << items.at(12)->value    << QVariant("")
                           << items.at(12)->text     << ""
                           << items.at(12)->optionId << -1
                           << static_cast<int>(items.at(12)->error)    << static_cast<int>(OptionErrorType::No_Error);

    // valid options
    QTest::newRow("print level 1")  << items.at(0)->disabled <<  false
                           << items.at(0)->key      << "print level"
                           << items.at(0)->value    << QVariant("1")
                           << items.at(0)->text     << ""
                           << items.at(0)->optionId << 6
                           << static_cast<int>(items.at(0)->error)    << static_cast<int>(OptionErrorType::No_Error);

    QTest::newRow("hessian dimension=8")  << items.at(3)->disabled <<  false
                           << items.at(3)->key      << "hessian dimension"
                           << items.at(3)->value    << QVariant("8")
                           << items.at(3)->text     << ""
                           << items.at(3)->optionId << 23
                           << static_cast<int>(items.at(3)->error)    << static_cast<int>(OptionErrorType::No_Error);

    QTest::newRow("crash option 2")  << items.at(4)->disabled <<  false
                           << items.at(4)->key      << "crash option"
                           << items.at(4)->value    << QVariant("2")
                           << items.at(4)->text     << ""
                           << items.at(4)->optionId << 32
                           << static_cast<int>(items.at(4)->error)    << static_cast<int>(OptionErrorType::No_Error);

    QTest::newRow("major damping parameter 4.2")  << items.at(5)->disabled <<  false
                           << items.at(5)->key      << "major damping parameter"
                           << items.at(5)->value    << QVariant("4.2")
                           << items.at(5)->text     << ""
                           << items.at(5)->optionId << 39
                           << static_cast<int>(items.at(5)->error)    << static_cast<int>(OptionErrorType::No_Error);

    QTest::newRow("radius of convergence = 0.1")  << items.at(6)->disabled <<  false
                           << items.at(6)->key      << "radius of convergence"
                           << items.at(6)->value    << QVariant("0.1")
                           << items.at(6)->text     << ""
                           << items.at(6)->optionId << 44
                           << static_cast<int>(items.at(6)->error)    << static_cast<int>(OptionErrorType::No_Error);

    QTest::newRow("lagrangian YES      * Default value (recommended) ")  << items.at(7)->disabled <<  false
                           << items.at(7)->key      << "lagrangian"
                           << items.at(7)->value    << QVariant("YES")
                           << items.at(7)->text     << "Default value (recommended)"
                           << items.at(7)->optionId << 35
                           << static_cast<int>(items.at(7)->error)    << static_cast<int>(OptionErrorType::No_Error);

    QTest::newRow("start assigned nonlinears BASIC")  << items.at(8)->disabled <<  false
                           << items.at(8)->key      << "start assigned nonlinears"
                           << items.at(8)->value    << QVariant("BASIC")
                           << items.at(8)->text     << ""
                           << items.at(8)->optionId << 51
                           << static_cast<int>(items.at(8)->error)    << static_cast<int>(OptionErrorType::No_Error);

    QTest::newRow("scale nonlinear variables  * No value")  << items.at(9)->disabled <<  false
                           << items.at(9)->key      << "scale nonlinear variables"
                           << items.at(9)->value    << QVariant("")
                           << items.at(9)->text     << "No value"
                           << items.at(9)->optionId << 49
                           << static_cast<int>(items.at(9)->error)    << static_cast<int>(OptionErrorType::No_Error);

    QTest::newRow("scale print")  << items.at(10)->disabled <<  false
                           << items.at(10)->key      << "scale print"
                           << items.at(10)->value    << QVariant("")
                           << items.at(10)->text     << ""
                           << items.at(10)->optionId << 7
                           << static_cast<int>(items.at(10)->error)    << static_cast<int>(OptionErrorType::No_Error);

    QTest::newRow("secret strlist - back door for secret or undocumented MINOS options")  << items.at(11)->disabled <<  false
                           << items.at(11)->key      << "secret"
                           << items.at(11)->value    << QVariant("strlist - back door for secret or undocumented MINOS options")
                           << items.at(11)->text     << ""
                           << items.at(11)->optionId << 59
                           << static_cast<int>(items.at(11)->error)    << static_cast<int>(OptionErrorType::No_Error);

    QTest::newRow("unbounded step size * ub")
                           << items.at(13)->disabled <<  false
                           << items.at(13)->key      << "unbounded step size"
                           << items.at(13)->value    << QVariant("")
                           << items.at(13)->text     << "ub"
                           << items.at(13)->optionId << -1
                           << static_cast<int>(items.at(13)->error)    << static_cast<int>(OptionErrorType::UserDefined_Error);

    // incorrect Option Key
    QTest::newRow("unbounded_step_size 1.2345")
                           << items.at(14)->disabled <<  false
                           << items.at(14)->key      << "unbounded_step_size 1.2345"
                           << items.at(14)->value    << QVariant("")
                           << items.at(14)->text     << ""
                           << items.at(14)->optionId << -1
                           << static_cast<int>(items.at(14)->error)    << static_cast<int>(OptionErrorType::UserDefined_Error);

    // incorrect EOL Comment Character
    QTest::newRow("scale option 2 ! Scale linear + nonlinear variables")
                           << items.at(15)->disabled <<  false
                           << items.at(15)->key      << "scale option 2 ! Scale linear + nonlinear variables"
                           << items.at(15)->value    << QVariant("")
                           << items.at(15)->text     << ""
                           << items.at(15)->optionId << -1
                           << static_cast<int>(items.at(15)->error)    << static_cast<int>(OptionErrorType::Incorrect_Value_Type);

    QTest::newRow("start_assigned_nonlinears ELIGIBLE_FOR_CRASH")
                           << items.at(16)->disabled <<  false
                           << items.at(16)->key      << "start_assigned_nonlinears ELIGIBLE_FOR_CRASH"
                           << items.at(16)->value    << QVariant("")
                           << items.at(16)->text     << ""
                           << items.at(16)->optionId << -1
                           << static_cast<int>(items.at(16)->error)    << static_cast<int>(OptionErrorType::UserDefined_Error);

}

void TestMINOSOption::testReadOptionFile()
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

void TestMINOSOption::testNonExistReadOptionFile()
{
    // when
    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("minos.op012345");
    QList<SolverOptionItem *> items = optionTokenizer->readOptionFile(optFile, QTextCodec::codecForLocale());

    // then
    QCOMPARE( items.size(), 0);
}

void TestMINOSOption::testWriteOptionFile_data()
{
    // given
    QList<SolverOptionItem *> items;
    items.append(new SolverOptionItem(-1, "* summary frequency 1234", "", "", true));
    items.append(new SolverOptionItem(-1, "summary frequency", "1000", "", false));
    items.append(new SolverOptionItem(-1, "crash option", "3", "", false));
    items.append(new SolverOptionItem(-1, "factorization frequency", "99", "", false));

    items.append(new SolverOptionItem(-1, "LU factor tolerance", "2e+8", "", false));
    items.append(new SolverOptionItem(-1, "optimality tolerance", "1.0e-2", "", false));

    items.append(new SolverOptionItem(-1, "solution", "YES", "", false));
    items.append(new SolverOptionItem(-1, "start assigned nonlinears", "ELIGIBLE FOR CRASH", "", false));
    items.append(new SolverOptionItem(-1, "LU complete pivoting", "", "", false));
    items.append(new SolverOptionItem(-1, "scale no", "", "", false));
    items.append(new SolverOptionItem(-1, "verify gradients", "", "", false));

    int size = items.size();

    // when
    QVERIFY( optionTokenizer->writeOptionFile(items, QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("minos.op4"), QTextCodec::codecForLocale()) );

    // clean up
    qDeleteAll(items);
    items.clear();

    // then
    QFile inputFile(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("minos.op4"));
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
    QTest::addColumn<QString>("writtenLine");

    QTest::newRow("line0") << optionItems.at(0) <<  "* summary frequency 1234"   <<  "* summary frequency 1234";
    QTest::newRow("line1") << optionItems.at(1) <<  "summary frequency 1000"     <<  "summary frequency=1000";
    QTest::newRow("line2") << optionItems.at(2) <<  "crash option 3"             <<  "crash option=3";
    QTest::newRow("line3") << optionItems.at(3) <<  "factorization frequency 99" <<  "factorization frequency=99";

    QTest::newRow("line4") << optionItems.at(4) <<  "LU factor tolerance 2e+8"    <<  "LU factor tolerance=2e+8";
    QTest::newRow("line5") << optionItems.at(5) <<  "optimality tolerance 1.0e-2" <<  "optimality tolerance=1.0e-2";

    QTest::newRow("line6")  << optionItems.at(6)  <<  "solution YES"              <<  "solution=YES";
    QTest::newRow("line7")  << optionItems.at(7)  <<  "start assigned nonlinears \"ELIGIBLE FOR CRASH\"" <<  "start assigned nonlinears=\"ELIGIBLE FOR CRASH\"";
    QTest::newRow("line8")  << optionItems.at(8)  <<  "LU complete pivoting"      <<  "LU complete pivoting";
    QTest::newRow("line9")  << optionItems.at(9)  <<  "scale no"                  <<  "scale no";
    QTest::newRow("line10") << optionItems.at(10) <<  "verify gradients"          <<  "verify gradients";
}

void TestMINOSOption::testWriteOptionFile()
{
    QFETCH(QString, optionString);
    QFETCH(QString, line);
    QFETCH(QString, writtenLine);

    QCOMPARE( optionString, writtenLine );
}

void TestMINOSOption::testEOLChars()
{
    char eolchars[256];
    int numchar = optEOLChars( mOPTHandle, eolchars);

    QCOMPARE( 1, numchar );
    QCOMPARE( "*", QString::fromLatin1(eolchars) );
}

void TestMINOSOption::cleanupTestCase()
{
    if (mOPTHandle)
        optFree(&mOPTHandle);

    if (optionTokenizer)
        delete optionTokenizer;
}

bool TestMINOSOption::containKey(QList<OptionItem> &items, const QString &key) const
{
    for(OptionItem item : items) {
        if (QString::compare(item.key, key, Qt::CaseInsensitive)==0)
            return true;
    }
    return false;
}

QVariant TestMINOSOption::getValue(QList<OptionItem> &items, const QString &key) const
{
    QVariant value;
    for(OptionItem item : items) {
        if (QString::compare(item.key, key, Qt::CaseInsensitive)==0)
            return QVariant(item.value);
    }
    return value;
}

QTEST_MAIN(TestMINOSOption)
