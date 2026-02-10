/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include <QTest>

#include "searchcommon.h"

using namespace gams::studio::search;

class TestSearchCommon : public QObject
{
    Q_OBJECT

public:
    TestSearchCommon();
    ~TestSearchCommon();

private slots:
    void test_toRegularExpression_data();
    void test_toRegularExpression();

    void test_includeFiltersUnix_data();
    void test_includeFiltersUnix();
    void test_includeFiltersWindows_data();
    void test_includeFiltersWindows();

    void test_excludeFiltersUnix_data();
    void test_excludeFiltersUnix();

    void test_excludeFiltersWindows_data();
    void test_excludeFiltersWindows();

    void test_fileName_data();
    void test_fileName();

    void test_parametersDefault();
    void test_parametersGetSet();

private:
    QStringList applyRegEx(const QStringList &paths,
                               const QList<QRegularExpression> &pattern,
                               const QChar &separator);

private:
    const QStringList mUnixPaths {
        "/home/someuser/Documents/GAMS/Studio/workspace/copper.log",
        "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.log",
        "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.lst",
        "/home/someuser/Documents/GAMS/Studio/workspace/225b",
        "/home/someuser/Documents/GAMS/Studio/workspace/copper.lxi",
        "/home/someuser/Documents/GAMS/Studio/workspace/copper.lst",
        "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
        "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
        "/home/someuser/Documents/GAMS/Studio/workspace/225a",
        "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
        "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms",
        "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.lxi",
        "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
        "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
        "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gms"
    };

    const QStringList mWindowsPaths {
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.log",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.log",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.lst",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\225b",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.lxi",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.lst",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\225a",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.lxi",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
        "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gms"
    };
};

TestSearchCommon::TestSearchCommon()
{

}

TestSearchCommon::~TestSearchCommon()
{

}

void TestSearchCommon::test_toRegularExpression_data()
{

}

void TestSearchCommon::test_toRegularExpression()
{

}

void TestSearchCommon::test_includeFiltersUnix_data()
{
    QTest::addColumn<QString>("wildcard");
    QTest::addColumn<QStringList>("paths");
    QTest::addColumn<QStringList>("matched");

    QTest::newRow("none1")  << ""                 << mUnixPaths << QStringList();
    QTest::newRow("none2")  << "ran*"             << mUnixPaths << QStringList();
    QTest::newRow("none3")  << "*ran"             << mUnixPaths << QStringList();
    QTest::newRow("none4")  << "*use*"            << mUnixPaths << QStringList();
    QTest::newRow("miss1")  << "d*.gms"           << mUnixPaths << QStringList();
    QTest::newRow("miss2")  << "b*.gms"           << mUnixPaths << QStringList();
    QTest::newRow("miss3")  << "d*.gms"           << mUnixPaths << QStringList();
    QTest::newRow("star")   << "*"                << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.log",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.log",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.lst",
                               "/home/someuser/Documents/GAMS/Studio/workspace/225b",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.lxi",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.lst",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/225a",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.lxi",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gms"
    };
    QTest::newRow("list")   << "*.log, *.ref,co*" << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.log",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.log",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.lxi",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.lst",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms"
    };
    QTest::newRow("start")  << "*ref"             << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref"
    };
    QTest::newRow("middle") << "tr*.lxi"          << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.lxi"
    };
    QTest::newRow("end")    << "copper.ref*"      << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref"
    };
    QTest::newRow("single") << "copper?gms"       << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms"
    };
    QTest::newRow("squ1")   << "[ct]*.gms"       << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gms"
    };
    QTest::newRow("squ2")   << "[c-t]*.gms"       << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gms"
    };
    QTest::newRow("gms")    << "*.gms"            << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gms"
    };
    QTest::newRow("gdx")    << "*.gdx"            << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx"
    };
    QTest::newRow("zip")    << "*.zip"            << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip"
    };
    QTest::newRow("*rn*")  << "*rn*"            << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.log",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.lst",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.lxi",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gms"
    };
}

