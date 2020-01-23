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

#include "testconopt4option.h"
#include "commonpaths.h"
#include "gclgms.h"
#include "optcc.h"

using gams::studio::CommonPaths;

void TestConopt4Option::initTestCase()
{
    // given
    const QString expected = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    CommonPaths::setSystemDir(expected.toLatin1());
    // when
    optionTokenizer = new OptionTokenizer(QString("optconopt4.def"));
    if  ( !optionTokenizer->getOption()->available() ) {
       QFAIL("expected successful read of optconopt4.def, but failed");
    }

    // when
    char msg[GMS_SSSIZE];
    optCreateD(&mOPTHandle, CommonPaths::systemDir().toLatin1(), msg, sizeof(msg));
    if (msg[0] != '\0')
        Dcreated = false;
    else
        Dcreated = true;

    // test cplex for now
    QString optdef = "optconopt4.def";
    if (optReadDefinition(mOPTHandle, QDir(CommonPaths::systemDir()).filePath(optdef).toLatin1())) {
        optdefRead = false;
    } else {
        optdefRead = true;
    }
}

void TestConopt4Option::testOptionBooleanType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("defaultValue");
    QTest::addColumn<QString>("description");

    QTest::newRow("FByCol")          << "FByCol"          << false << 1  << "";
    QTest::newRow("Flg_Dbg_Intv")    << "Flg_Dbg_Intv"    << true  << 0  << "Flag for debugging interval evaluations.";
    QTest::newRow("Flg_DC_Unique")   << "Flg_DC_Unique"   << true  << 1  << "Flag for requiring definitional constraints to be unique";
    QTest::newRow("Scale_Infeas")    << "Scale_Infeas"    << false << 1  << "";
    QTest::newRow("Flg_Crash_Basis") << "Flg_Crash_Basis" << true  << 1  << "Flag for crashing an initial basis without fixed slacks";
    QTest::newRow("LSLACK")          << "LSLACK"          << false << 0  << "";
}

void TestConopt4Option::testOptionBooleanType()
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

void TestConopt4Option::testOptionEnumIntType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("numberOfEnumint");
    QTest::addColumn<int>("defaultValue");

    QTest::newRow("DF_Method")  << "Mtd_RedHess"   << true  << 2  << 0;
    QTest::newRow("Mtd_Scale")  << "Mtd_Scale"     << true  << 4  << 3;
}

void TestConopt4Option::testOptionEnumIntType()
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

void TestConopt4Option::testOptionDoubleType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<double>("lowerBound");
    QTest::addColumn<double>("upperBound");
    QTest::addColumn<double>("defaultValue");

    QTest::newRow("Rat_NoPen")     <<  "Rat_NoPen"     << true  << 0.0    << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 0.1;
    QTest::newRow("Lim_Variable")  <<  "Lim_Variable"  << true  << 1e+5   << 1e+30                                << 1e+15;
    QTest::newRow("Tol_Bound")     <<  "Tol_Bound"     << true  << 3e-13  << 1e-5                                 << 1e-7;
    QTest::newRow("HEAPLIMIT")     <<  "HEAPLIMIT"     << true  << 0.0    << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 1e+20;
    QTest::newRow("HessianMemFac") <<  "HessianMemFac" << true  << 0.0    << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 0.0;
    QTest::newRow("Lim_Hess_Est")  <<  "Lim_Hess_Est"  << true  << 1.0    << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 1e+4;
    QTest::newRow("Lim_Time")      <<  "Lim_Time"      << true  << 0.0    << gams::studio::option::OPTION_VALUE_MAXDOUBLE << 10000.0;
}

void TestConopt4Option::testOptionDoubleType()
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

void TestConopt4Option::testOptionIntegerType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("lowerBound");
    QTest::addColumn<int>("upperBound");
    QTest::addColumn<int>("defaultValue");

    QTest::newRow("LFDERR")        << "LFDERR"        << false  << 1  << gams::studio::option::OPTION_VALUE_MAXINT  << 10;
    QTest::newRow("LFEMSG")        << "LFEMSG"        << false  << 1  << gams::studio::option::OPTION_VALUE_MAXINT  << 10;
    QTest::newRow("Lim_StallIter") << "Lim_StallIter" << true   << 2  << gams::studio::option::OPTION_VALUE_MAXINT  << 100;
    QTest::newRow("LKDEBG")        << "LKDEBG"        << false  << -1 << gams::studio::option::OPTION_VALUE_MAXINT  << 0;
}

