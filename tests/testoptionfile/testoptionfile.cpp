/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
#include "testoptionfile.h"
#include "gclgms.h"
#include "optcc.h"

using gams::studio::CommonPaths;

void TestOptionFile::initTestCase()
{
    QString datafile = QFINDTESTDATA("optdummy.def");
    if (datafile.isEmpty())
        EXIT_FAILURE;

    // given
    const QString expected = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    CommonPaths::setSystemDir(expected.toLatin1());
    // when
    optionTokenizer = new OptionTokenizer(QString("optdummy.def"), QFileInfo(datafile).absolutePath());
    if  ( !optionTokenizer->getOption()->available() ) {
       QFAIL("expected successful read of optdummy.def, but failed");
    }

    // when
    char msg[GMS_SSSIZE];
    optCreateD(&mOPTHandle, CommonPaths::systemDir().toLatin1(), msg, sizeof(msg));
    if (msg[0] != '\0')
        Dcreated = false;
    else
        Dcreated = true;

    // test cplex for now
    QString optdef = "optdummy.def";
    if (optReadDefinition(mOPTHandle, QDir(CommonPaths::systemDir()).filePath(optdef).toLatin1())) {
        optdefRead = false;
    } else {
        optdefRead = true;
    }
}

void TestOptionFile::testOptionBooleanType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("defaultValue");
    QTest::addColumn<QString>("description");

    QTest::newRow("bool_0")  << "bool_0"  << true  << 0  << "description for option bool_0";
    QTest::newRow("bool_1")  << "bool_1"  << true  << 1  << "description for option bool_1";
    QTest::newRow("bool_2")  << "bool_2"  << true  << 0  << "description for option bool_2";
    QTest::newRow("bool_3")  << "bool_3"  << true  << 1  << "description for option bool_3";
}

void TestOptionFile::testOptionBooleanType()
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

void TestOptionFile::testOptionStringType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QString>("defaultValue");
    QTest::addColumn<QString>("description");

    QTest::newRow("str_0")   << "str_0"  << true  << "defval_0" << "description for option str_0";
    QTest::newRow("str_1")   << "str_1"  << true  << "defval_1" << "description for option str_1";
    QTest::newRow("str_2")   << "str_2"  << true  << "defval_2" << "description for option str_2";
    QTest::newRow("str_3")   << "str_3"  << true  << "defval_3" << "description for option str_3";
    QTest::newRow("str_4")   << "str_4"  << true  << "defval_4" << "description for option str_4";
}

void TestOptionFile::testOptionStringType()
{
    QFETCH(QString, optionName);
    QFETCH(bool, valid);
    QFETCH(QString, defaultValue);
    QFETCH(QString, description);

    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).valid, valid);
    QCOMPARE( optionTokenizer->getOption()->getOptionType(optionName),  optTypeString);
    QCOMPARE( optionTokenizer->getOption()->getOptionDefinition(optionName).description, description);
    QCOMPARE( optionTokenizer->getOption()->getDefaultValue(optionName).toString(), defaultValue );
}

