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
       QFAIL("expected successful read of optgams.def, but failed");
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

void TestCPLEXOption::cleanupTestCase()
{
    if (cplexOption)
       delete cplexOption;
}

QTEST_MAIN(TestCPLEXOption)