void TestConopt4Option::testOptionIntegerType()
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

void TestConopt4Option::testOptionSynonym_data()
{
    QTest::addColumn<QString>("optionSynonym");
    QTest::addColumn<QString>("optionName");

    QTest::newRow("domlim")  << "domlim"  << "LFEERR";
    QTest::newRow("iterlim") << "iterlim" << "LFITER";
    QTest::newRow("reslim")  << "reslim"  << "RVTIME";

    QTest::newRow("Flg_Crash_Basis")  << ""  << "Flg_Crash_Basis";
    QTest::newRow("Flg_Crash")        << ""  << "Flg_Crash";
    QTest::newRow("LSSQRS")           << ""  << "LSSQRS";
    QTest::newRow("Lim_Variable")     << ""  << "Lim_Variable";
}


void TestConopt4Option::testOptionSynonym()
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

void TestConopt4Option::testOptionGroup_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<int>("groupNumber");
    QTest::addColumn<QString>("optionGroupName");
    QTest::addColumn<QString>("optionGroupDescription");
    QTest::addColumn<QString>("optionType");

    QTest::newRow("Flg_Convex_1")     << "Flg_Convex"      << 1 << "a" << "Algorithmic options" << "boolean";
    QTest::newRow("Flg_Square_1")     << "Flg_Square"      << 1 << "a" << "Algorithmic options" << "boolean";
    QTest::newRow("Lim_Iteration_1")  << "Lim_Iteration"   << 1 << "a" << "Algorithmic options" << "integer";
    QTest::newRow("Num_Rounds_1")     << "Num_Rounds"      << 1 << "a" << "Algorithmic options" << "integer";
    QTest::newRow("Rat_NoPen_1")      << "Rat_NoPen"       << 1 << "a" << "Algorithmic options" << "double";
    QTest::newRow("Tol_Feas_Max_1")   << "Tol_Feas_Max"    << 1 << "a" << "Algorithmic options" << "double";
    QTest::newRow("Tol_Feas_Min_1")   << "Tol_Feas_Min"    << 1 << "a" << "Algorithmic options" << "double";
    QTest::newRow("Tol_Feas_Tria_1")  << "Tol_Feas_Tria"   << 1 << "a" << "Algorithmic options" << "double";

    QTest::newRow("Flg_Interv_2")       << "Flg_Interv"       << 2 << "d" << "Debugging options" << "boolean";
    QTest::newRow("Flg_Prep_2")         << "Flg_Prep"         << 2 << "d" << "Debugging options" << "boolean";
    QTest::newRow("Lim_Dbg_1Drv_2")     << "Lim_Dbg_1Drv"     << 2 << "d" << "Debugging options" << "integer";
    QTest::newRow("Lim_Hess_Est_2")     << "Lim_Hess_Est"     << 2 << "d" << "Debugging options" << "double";
    QTest::newRow("Lim_Msg_Dbg_1Drv_2") << "Lim_Msg_Dbg_1Drv" << 2 << "d" << "Debugging options" << "integer";

    QTest::newRow("Frq_Log_Simple_3") << "Frq_Log_Simple" << 3 << "o" << "Output options" << "integer";
    QTest::newRow("Frq_Log_SlpSqp_3") << "Frq_Log_SlpSqp" << 3 << "o" << "Output options" << "integer";
    QTest::newRow("Lim_Msg_Large_3")  << "Lim_Msg_Large"  << 3 << "o" << "Output options" << "integer";
    QTest::newRow("Lim_Pre_Msg_3")    << "Lim_Pre_Msg"    << 3 << "o" << "Output options" << "integer";