void TestOptionFile::testOptionEnumIntType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("valueIndex");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<int>("value");
    QTest::addColumn<QString>("description");

    QTest::newRow("EnumInt_1_0")  << "EnumInt_1"  << true  << 0 << false << 1  << "enumint_1_0";
    QTest::newRow("EnumInt_1_1")  << "EnumInt_1"  << true  << 1 << false << 2  << "enumint_1_1";

    QTest::newRow("EnumInt_2_0")  << "EnumInt_2"  << true  << 0 << false << 1  << "enumint_2_0";
    QTest::newRow("EnumInt_2_1")  << "EnumInt_2"  << true  << 1 << false << 2  << "enumint_2_1";
    QTest::newRow("EnumInt_2_2")  << "EnumInt_2"  << true  << 2 << false << 3  << "enumint_2_2";
    QTest::newRow("EnumInt_2_3")  << "EnumInt_2"  << true  << 3 << false << 4  << "enumint_2_3";

    QTest::newRow("EnumInt_3_0")  << "EnumInt_3"  << true  << 0 << false << 1  << "enumint_3_0";
    QTest::newRow("EnumInt_3_1")  << "EnumInt_3"  << true  << 1 << false << 2  << "enumint_3_1";
    QTest::newRow("EnumInt_3_2")  << "EnumInt_3"  << true  << 2 << false << 3  << "enumint_3_2";
    QTest::newRow("EnumInt_3_3")  << "EnumInt_3"  << true  << 3 << false << 4  << "enumint_3_3";
    QTest::newRow("EnumInt_3_4")  << "EnumInt_3"  << true  << 4 << false << 5  << "enumint_3_4";
    QTest::newRow("EnumInt_3_5")  << "EnumInt_3"  << true  << 5 << false << 6  << "enumint_3_5";

    QTest::newRow("EnumInt_4_0")  << "EnumInt_4"  << true  << 0 << false << 1  << "enumint_4_0";
    QTest::newRow("EnumInt_4_1")  << "EnumInt_4"  << true  << 1 << false << 2  << "enumint_4_1";
    QTest::newRow("EnumInt_4_2")  << "EnumInt_4"  << true  << 2 << false << 3  << "enumint_4_2";
    QTest::newRow("EnumInt_4_3")  << "EnumInt_4"  << true  << 3 << false << 4  << "enumint_4_3";
    QTest::newRow("EnumInt_4_4")  << "EnumInt_4"  << true  << 4 << false << 5  << "enumint_4_4";
    QTest::newRow("EnumInt_4_5")  << "EnumInt_4"  << true  << 5 << false << 6  << "enumint_4_5";
    QTest::newRow("EnumInt_4_6")  << "EnumInt_4"  << true  << 6 << false << 7  << "enumint_4_6";
    QTest::newRow("EnumInt_4_7")  << "EnumInt_4"  << true  << 7 << false << 8  << "enumint_4_7";

    QTest::newRow("EnumInt_5_0")  << "EnumInt_5"  << true  << 0 << false << 1  << "enumint_5_0";
    QTest::newRow("EnumInt_5_1")  << "EnumInt_5"  << true  << 1 << false << 2  << "enumint_5_1";
    QTest::newRow("EnumInt_5_2")  << "EnumInt_5"  << true  << 2 << false << 3  << "enumint_5_2";
    QTest::newRow("EnumInt_5_3")  << "EnumInt_5"  << true  << 3 << false << 4  << "enumint_5_3";
    QTest::newRow("EnumInt_5_4")  << "EnumInt_5"  << true  << 4 << false << 5  << "enumint_5_4";
    QTest::newRow("EnumInt_5_5")  << "EnumInt_5"  << true  << 5 << false << 6  << "enumint_5_5";
    QTest::newRow("EnumInt_5_6")  << "EnumInt_5"  << true  << 6 << false << 7  << "enumint_5_6";
    QTest::newRow("EnumInt_5_7")  << "EnumInt_5"  << true  << 7 << false << 8  << "enumint_5_7";
    QTest::newRow("EnumInt_5_8")  << "EnumInt_5"  << true  << 8 << false << 9  << "enumint_5_8";
    QTest::newRow("EnumInt_5_9")  << "EnumInt_5"  << true  << 9 << false << 10 << "enumint_5_9";
}

void TestOptionFile::testOptionEnumIntType()
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

void TestOptionFile::testOptionEnumStrType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("valueIndex");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<QString>("value");
    QTest::addColumn<QString>("description");

    QTest::newRow("EnumStr_1_0")  << "EnumStr_1"  << true  << 0 << false << "str11"  << "enumstr_1_0";
    QTest::newRow("EnumStr_1_1")  << "EnumStr_1"  << true  << 1 << false << "str12"  << "enumstr_1_1";

    QTest::newRow("EnumStr_2_0")  << "EnumStr_2"  << true  << 0 << false << "str21"  << "enumstr_2_0";
    QTest::newRow("EnumStr_2_1")  << "EnumStr_2"  << true  << 1 << false << "str22"  << "enumstr_2_1";
    QTest::newRow("EnumStr_2_2")  << "EnumStr_2"  << true  << 2 << false << "str23"  << "enumstr_2_2";
    QTest::newRow("EnumStr_2_3")  << "EnumStr_2"  << true  << 3 << false << "str24"  << "enumstr_2_3";

    QTest::newRow("EnumStr_3_0")  << "EnumStr_3"  << true  << 0 << false << "str31"  << "enumstr_3_0";
    QTest::newRow("EnumStr_3_1")  << "EnumStr_3"  << true  << 1 << false << "str32"  << "enumstr_3_1";
    QTest::newRow("EnumStr_3_2")  << "EnumStr_3"  << true  << 2 << false << "str33"  << "enumstr_3_2";
    QTest::newRow("EnumStr_3_3")  << "EnumStr_3"  << true  << 3 << false << "str34"  << "enumstr_3_3";
    QTest::newRow("EnumStr_3_4")  << "EnumStr_3"  << true  << 4 << false << "str35"  << "enumstr_3_4";
    QTest::newRow("EnumStr_3_5")  << "EnumStr_3"  << true  << 5 << false << "str36"  << "enumstr_3_5";
}