void TestSearchCommon::test_includeFiltersUnix()
{
    QFETCH(QString, wildcard);
    QFETCH(QStringList, paths);
    QFETCH(QStringList, matched);

    QList<QRegularExpression> regex;
    auto wildcards = wildcard.split(",", Qt::SkipEmptyParts);
    SearchCommon::includeFilters(wildcards, regex);
    auto result = applyRegEx(paths, regex, '/');
    matched.sort();
    result.sort();
    QCOMPARE(matched, result);
}

void TestSearchCommon::test_includeFiltersWindows_data()
{
    QTest::addColumn<QString>("wildcard");
    QTest::addColumn<QStringList>("paths");
    QTest::addColumn<QStringList>("matched");

    QTest::newRow("none1")  << ""                 << mWindowsPaths << QStringList();
    QTest::newRow("none2")  << "ran*"             << mWindowsPaths << QStringList();
    QTest::newRow("none3")  << "*ran"             << mWindowsPaths << QStringList();
    QTest::newRow("none4")  << "*use*"            << mWindowsPaths << QStringList();
    QTest::newRow("miss1")  << "b*.gms"           << mWindowsPaths << QStringList();
    QTest::newRow("miss2")  << "d*.gms"           << mWindowsPaths << QStringList();
    QTest::newRow("star")   << "*"                << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.log",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.log",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.lst",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\225b",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.lxi",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.lst",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\225a",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.lxi",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gms"
    };
    QTest::newRow("list")   << "*.log, *.ref,co*" << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.log",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.log",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.lxi",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.lst",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms"
    };
    QTest::newRow("start")  << "*ref"             << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref"
    };
    QTest::newRow("middle") << "tr*.lxi"          << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.lxi"
    };
    QTest::newRow("end")    << "copper.ref*"      << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref"
    };
    QTest::newRow("single") << "copper?gms"       << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms"
    };
    QTest::newRow("squ1")   << "[ct]*.gms"            << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gms"
    };
    QTest::newRow("squ2")   << "[c-t]*.gms"            << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gms"
    };
    QTest::newRow("gms")    << "*.gms"            << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gms"
    };
    QTest::newRow("gdx")    << "*.gdx"            << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx"
    };
    QTest::newRow("zip")    << "*.zip"            << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip"
    };
    QTest::newRow("*rn*")   << "*rn*"                << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.log",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.lst",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.lxi",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gms"
    };
}

void TestSearchCommon::test_includeFiltersWindows()
{
    QFETCH(QString, wildcard);
    QFETCH(QStringList, paths);
    QFETCH(QStringList, matched);

    QList<QRegularExpression> regex;
    auto wildcards = wildcard.split(",", Qt::SkipEmptyParts);
    SearchCommon::includeFilters(wildcards, regex);
    auto result = applyRegEx(paths, regex, '\\');
    matched.sort();
    result.sort();
    QCOMPARE(matched, result);
}

void TestSearchCommon::test_excludeFiltersUnix_data()
{
    QTest::addColumn<QString>("wildcard");
    QTest::addColumn<QStringList>("paths");
    QTest::addColumn<QStringList>("matched");

    QTest::newRow("none1")   << ""                 << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx"
    };
    QTest::newRow("none2")  << "ran*"             << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx"
    };
    QTest::newRow("none3")  << "*ran"             << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx"
    };
    QTest::newRow("none4")  << "*use*"            << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx"
    };
    QTest::newRow("miss1")  << "b*.gms"           << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx"
    };
    QTest::newRow("miss2")  << "d*.gms"           << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx"
    };
    QTest::newRow("star")   << "*"                << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.log",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.log",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.lst",
                               "/home/someuser/Documents/GAMS/Studio/workspace/225b",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.lxi",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.lst",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/225a",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.lxi",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gms"
    };
    QTest::newRow("list")   << "*.log, *.ref,co*" << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.log",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.log",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.lxi",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.lst",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms"
    };
    QTest::newRow("start")  << "*ref"             << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref"
    };
    QTest::newRow("middle") << "tr*.lxi"          << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.lxi",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref"
    };
    QTest::newRow("end")    << "copper.ref*"      << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref"
    };
    QTest::newRow("single") << "copper?gms"       << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms"
    };
    QTest::newRow("squ1")   << "[ct]*.gms"       << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gms"
    };
    QTest::newRow("squ2")   << "[c-t]*.gms"       << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gms"
    };
    QTest::newRow("gms")    << "*.gms"            << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gms"
    };
    QTest::newRow("gdx")    << "*.gdx"            << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref"
    };
    QTest::newRow("zip")    << "*.zip"            << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref"
    };
    QTest::newRow("*rn*")  << "*rn*"            << mUnixPaths << QStringList {
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.zip",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/copper.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.log",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.lst",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.lxi",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.ref",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gdx",
                               "/home/someuser/Documents/GAMS/Studio/workspace/trnsport.gms"
    };
}