//    QTest::newRow("LF2DRV_4")      << "LF2DRV"     << 4 << "h" << "Hidden options" << "integer";
//    QTest::newRow("LFDEGI_4")      << "LFDEGI"     << 4 << "h" << "Hidden options" << "integer";
//    QTest::newRow("LFDERR_4")      << "LFDERR"     << 4 << "h" << "Hidden options" << "integer";
//    QTest::newRow("LFDLIM_4")      << "LFDLIM"     << 4 << "h" << "Hidden options" << "integer";
//    QTest::newRow("LFSQPP_4")      << "LFSQPP"     << 4 << "h" << "Hidden options" << "integer";
//    QTest::newRow("MAXMEM_4")      << "MAXMEM"     << 4 << "h" << "Hidden options" << "boolean";
//    QTest::newRow("STATFILE_4")    << "STATFILE"   << 4 << "h" << "Hidden options" << "boolean";
//    QTest::newRow("RTMINJ_4")      << "RTMINJ"     << 4 << "h" << "Hidden options" << "double";
//    QTest::newRow("WRTPAR_4")      << "WRTPAR"     << 4 << "h" << "Hidden options" << "boolean";
//    QTest::newRow("nobounds_4")    << "nobounds"   << 4 << "h" << "Hidden options" << "immediate";
//    QTest::newRow("readfile_4")    << "readfile"   << 4 << "h" << "Hidden options" << "immediate";

    QTest::newRow("cooptfile_5")      << "cooptfile"     << 5 << "l" << "Interface options" << "string";
    QTest::newRow("Flg_2DDir_5")      << "Flg_2DDir"     << 5 << "l" << "Interface options" << "boolean";
    QTest::newRow("Flg_Hessian_5")    << "Flg_Hessian"   << 5 << "l" << "Interface options" << "boolean";
    QTest::newRow("HEAPLIMIT_5")      << "HEAPLIMIT"     << 5 << "l" << "Interface options" << "double";
    QTest::newRow("HessianMemFac_5")  << "HessianMemFac" << 5 << "l" << "Interface options" << "double";
    QTest::newRow("THREAD2D_5")       << "THREAD2D"      << 5 << "l" << "Interface options" << "integer";
    QTest::newRow("THREADC_5")        << "THREADC"       << 5 << "l" << "Interface options" << "integer";
    QTest::newRow("THREADF_5")        << "THREADF"       << 5 << "l" << "Interface options" << "integer";
    QTest::newRow("threads_5")        << "threads"       << 5 << "l" << "Interface options" << "integer";
}

void TestConopt4Option::testOptionGroup()
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

void TestConopt4Option::testInvalidOption_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("nameValid");
    QTest::addColumn<bool>("synonymValid");

    QTest::newRow("FLG_CONVEX_valid")  << "FLG_CONVEX"  << true     << false;
    QTest::newRow("Flg_Interv_valid")  << "Flg_Interv"  << true     << false;
    QTest::newRow("domlim_valid")      << "domlim"      << false    << true;
    QTest::newRow("iterlim_valid")     << "iterlim"     << false    << true;
    QTest::newRow("reslim_valid")      << "reslim"      << false    << true;

    QTest::newRow("DF_Method_invalid")      << "DF_Method"       << false    << false;
    QTest::newRow("LimNewSuper_invalid")    << "LimNewSuper"     << false    << false;
    QTest::newRow("Tol_IFix_invalid")       << "Tol_IFix"        << false    << false;
}

void TestConopt4Option::testInvalidOption()
{
    QFETCH(QString, optionName);
    QFETCH(bool, nameValid);
    QFETCH(bool, synonymValid);

    QCOMPARE( optionTokenizer->getOption()->isValid(optionName), nameValid);
    QCOMPARE( optionTokenizer->getOption()->isASynonym(optionName), synonymValid);
}