void TestOptionFile::testOptionEnumStrType()
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
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).value.toString(), value );
    QCOMPARE( optionTokenizer->getOption()->getValueList(optionName).at(valueIndex).description.toLower(), description.toLower() );
}

void TestOptionFile::testOptionDoubleType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<double>("lowerBound");
    QTest::addColumn<double>("upperBound");
    QTest::addColumn<double>("defaultValue");

    QTest::newRow("double_0")  <<  "double_0"  << true  << 0.1      << 2857.14                                        << 2400.55;
    QTest::newRow("double_1")  <<  "double_1"  << true  << -0.9     << 5714.29                                        << 2253.07;
    QTest::newRow("double_2")  <<  "double_2"  << true  << -1.9     << 8571.43                                        << 6711.87;
    QTest::newRow("double_3")  <<  "double_3"  << true  << -2.9     << 11428.6                                        << 9124.44;

    QTest::newRow("double_4")  <<  "double_4"  << true  << -3.9     << gams::studio::option::OPTION_VALUE_MAXDOUBLE   << 9.11647e+298;
    QTest::newRow("double_5")  <<  "double_5"  << true  << -4.9     << gams::studio::option::OPTION_VALUE_MAXDOUBLE   << 1.97551e+298;
    QTest::newRow("double_6")  <<  "double_6"  << true  << -5.9     << gams::studio::option::OPTION_VALUE_MAXDOUBLE   << 3.35223e+298;
    QTest::newRow("double_7")  <<  "double_7"  << true  << -6.9     << gams::studio::option::OPTION_VALUE_MAXDOUBLE   << 7.6823e+298;
    QTest::newRow("double_8")  <<  "double_8"  << true  << -7.9     << gams::studio::option::OPTION_VALUE_MAXDOUBLE   << 2.77775e+298;
    QTest::newRow("double_9")  <<  "double_9"  << true  << -8.9     << gams::studio::option::OPTION_VALUE_MAXDOUBLE   << 5.5397e+298;

}

void TestOptionFile::testOptionDoubleType()
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

void TestOptionFile::testOptionIntegerType_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("lowerBound");
    QTest::addColumn<int>("upperBound");
    QTest::addColumn<int>("defaultValue");

    QTest::newRow("int_0")  << "int_0"   << true  << 0    << 10    << 0;
    QTest::newRow("int_1")  << "int_1"   << true  << -1   << 11    << 1;
    QTest::newRow("int_2")  << "int_2"   << true  << -2   << 12    << 2;
    QTest::newRow("int_3")  << "int_3"   << true  << -3   << 13    << 3;
    QTest::newRow("int_4")  << "int_4"   << true  << -4   << 14    << 4;
    QTest::newRow("int_5")  << "int_5"   << true  << 5    << 50000 << 50;
    QTest::newRow("int_6")  << "int_6"   << true  << 6    << 60000 << 60;
    QTest::newRow("int_7")  << "int_7"   << true  << 7    << 70000 << 70;
    QTest::newRow("int_8")  << "int_8"   << true  << 8    << 80000 << 80;
    QTest::newRow("int_9")  << "int_9"   << true  << 9    << 90000 << 90;
}

void TestOptionFile::testOptionIntegerType()
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

