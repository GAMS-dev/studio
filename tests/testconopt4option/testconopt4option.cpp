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
#include "commonpaths.h"

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

    QTest::newRow("DF_Method")  << "DF_Method"   << true  << 2  << 0;
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
    QTest::newRow("Mtd_RedHess")   << "Mtd_RedHess"   << true   << 0  << 1                                  << 0;
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

    QCOMPARE( optionTokenizer->getOption()->isValid(optionName), nameValid);
    QCOMPARE( optionTokenizer->getOption()->isASynonym(optionName), synonymValid);
}

void TestConopt4Option::testReadOptionFile()
{
    // given
    QFile outputFile(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("conopt4.op2"));
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
        QFAIL("expected to open conopt4.op2 to write, but failed");

    QTextStream out(&outputFile);
    out << "* This is comment line" << endl;
    out << "DF_Method 1" << endl;
    out << "Lim_Iteration=100" << endl;
    out << "Flg_Hessian 1" << endl;
    out << "cooptfile \"C:/Users/Dude/coopt.file\"" << endl;
    out << "Tol_Bound=5.E-9" << endl;
    out << "HEAPLIMIT 1e20" << endl;
    outputFile.close();

    // when
    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("conopt4.op2");
    QList<OptionItem> items = optionTokenizer->readOptionParameterFile(optFile);

    // then
    QCOMPARE( items.size(), 6 );

    QVERIFY( containKey (items,"DF_Method") );
    QCOMPARE( getValue(items,"DF_Method").toInt(),  QVariant("1").toInt() );

    QVERIFY( containKey (items,"Lim_Iteration") );
    QCOMPARE( getValue(items,"Lim_Iteration").toInt(),  QVariant("100").toInt() );

    QVERIFY( containKey (items,"Flg_Hessian") );
    QCOMPARE( getValue(items,"Flg_Hessian").toInt(),  QVariant("1").toInt() );

    QVERIFY( containKey (items,"Tol_Bound") );
    QCOMPARE( getValue(items,"Tol_Bound").toDouble(), QVariant("5.E-9").toDouble() );

    QVERIFY( containKey (items,"HEAPLIMIT") );
    QCOMPARE( getValue(items,"HEAPLIMIT").toDouble(), QVariant("1e20").toDouble() );

    QVERIFY( containKey (items,"cooptfile") );
    QCOMPARE( getValue(items,"cooptfile").toString(), QVariant("C:/Users/Dude/coopt.file").toString() );
}

void TestConopt4Option::testNonExistReadOptionFile()
{
    // when
    QString optFile = QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("conopt4.op012345");
    QList<OptionItem> items = optionTokenizer->readOptionParameterFile(optFile);

    // then
    QCOMPARE( items.size(), 0);
}

void TestConopt4Option::testWriteOptionFile()
{
    // given
    QList<OptionItem> items;
    items.append(OptionItem("DF_Method", "1"));
    items.append(OptionItem("Lim_Iteration", "100"));
    items.append(OptionItem("Tol_Bound", "5.e-9"));
    items.append(OptionItem("Tol_Optimality", "1.e-10"));
    items.append(OptionItem("cooptfile", "C:/Users/Programs Files/Dude/coopt.file"));
//    items.append(OptionItem("readfile", "this is read file"));  => optTypeImmediate

    // when
    QVERIFY( optionTokenizer->writeOptionParameterFile(items, CommonPaths::defaultWorkingDir(), "conopt4.opt") );

    // then
    QFile inputFile(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("conopt4.opt"));
    int i = 0;
    if (inputFile.open(QIODevice::ReadOnly)) {
       QTextStream in(&inputFile);
       while (!in.atEnd()) {
          QStringList strList = in.readLine().split( "=" );

          QVERIFY( containKey (items, strList.at(0)) );
          if ((QString::compare(strList.at(0), "DF_Method", Qt::CaseInsensitive)==0) ||
              (QString::compare(strList.at(0), "Lim_Iteration", Qt::CaseInsensitive)==0)
             ) {
             QCOMPARE( getValue(items, strList.at(0)).toInt(), strList.at(1).toInt() );
          } else if ((QString::compare(strList.at(0), "Tol_Bound", Qt::CaseInsensitive)==0) ||
                     (QString::compare(strList.at(0), "Tol_Optimality", Qt::CaseInsensitive)==0)) {
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

void TestConopt4Option::cleanupTestCase()
{
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