void TestConopt4Option::testReadOptionFile_data()
{
    // given
    QFile outputFile(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("conopt4.op2"));
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
        QFAIL("expected to open conopt4.op2 to write, but failed");

    QTextStream out(&outputFile);
    out << "* This is comment line" << endl;
    out << "* DF_Method 1" << endl;
    out << "Lim_Iteration=100" << endl;
    out << "Lim_Iteration 100" << endl;
    out << "Flg_Hessian 1" << endl;
    out << "cooptfile \"C:/Users/Dude/coopt.file\"" << endl;
    out << "Tol_Bound=5.E-9 ; Bound filter tolerance for solution values close to a bound." << endl;
    out << "HEAPLIMIT 1e20  # Maximum Heap size in MB allowed" << endl;
    out << "Lim_Variable  1e+03 " << endl;
    outputFile.close();

    // when
    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("conopt4.op2");
    QList<SolverOptionItem *> items = optionTokenizer->readOptionFile(optFile, QTextCodec::codecForLocale());

    // then
    QCOMPARE( items.size(), 9 );

    QTest::addColumn<bool>("optionItem_disabledFlag");
    QTest::addColumn<bool>("disabledFlag");
    QTest::addColumn<QString>("optionItem_optionKey");
    QTest::addColumn<QString>("optionKey");
    QTest::addColumn<QVariant>("optionItem_optionValue");
    QTest::addColumn<QVariant>("optionValue");
    QTest::addColumn<bool>("isValueDouble");
    QTest::addColumn<QString>("optionItem_optionText");
    QTest::addColumn<QString>("optionText");
    QTest::addColumn<int>("optionItem_optionId");
    QTest::addColumn<int>("optionId");
    QTest::addColumn<int>("optionItem_error");
    QTest::addColumn<int>("error");

    QTest::newRow("* This is comment line")
            << items.at(0)->disabled <<  true
            << items.at(0)->key      << "* This is comment line"
            << items.at(0)->value    << QVariant("")   << false
            << items.at(0)->text     << ""
            << items.at(0)->optionId << -1
            << static_cast<int>(items.at(0)->error)    << static_cast<int>(No_Error);
    QTest::newRow("DF_Method 1")
            << items.at(1)->disabled <<  true
            << items.at(1)->key      << "* DF_Method 1"
            << items.at(0)->value    << QVariant("")   << false
            << items.at(0)->text     << ""
            << items.at(0)->optionId << -1
            << static_cast<int>(items.at(1)->error)    << static_cast<int>(No_Error);
    QTest::newRow("Lim_Iteration=100")
            << items.at(2)->disabled <<  false
            << items.at(2)->key      << "Lim_Iteration"
            << items.at(2)->value    << QVariant("100") << false
            << items.at(2)->text     << ""
            << items.at(2)->optionId << 25
            << static_cast<int>(items.at(2)->error)    << static_cast<int>(No_Error);
    QTest::newRow("Lim_Iteration 100")
            << items.at(3)->disabled <<  false
            << items.at(3)->key      << "Lim_Iteration"
            << items.at(3)->value    << QVariant("100") << false
            << items.at(3)->text     << ""
            << items.at(3)->optionId << 25
            << static_cast<int>(items.at(3)->error)    << static_cast<int>(No_Error);
    QTest::newRow("Flg_Hessian 1")
            << items.at(4)->disabled <<  false
            << items.at(4)->key      << "Flg_Hessian"
            << items.at(4)->value    << QVariant("1")  << false
            << items.at(4)->text     << ""
            << items.at(4)->optionId << 281
            << static_cast<int>(items.at(4)->error)    << static_cast<int>(No_Error);
    QTest::newRow("cooptfile \"C:/Users/Dude/coopt.file\"")
            << items.at(5)->disabled <<  false
            << items.at(5)->key      << "cooptfile"
            << items.at(5)->value    << QVariant("\"C:/Users/Dude/coopt.file\"") << false
            << items.at(5)->text     << ""
            << items.at(5)->optionId << 298
            << static_cast<int>(items.at(5)->error)    << static_cast<int>(No_Error);
    QTest::newRow("Tol_Bound=5.E-9 ; Bound filter tolerance for solution values close to a bound.")
            << items.at(6)->disabled <<  false
            << items.at(6)->key      << "Tol_Bound"
            << items.at(6)->value    << QVariant("5.E-9")  << true
            << items.at(6)->text     << "Bound filter tolerance for solution values close to a bound."
            << items.at(6)->optionId << 224
            << static_cast<int>(items.at(6)->error)    << static_cast<int>(No_Error);
    QTest::newRow("HEAPLIMIT 1e20  # Maximum Heap size in MB allowed")
            << items.at(7)->disabled <<  false
            << items.at(7)->key      << "HEAPLIMIT"
            << items.at(7)->value    << QVariant("1e20")   << true
            << items.at(7)->text     << "Maximum Heap size in MB allowed"
            << items.at(7)->optionId << 299
            << static_cast<int>(items.at(7)->error)    << static_cast<int>(No_Error);
    QTest::newRow("Lim_Variable  1e+03 ")
            << items.at(8)->disabled <<  false
            << items.at(8)->key      << "Lim_Variable"
            << items.at(8)->value    << QVariant("1e+03")   << true
            << items.at(8)->text     << ""
            << items.at(8)->optionId << 175
            << static_cast<int>(items.at(8)->error)    << static_cast<int>(Value_Out_Of_Range);
}