void TestOptionFile::testOptionGroup_data()
{
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<int>("groupNumber");
    QTest::addColumn<QString>("optionGroupName");
    QTest::addColumn<QString>("optionType");

    QTest::newRow("bool_0")      << "bool_0"         << 1 << "gr_FirstGroup" << "boolean";
    QTest::newRow("bool_1")      << "bool_1"         << 1 << "gr_FirstGroup" << "boolean";
    QTest::newRow("bool_1")      << "bool_2"         << 1 << "gr_FirstGroup" << "boolean";
    QTest::newRow("bool_2")      << "bool_3"         << 1 << "gr_FirstGroup" << "boolean";

    QTest::newRow("int_0")      << "int_0"         << 1 << "gr_FirstGroup" << "integer";
    QTest::newRow("int_1")      << "int_1"         << 1 << "gr_FirstGroup" << "integer";
    QTest::newRow("int_1")      << "int_2"         << 1 << "gr_FirstGroup" << "integer";
    QTest::newRow("int_2")      << "int_3"         << 1 << "gr_FirstGroup" << "integer";

    QTest::newRow("EnumInt_1")   << "EnumInt_1"      << 1 << "gr_FirstGroup" << "enumint";
    QTest::newRow("EnumInt_2")   << "EnumInt_2"      << 1 << "gr_FirstGroup" << "enumint";
    QTest::newRow("EnumInt_3")   << "EnumInt_3"      << 1 << "gr_FirstGroup" << "enumint";
    QTest::newRow("EnumInt_4")   << "EnumInt_4"      << 1 << "gr_FirstGroup" << "enumint";
    QTest::newRow("EnumInt_5")   << "EnumInt_5"      << 1 << "gr_FirstGroup" << "enumint";

    QTest::newRow("double_0")    << "double_0"       << 2 << "gr_SecondGroup" << "double";
    QTest::newRow("double_1")    << "double_1"       << 2 << "gr_SecondGroup" << "double";
    QTest::newRow("double_2")    << "double_2"       << 2 << "gr_SecondGroup" << "double";
    QTest::newRow("double_3")    << "double_3"       << 2 << "gr_SecondGroup" << "double";
    QTest::newRow("double_4")    << "double_4"       << 2 << "gr_SecondGroup" << "double";
    QTest::newRow("double_5")    << "double_5"       << 2 << "gr_SecondGroup" << "double";
    QTest::newRow("double_6")    << "double_6"       << 2 << "gr_SecondGroup" << "double";
    QTest::newRow("double_7")    << "double_7"       << 2 << "gr_SecondGroup" << "double";
    QTest::newRow("double_8")    << "double_8"       << 2 << "gr_SecondGroup" << "double";
    QTest::newRow("double_9")    << "double_9"       << 2 << "gr_SecondGroup" << "double";

    QTest::newRow("str_0")       << "str_0"         << 3 << "gr_ThirdGroup" << "string";
    QTest::newRow("str_1")       << "str_1"         << 3 << "gr_ThirdGroup" << "string";
    QTest::newRow("str_2")       << "str_2"         << 3 << "gr_ThirdGroup" << "string";
    QTest::newRow("str_3")       << "str_3"         << 3 << "gr_ThirdGroup" << "string";
    QTest::newRow("str_4")       << "str_4"         << 3 << "gr_ThirdGroup" << "string";
    QTest::newRow("str_5")       << "str_5"         << 3 << "gr_ThirdGroup" << "string";
}

void TestOptionFile::testOptionGroup()
{
    QFETCH(QString, optionName);
    QFETCH(int, groupNumber);
    QFETCH(QString, optionGroupName);
    QFETCH(QString, optionType);

    QCOMPARE( optionTokenizer->getOption()->getGroupNumber(optionName), groupNumber );
    QCOMPARE( optionTokenizer->getOption()->getGroupName(optionName), optionGroupName );
    QCOMPARE( optionTokenizer->getOption()->getOptionTypeName(optionTokenizer->getOption()->getOptionType(optionName)), optionType );
}

void TestOptionFile::testOptionSynonym_data()
{
    QTest::addColumn<QString>("optionSynonym");
    QTest::addColumn<QString>("optionName");

    QTest::newRow("EnumInt_2")  << "eint_2"   << "EnumInt_2";
    QTest::newRow("EnumInt_4")  << "eint_4"   << "EnumInt_4";

    QTest::newRow("str_0")  << "s_0"   << "str_0";
    QTest::newRow("str_2")  << "s_2"   << "str_2";
    QTest::newRow("str_4")  << "s_4"   << "str_4";
}

void TestOptionFile::testOptionSynonym()
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

void TestOptionFile::testNonExistReadOptionFile()
{
    // when
    QString optFile = QDir(".").absoluteFilePath("dummy.opt");
    QList<SolverOptionItem *> items = optionTokenizer->readOptionFile(optFile, QTextCodec::codecForLocale());

    // then
    QCOMPARE( items.size(), 0);
}

void TestOptionFile::testEOLChars()
{
    char eolchars[256];
    int numchar = optEOLChars( mOPTHandle, eolchars);

    QCOMPARE( 0, numchar );
    QVERIFY( QString::fromLatin1(eolchars).isEmpty() );
}

void TestOptionFile::cleanupTestCase()
{
    if (mOPTHandle)
        optFree(&mOPTHandle);

    if (optionTokenizer)
        delete optionTokenizer;
}

QTEST_MAIN(TestOptionFile)
