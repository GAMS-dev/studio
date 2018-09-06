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

#include "testconopt4option.h"
//#include "option/option.h"
#include "commonpaths.h"

using gams::studio::Option;
using gams::studio::OptionItem;
using gams::studio::CommonPaths;


void TestConopt4Option::initTestCase()
{
    // given
    const QString expected = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    CommonPaths::setSystemDir(expected.toLatin1());
    // when
    mOption = new Option(CommonPaths::systemDir(), "optconopt4.def");
    if  ( !mOption->available() ) {
       QFAIL("expected successful read of optconopt4.def, but failed");
    }
}

void TestConopt4Option::testOptionEnumIntType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("numberOfEnumint");
    QTest::addColumn<int>("defaultValue");

    QTest::newRow("DF_Method")  << "DF_Method"   << true  << 2  << 0;
}

void TestConopt4Option::testOptionEnumIntType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(int, numberOfEnumint);
    QFETCH(int, defaultValue);

    QCOMPARE( mOption->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( mOption->getOptionType(optionName),  optTypeEnumInt);
    QCOMPARE( mOption->getValueList(optionName).size() , numberOfEnumint);
    QCOMPARE( mOption->getDefaultValue(optionName).toInt(), defaultValue );
}

void TestConopt4Option::testOptionDoubleType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<double>("lowerBound");
    QTest::addColumn<double>("upperBound");
    QTest::addColumn<double>("defaultValue");

    QTest::newRow("Rat_NoPen")     <<  "Rat_NoPen"     << true  << 0.0    << gams::studio::OPTION_VALUE_MAXDOUBLE << 0.1;
    QTest::newRow("Lim_Variable")  <<  "Lim_Variable"  << true  << 1e+5   << 1e+30                                << 1e+15;
    QTest::newRow("Tol_Bound")     <<  "Tol_Bound"     << true  << 3e-13  << 1e-5                                 << 1e-7;
    QTest::newRow("HEAPLIMIT")     <<  "HEAPLIMIT"     << true  << 0.0    << gams::studio::OPTION_VALUE_MAXDOUBLE << 1e+20;
    QTest::newRow("HessianMemFac") <<  "HessianMemFac" << true  << 0.0    << gams::studio::OPTION_VALUE_MAXDOUBLE << 0.0;
    QTest::newRow("Lim_Hess_Est")  <<  "Lim_Hess_Est"  << true  << 1.0    << gams::studio::OPTION_VALUE_MAXDOUBLE << 1e+4;
    QTest::newRow("Lim_Time")      <<  "Lim_Time"      << true  << 0.0    << gams::studio::OPTION_VALUE_MAXDOUBLE << 10000.0;
}

void TestConopt4Option::testOptionDoubleType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(double, lowerBound);
    QFETCH(double, upperBound);
    QFETCH(double, defaultValue);

    QCOMPARE( mOption->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( mOption->getOptionType(optionName),  optTypeDouble);
    QCOMPARE( mOption->getLowerBound(optionName).toDouble(), lowerBound );
    QCOMPARE( mOption->getUpperBound(optionName).toDouble(), upperBound );
    QCOMPARE( mOption->getDefaultValue(optionName).toDouble(), defaultValue );
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
        QVERIFY( mOption->getNameFromSynonym(optionSynonym).toUpper().isEmpty() );
        QVERIFY( !mOption->isASynonym(optionName) );
    } else {
       QVERIFY( mOption->isASynonym(optionSynonym) );
       QCOMPARE( mOption->getNameFromSynonym(optionSynonym).toUpper(), optionName.toUpper() );
    }
}