void TestConopt4Option::testReadOptionFile()
{
    QFETCH(bool, optionItem_disabledFlag);
    QFETCH(bool, disabledFlag);
    QFETCH(QString, optionItem_optionKey);
    QFETCH(QString, optionKey);
    QFETCH(QVariant, optionItem_optionValue);
    QFETCH(QVariant, optionValue);
    QFETCH(bool, isValueDouble);
    QFETCH(QString, optionItem_optionText);
    QFETCH(QString, optionText);
//    QFETCH(int, optionItem_optionId);
//    QFETCH(int, optionId);
    QFETCH(int, optionItem_error);
    QFETCH(int, error);

    QCOMPARE( optionItem_disabledFlag, disabledFlag );
    QCOMPARE( optionItem_optionKey.compare(optionKey, Qt::CaseInsensitive), 0 );
    if (isValueDouble) {
        QCOMPARE( optionItem_optionValue.toDouble(), optionValue.toDouble() );
    } else {
        QCOMPARE( optionItem_optionValue, optionValue );
    }
    QCOMPARE( optionItem_optionText.compare(optionText, Qt::CaseInsensitive), 0 );
//    QCOMPARE( optionItem_optionId, optionId );
    QCOMPARE( optionItem_error, error );
}

void TestConopt4Option::testNonExistReadOptionFile()
{
    // when
    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("conopt4.op012345");
    QList<SolverOptionItem *> items = optionTokenizer->readOptionFile(optFile, QTextCodec::codecForLocale());

    // then
    QCOMPARE( items.size(), 0);
}

void TestConopt4Option::testWriteOptionFile_data()
{
    // given
    QList<SolverOptionItem *> items;
    items.append(new SolverOptionItem(-1, "DF_Method", "1", "", false));
    items.append(new SolverOptionItem(-1, "Lim_Iteration", "100", "", false));
    items.append(new SolverOptionItem(-1, "Tol_Bound", " 5.e-9", "", false));
    items.append(new SolverOptionItem(-1, "Tol_Optimality", "1.e-10", "", false));
    items.append(new SolverOptionItem(-1, "cooptfile", "C:/Users/Programs Files/Dude/coopt.file", "", false));
//    items.append(OptionItem("readfile", "this is read file"));  => optTypeImmediate
    int size = items.size();

    // when
    QVERIFY( optionTokenizer->writeOptionFile(items, QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("conopt4.op4"), QTextCodec::codecForLocale()) );

    // clean up
    qDeleteAll(items);
    items.clear();

    // then
    QFile inputFile(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("conopt4.op4"));
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

    QTest::newRow("line0") << optionItems.at(0) << "DF_Method 1"           << "DF_Method=1";
    QTest::newRow("line1") << optionItems.at(1) << "Lim_Iteration 100"     << "Lim_Iteration=100";
    QTest::newRow("line2") << optionItems.at(2) << "Tol_Bound 5.e-9"       << "Tol_Bound=5.e-9";
    QTest::newRow("line3") << optionItems.at(3) << "Tol_Optimality 1.e-10" << "Tol_Optimality=1.e-10";
    QTest::newRow("line4") << optionItems.at(4) << "cooptfile \"C:/Users/Programs Files/Dude/coopt.file\"" << "cooptfile=\"C:/Users/Programs Files/Dude/coopt.file\"";
}

void TestConopt4Option::testWriteOptionFile()
{
    QFETCH(QString, optionString);
    QFETCH(QString, writtenLine);

    QCOMPARE( optionString, writtenLine );
}

void TestConopt4Option::testEOLChars()
{
    char eolchars[256];
    int numchar = optEOLChars( mOPTHandle, eolchars);

    QCOMPARE( 3, numchar );
    QCOMPARE( ";#!", QString::fromLatin1(eolchars) );
}

void TestConopt4Option::cleanupTestCase()
{
    if (mOPTHandle)
        optFree(&mOPTHandle);

    if (optionTokenizer)
        delete optionTokenizer;
}

bool TestConopt4Option::containKey(QList<OptionItem> &items, const QString &key) const
{
    for(OptionItem item : items) {
        if (QString::compare(item.key, key, Qt::CaseInsensitive)==0)
            return true;
    }
    return false;
}

QVariant TestConopt4Option::getValue(QList<OptionItem> &items, const QString &key) const
{
    QVariant value;
    for(OptionItem item : items) {
        if (QString::compare(item.key, key, Qt::CaseInsensitive)==0)
            return QVariant(item.value);
    }
    return value;
}

QTEST_MAIN(TestConopt4Option)
