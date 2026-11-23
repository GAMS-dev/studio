/*
 * This file is part of the GAMS Studio project.
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
#include "testcompleter.h"
#include "editors/codecompleter.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"

#include <QTemporaryDir>

using namespace gams::studio;
using namespace gams::studio::syntax;

QByteArray TestCompleter::describe(int receive, int expect, QStringList types)
{
    QString descript("Filter: %1 != %2  -> diff: %3\n[%4]");
    return descript.arg(QString::number(receive, 16),
                        QString::number(expect, 16),
                        QString::number((expect & ~receive) | (receive & ~expect), 16),
                        types.join(",")).toLatin1();
}

void TestCompleter::initTestCase()
{
    mCompleter = new CodeCompleter(nullptr);
    connect(mCompleter, &CodeCompleter::scanSyntax, &mSynSim, &SyntaxSimulator::scanSyntax);
}

void TestCompleter::testDco()
{
    QString line;
    int expect;

    // ===== TEST: empty line
    line = "";
    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::Standard, 0);

    expect = cc_Start | ccSnippet;
    mCompleter->updateFilter( 0, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    // ===== TEST: empty line in non-GAMS blocks
    line = "";
    expect = ccDcoEnd | cc_Start;

    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::CommentBlock, 0);
    mCompleter->updateFilter( 0, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::IgnoredBlock, 5);
    mCompleter->updateFilter( 0, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::EmbeddedBody, 19);
    mCompleter->updateFilter( 0, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::EmbeddedBody, 20);
    mCompleter->updateFilter( 0, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    expect = ccResEnd | cc_Start;
    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::EmbeddedBody, 0);
    mCompleter->updateFilter( 0, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

}

void TestCompleter::testDeclaration()
{
    QString line;
    int expect;

    // ===== TEST: set declaration
    line = "    i 'canning plants' / seattle  san-diego /;";
    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::Declaration, 16);
    mSynSim.addBlockSyntax(4, SyntaxKind::Declaration, 16);
    mSynSim.addBlockSyntax(6, SyntaxKind::Identifier, 16);
    mSynSim.addBlockSyntax(22, SyntaxKind::IdentifierDescription, 16);
    mSynSim.addBlockSyntax(24, SyntaxKind::IdentifierAssignment, 16);
    mSynSim.addBlockSyntax(32, SyntaxKind::AssignmentLabel, 16);
    mSynSim.addBlockSyntax(33, SyntaxKind::IdentifierAssignment, 16);
    mSynSim.addBlockSyntax(44, SyntaxKind::AssignmentLabel, 16);
    mSynSim.addBlockSyntax(46, SyntaxKind::IdentifierAssignmentEnd, 16);
    mSynSim.addBlockSyntax(47, SyntaxKind::Semicolon, 16);

    expect = ccSysSufC | ccCtConst | ccDeclT;
    mCompleter->updateFilter( 3, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    expect = ccSysSufC | ccCtConst;
    mCompleter->updateFilter(21, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    expect = ccSysSufC | ccCtConst | ccSysDat;
    mCompleter->updateFilter(24, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

}

void TestCompleter::testPut()
{
    QString line;
    int expect;

    // ===== TEST: put command with system constant
    line = "put 'abc %system.Date%';";
    mSynSim.clearBlockSyntax();
    mSynSim.addBlockSyntax(0, SyntaxKind::Standard, 0);
    mSynSim.addBlockSyntax(3, SyntaxKind::Put, 0);
    mSynSim.addBlockSyntax(4, SyntaxKind::PutFormula, 0);
    mSynSim.addBlockSyntax(23, SyntaxKind::String, 0);
    mSynSim.addBlockSyntax(24, SyntaxKind::Semicolon, 0);

    expect = cc_Start | ccSnippet;
    mCompleter->updateFilter( 3, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    expect = cc_Res | ccSysSufR | ccSysSufC | ccCtConst | ccSnippet;
    mCompleter->updateFilter( 4, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    expect = ccSysSufC | ccCtConst;
    mCompleter->updateFilter( 6, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));

    expect = (cc_Start | ccSnippet) & ~(ccDcoStrt | ccDcoEnd);
    mCompleter->updateFilter(24, line);
    QVERIFY2(mCompleter->typeFilter() == expect, describe(mCompleter->typeFilter(), expect, mCompleter->splitTypes()));
}



QTEST_MAIN(TestCompleter)

namespace {

class LogCollector : public AbstractSystemLogger
{
public:
    void append(const QString &msg, LogMsgType type) override {
        if (type == LogMsgType::Error) errors << msg;
    }
    QStringList errors;
};

QStringList loadSnippetErrors(CodeCompleter *completer, const QString &file)
{
    LogCollector log;
    SysLogLocator::provide(&log);
    completer->loadSnippets(file);
    SysLogLocator::provide(nullptr);
    return log.errors;
}

} // namespace

void TestCompleter::testSnippetErrors_data()
{
    QTest::addColumn<QByteArray>("content");
    QTest::addColumn<QString>("error");     // part of the expected error, empty for no error

    QTest::newRow("valid")
            << QByteArray(R"([ { "trigger": "t1", "body": "x;" }, { "_trigger": "t2", "body": "Variable x;" } ])")
            << QString();
    QTest::newRow("missing comma")
            << QByteArray("[\n  { \"trigger\": \"t1\", \"body\": \"x;\" }\n  { \"trigger\": \"t2\" } ]")
            << QString("in line 3, column");
    QTest::newRow("no list")
            << QByteArray(R"({ "trigger": "t1", "body": "x;" })")
            << QString("must be a list");
    QTest::newRow("entry no object")
            << QByteArray(R"([ "t1" ])")
            << QString("Entry 1 is skipped");
    QTest::newRow("no body")
            << QByteArray(R"([ { "trigger": "t1" } ])")
            << QString("'t1' is skipped");
    QTest::newRow("one character variable trigger")
            << QByteArray(R"([ { "trigger": "v", "body": "Variable x;" } ])")
            << QString();
}

void TestCompleter::testSnippetErrors()
{
    QFETCH(QByteArray, content);
    QFETCH(QString, error);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString fileName = dir.filePath("user_snippets.json");
    QFile file(fileName);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(content);
    file.close();

    const QStringList errors = loadSnippetErrors(mCompleter, fileName);
    if (error.isEmpty()) {
        QVERIFY2(errors.isEmpty(), errors.join('\n').toUtf8());
    } else {
        QCOMPARE(errors.size(), 1);
        QVERIFY2(errors.first().contains(error), errors.first().toUtf8());
        QVERIFY2(errors.first().contains("user_snippets.json"), errors.first().toUtf8());
    }
}

void TestCompleter::testSnippetTemplate()
{
    // a missing user file is created from the template, which must load without errors
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString fileName = dir.filePath("user_snippets.json");
    const QStringList errors = loadSnippetErrors(mCompleter, fileName);
    QVERIFY(QFile::exists(fileName));
    QVERIFY2(errors.isEmpty(), errors.join('\n').toUtf8());
}