void TestSearchCommon::test_excludeFiltersUnix()
{
    QFETCH(QString, wildcard);
    QFETCH(QStringList, paths);
    QFETCH(QStringList, matched);

    QList<QRegularExpression> regex;
    auto wildcards = wildcard.split(",", Qt::SkipEmptyParts);
    SearchCommon::excludeFilters(wildcards, regex);
    auto result = applyRegEx(paths, regex, '/');
    matched.sort();
    result.sort();
    QCOMPARE(matched, result);
}

void TestSearchCommon::test_excludeFiltersWindows_data()
{
    QTest::addColumn<QString>("wildcard");
    QTest::addColumn<QStringList>("paths");
    QTest::addColumn<QStringList>("matched");

    QTest::newRow("none1")   << ""                 << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx"
    };
    QTest::newRow("none2")  << "ran*"             << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref"
    };
    QTest::newRow("none3")  << "*ran"             << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx"
    };
    QTest::newRow("none4")  << "*use*"            << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx"
    };
    QTest::newRow("miss1")  << "b*.gms"           << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx"
    };
    QTest::newRow("miss2")  << "d*.gms"           << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx"
    };
    QTest::newRow("star")   << "*"                << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.log",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.log",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.lst",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\225b",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.lxi",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.lst",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\225a",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.lxi",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gms"
    };
    QTest::newRow("list")   << "*.log, *.ref,co*" << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.log",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.log",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.lxi",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.lst",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms"
    };
    QTest::newRow("start")  << "*ref"             << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref"
    };
    QTest::newRow("middle") << "tr*.lxi"          << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.lxi"
    };
    QTest::newRow("end")    << "copper.ref*"      << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref"
    };
    QTest::newRow("single") << "copper?gms"       << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms"
    };
    QTest::newRow("squ1")   << "[ct]*.gms"            << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gms"
    };
    QTest::newRow("squ2")   << "[c-t]*.gms"            << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gms"
    };
    QTest::newRow("gms")    << "*.gms"            << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gms",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gms"
    };
    QTest::newRow("gdx")    << "*.gdx"            << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref"
    };
    QTest::newRow("zip")    << "*.zip"            << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref"
    };
    QTest::newRow("*rn*")   << "*rn*"                << mWindowsPaths << QStringList {
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.zip",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\copper.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.log",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.lst",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.lxi",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.ref",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx",
                               "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gms"
    };
}

void TestSearchCommon::test_excludeFiltersWindows()
{
    QFETCH(QString, wildcard);
    QFETCH(QStringList, paths);
    QFETCH(QStringList, matched);

    QList<QRegularExpression> regex;
    auto wildcards = wildcard.split(",", Qt::SkipEmptyParts);
    SearchCommon::excludeFilters(wildcards, regex);
    auto result = applyRegEx(paths, regex, '\\');
    matched.sort();
    result.sort();
    QCOMPARE(matched, result);
}

