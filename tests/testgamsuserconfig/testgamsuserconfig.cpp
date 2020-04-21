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
#include "exception.h"
#include "commonpaths.h"
#include "testgamsuserconfig.h"

using gams::studio::CommonPaths;

void TestGamsUserConfig::testUserConfigDir()
{
    qDebug() << QString("QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)=[%1]").arg(CommonPaths::gamsUserConfigDir());
    QVERIFY(true);
}

void TestGamsUserConfig::testVersionFormat_data()
{
    QTest::addColumn<QString>("version");
    QTest::addColumn<bool>("valid");

    QTest::newRow("1")       << "1"        << false;
    QTest::newRow("0")       << "0"        << false;
    QTest::newRow("09")      << "09"       << false;
    QTest::newRow("28")      << "28"       << true;
    QTest::newRow("30")      << "30"       << true;
    QTest::newRow("55")      << "55"       << true;
    QTest::newRow("100")     << "100"      << false;
    QTest::newRow("123")     << "123"      << false;
    QTest::newRow("1234")    << "1234"     << false;
    QTest::newRow("1.23")    << "1.23"     << false;
    QTest::newRow("24.7")    << "24.7"     << true;
    QTest::newRow("30.1")    << "30.1"     << true;
    QTest::newRow("30.0")    << "30.0"     << true;
    QTest::newRow("30.10")   << "30.10"    << false;
    QTest::newRow("23.01")   << "23.01"    << false;
    QTest::newRow("30.1.0")  << "30.1.0"   << true;
    QTest::newRow("30.1.2")  << "30.1.2"   << true;
    QTest::newRow("10.23")   << "10.23"    << false;
    QTest::newRow("27.0.11") << "27.0.11"  << false;
    QTest::newRow("30.11.2") << "30.11.2"  << false;

    QTest::newRow("30.1.02")  << "30.1.02"   << false;
    QTest::newRow("30.1.2.1") << "30.1.2.1"  << false;
    QTest::newRow("30.")      << "30."       << false;
    QTest::newRow("30.1.")    << "30.1."     << false;
    QTest::newRow("30.1.2.")  << "30.1.2."   << false;
    QTest::newRow(".1 ")      << ".1"        << false;
    QTest::newRow(".2. ")     << ".2."       << false;

    QTest::newRow("")       << ""       << false;
    QTest::newRow(" ")      << " "      << false;
    QTest::newRow("  ")     << "  "     << false;
    QTest::newRow(".")      << "."      << false;
    QTest::newRow("..")     << ".."     << false;
    QTest::newRow("...")    << "..."    << false;
    QTest::newRow("x")      << "x"      << false;
    QTest::newRow("x.y")    << "x.y"    << false;
    QTest::newRow("x.1")    << "x.1"    << false;
    QTest::newRow("1.y")    << "1.y"    << false;
    QTest::newRow("x.y.z")  << "x.y.z"  << false;
    QTest::newRow("x.y.1")  << "x.y.1"  << false;
    QTest::newRow("30.y.1") << "30.y.1" << false;
}

void TestGamsUserConfig::testVersionFormat()
{
    QFETCH(QString, version);
    QFETCH(bool, valid);

//    QRegExp re("[1-9][0-9]*(\\.([0-9]|[1-9][0-9]*)(\\.([0-9]|[1-9][0-9]*))?)?");
    QRegExp re("[1-9][0-9](\\.([0-9])(\\.([0-9]))?)?");
    QCOMPARE( re.exactMatch(version) , valid);
}

QTEST_MAIN(TestGamsUserConfig)