void TestConopt4Option::testOptionGroup_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<int>("groupNumber");
    QTest::addColumn<QString>("optionGroupName");
    QTest::addColumn<QString>("optionGroupDescription");
    QTest::addColumn<QString>("optionType");

    QTest::newRow("DF_Method_1")      << "DF_Method"       << 1 << "a" << "Algorithmic options" << "enumint";
    QTest::newRow("Flg_Convex_1")     << "Flg_Convex"      << 1 << "a" << "Algorithmic options" << "boolean";
    QTest::newRow("Flg_Square_1")     << "Flg_Square"      << 1 << "a" << "Algorithmic options" << "boolean";
    QTest::newRow("Lim_Iteration_1")  << "Lim_Iteration"   << 1 << "a" << "Algorithmic options" << "integer";
    QTest::newRow("Mtd_Scale_1")      << "Mtd_Scale"       << 1 << "a" << "Algorithmic options" << "integer";
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

    QTest::newRow("LF2DRV_4")      << "LF2DRV"     << 4 << "h" << "Hidden options" << "integer";
    QTest::newRow("LFDEGI_4")      << "LFDEGI"     << 4 << "h" << "Hidden options" << "integer";
    QTest::newRow("LFDERR_4")      << "LFDERR"     << 4 << "h" << "Hidden options" << "integer";
    QTest::newRow("LFDLIM_4")      << "LFDLIM"     << 4 << "h" << "Hidden options" << "integer";
    QTest::newRow("LFSQPP_4")      << "LFSQPP"     << 4 << "h" << "Hidden options" << "integer";
    QTest::newRow("MAXMEM_4")      << "MAXMEM"     << 4 << "h" << "Hidden options" << "boolean";
    QTest::newRow("STATFILE_4")    << "STATFILE"   << 4 << "h" << "Hidden options" << "boolean";
    QTest::newRow("RTMINJ_4")      << "RTMINJ"     << 4 << "h" << "Hidden options" << "double";
    QTest::newRow("WRTPAR_4")      << "WRTPAR"     << 4 << "h" << "Hidden options" << "boolean";
    QTest::newRow("nobounds_4")    << "nobounds"   << 4 << "h" << "Hidden options" << "immediate";
    QTest::newRow("readfile_4")    << "readfile"   << 4 << "h" << "Hidden options" << "immediate";

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

    QCOMPARE( mOption->getGroupNumber(optionName), groupNumber );
    QCOMPARE( mOption->getGroupName(optionName), optionGroupName );
    QCOMPARE( mOption->getGroupDescription(optionName), optionGroupDescription );
    QCOMPARE( mOption->getOptionTypeName(mOption->getOptionType(optionName)), optionType );
}

void TestConopt4Option::testInvalidOption_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("nameValid");
    QTest::addColumn<bool>("synonymValid");

    QTest::newRow("DF_Method_valid")   << "DF_Method"   << true     << false;
    QTest::newRow("FLG_CONVEX_valid")  << "FLG_CONVEX"  << true     << false;
    QTest::newRow("Flg_Interv_valid")  << "Flg_Interv"  << true     << false;
    QTest::newRow("domlim_valid")      << "domlim"      << false    << true;
    QTest::newRow("iterlim_valid")     << "iterlim"     << false    << true;
    QTest::newRow("reslim_valid")      << "reslim"      << false    << true;

    QTest::newRow("LimNewSuper_invalid")    << "LimNewSuper"     << false    << false;
    QTest::newRow("Mtd_RedHessian_invalid") << "Mtd_RedHessian"  << false    << false;
    QTest::newRow("Tol_IFix_invalid")       << "Tol_IFix"        << false    << false;
}

void TestConopt4Option::testInvalidOption()
{
    QFETCH(QString, optionName);
    QFETCH(bool, nameValid);
    QFETCH(bool, synonymValid);

    QCOMPARE( mOption->isValid(optionName), nameValid);
    QCOMPARE( mOption->isASynonym(optionName), synonymValid);
}

void TestConopt4Option::testWriteOptionFile()
{
    QList<OptionItem> items;
    items.append(OptionItem("DF_Method", "1", -1, -1));
    items.append(OptionItem("Lim_Iteration", "100", -1, -1));
    items.append(OptionItem("cooptfile", "C:/Users/Dude/coopt.file", -1, -1));
    items.append(OptionItem("Tol_Bound", "5.e-9", -1, -1));
//    items.append(OptionItem("readfile", "this is read file", -1, -1));
    QVERIFY( mOption->writeOptionParameterFile(items, CommonPaths::defaultWorkingDir(), "conopt4.opt") );

    QFile inputFile(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("conopt4.opt"));
    int i = 0;
    if (inputFile.open(QIODevice::ReadOnly)) {
       QTextStream in(&inputFile);
       while (!in.atEnd()) {
          QStringList strList = in.readLine().split( "=" );
          OptionItem item = items.at(i);
          switch(i) {
          case 0:
              QCOMPARE("DF_Method", item.key);
              QCOMPARE("1", item.value);
              break;
          case 1:
              QCOMPARE("Lim_Iteration", item.key);
              QCOMPARE("100", item.value);
              break;
          case 2:
              QCOMPARE("cooptfile", item.key);
              QCOMPARE("C:/Users/Dude/coopt.file", item.value);
              break;
          case 3:
              QCOMPARE("Tol_Bound", item.key);
              QCOMPARE("5.E-9", item.value.toUpper());
              break;
          default:
              break;
          }
          i++;
       }
       inputFile.close();
    }
    QCOMPARE(i, items.size());

}

void TestConopt4Option::cleanupTestCase()
{
    if (mOption)
       delete mOption;
}

QTEST_MAIN(TestConopt4Option)