void TestSearchCommon::test_fileName_data()
{
    QTest::addColumn<QChar>("separator");
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("fileName");


    QTest::newRow("empty1")     << QChar('\\') << QString() << QString();
    QTest::newRow("empty2")     << QChar('/') << QString() << QString();
    QTest::newRow("filename1")  << QChar('\\') << "copper.gms" << "copper.gms";
    QTest::newRow("filename2")  << QChar('/') << "copper.gms" << "copper.gms";
    QTest::newRow("unix1")      << QChar('/') << "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms"
                                << "copper.gms";
    QTest::newRow("unix2")      << QChar('\\') << "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms"
                                << "/home/someuser/Documents/GAMS/Studio/workspace/copper.gms";
    QTest::newRow("win1")       << QChar('\\') << "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx"
                                << "trnsport.gdx";
    QTest::newRow("win2")       << QChar('/')  << "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx"
                                << "C:\\Users\\someuser\\Documents\\GAMS\\Studio\\workspace\\trnsport.gdx";
}

void TestSearchCommon::test_fileName()
{
    QFETCH(QChar, separator);
    QFETCH(QString, path);
    QFETCH(QString, fileName);

    auto result = SearchCommon::fileName(path, separator);
    qDebug() << path << separator << result;
    QCOMPARE(fileName, result);
}

void TestSearchCommon::test_parametersDefault()
{
    Parameters p;
    QCOMPARE(p.searchTerm(), QString());
    QCOMPARE(p.replaceTerm(), QString());
    QVERIFY(!p.caseSensitive());
    QVERIFY(!p.useRegex());
    QVERIFY(!p.wholeWords());
    QVERIFY(!p.searchBackwards());
    QVERIFY(p.scope() == Scope::ThisFile);
    QCOMPARE(p.includeFilter(), QStringList());
    QCOMPARE(p.excludeFilter(), QStringList());
    QCOMPARE(p.directory(), QString());
    QVERIFY(p.includeSubdirs());
    QVERIFY(!p.showResults());
    QVERIFY(!p.ignoreReadOnly());
    QCOMPARE(p.regex(), QRegularExpression());
    QCOMPARE(Parameters::listSeparator(), QChar(';'));
}

void TestSearchCommon::test_parametersGetSet()
{
    Parameters p;
    p.setSearchTerm("some Text!");
    QCOMPARE(p.searchTerm(), QString("some Text!"));
    p.setReplaceTerm("e");
    QCOMPARE(p.replaceTerm(), QString("e"));
    p.setCaseSensitive(true);
    QVERIFY(p.caseSensitive());
    p.setUseRegex(true);
    QVERIFY(p.useRegex());
    p.setWholeWords(true);
    QVERIFY(p.wholeWords());
    p.setSearchBackwards(true);
    QVERIFY(p.searchBackwards());
    p.setScope(Scope::Folder);
    QVERIFY(p.scope() == Scope::Folder);
    QStringList filter { "*.gms", "*.ref", "t*" };
    p.setIncludeFilter(filter);
    QCOMPARE(p.includeFilter(), filter);
    p.setExcludeFilter(filter);
    QCOMPARE(p.excludeFilter(), filter);
    p.setDirectory("/some/path");
    QCOMPARE(p.directory(), QString("/some/path"));
    p.setIncludeSubdirs(false);
    QVERIFY(!p.includeSubdirs());
    p.setShowResults(true);
    QVERIFY(p.showResults());
    p.setIgnoreReadOnly(true);
    QVERIFY(p.ignoreReadOnly());
    QRegularExpression regex("^seattle$");
    p.setRegex(regex);
    QCOMPARE(p.regex(), regex);
}

QStringList TestSearchCommon::applyRegEx(const QStringList &paths,
                                         const QList<QRegularExpression> &patterns,
                                         const QChar &separator)
{
    QStringList matched;
    for (const QString &path : std::as_const(paths)) {
        auto fn = SearchCommon::fileName(path, separator);
        for (const QRegularExpression &pattern : std::as_const(patterns)) {
            if (pattern.match(fn).hasMatch()) {
                matched << path;
                break;
            }
        }
    }
    return matched;
}

QTEST_APPLESS_MAIN(TestSearchCommon)

#include "tst_testsearchcommon.moc"
