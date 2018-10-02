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
#include "testminosoption.h"

using gams::studio::CommonPaths;

void TestMINOSOption::initTestCase()
{
    // given
    const QString expected = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    CommonPaths::setSystemDir(expected.toLatin1());
    // when
    optionTokenizer = new OptionTokenizer(QString("optminos.def"));
    if  ( !optionTokenizer->getOption()->available() ) {
       QFAIL("expected successful read of optminos.def, but failed");
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

void TestMINOSOption::testReadOptionFile()
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
    outputFile.close();

    // when
    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("minos.op2");
    QList<OptionItem> items = optionTokenizer->readOptionParameterFile(optFile);

    // then
    QCOMPARE( items.size(), 10 );

    QVERIFY( containKey (items,"print level") );
    QCOMPARE( getValue(items,"print level").toInt(),  QVariant("1").toInt() );

    QVERIFY( containKey (items,"hessian dimension") );
    QCOMPARE( getValue(items,"hessian dimension").toInt(),  QVariant("8").toInt() );

    QVERIFY( containKey (items,"crash option") );
    QCOMPARE( getValue(items,"crash option").toInt(),  QVariant("2").toInt() );

    QVERIFY( containKey (items,"major damping parameter") );
    QCOMPARE( getValue(items,"major damping parameter").toDouble(), QVariant("4.2").toDouble() );

    QVERIFY( containKey (items,"radius of convergence") );
    QCOMPARE( getValue(items,"radius of convergence").toDouble(), QVariant("0.1").toDouble() );

    QVERIFY( containKey (items,"lagrangian") );
    QCOMPARE( getValue(items,"lagrangian").toString(), QVariant("YES").toString() );

    QVERIFY( containKey (items,"start assigned nonlinears") );
    QCOMPARE( getValue(items,"start assigned nonlinears").toString(), QVariant("BASIC").toString() );

    QVERIFY( containKey (items,"scale nonlinear variables") );
    QCOMPARE( getValue(items,"scale nonlinear variables").toString(), QVariant("").toString() );

    QVERIFY( containKey (items,"scale print") );
    QCOMPARE( getValue(items,"scale print").toString(), QVariant("").toString() );

    QVERIFY( containKey (items,"secret") );
    QCOMPARE( getValue(items,"secret").toString(),
              QVariant("strlist - back door for secret or undocumented MINOS options").toString() );
}

void TestMINOSOption::testNonExistReadOptionFile()
{
    // when
    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("minos.op012345");
    QList<OptionItem> items = optionTokenizer->readOptionParameterFile(optFile);

    // then
    QCOMPARE( items.size(), 0);
}

void TestMINOSOption::cleanupTestCase()
{
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
