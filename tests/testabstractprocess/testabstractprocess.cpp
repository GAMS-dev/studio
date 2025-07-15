/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "testabstractprocess.h"

#include <QSignalSpy>

DummyProcess::DummyProcess(QObject *parent)
    : gams::studio::AbstractProcess("dummy", parent) {

}

DummyProcess::~DummyProcess() {

}

void DummyProcess::execute() {

}

QProcess::ProcessState DummyProcess::state() const {
    return mProcess.state();
}

void DummyProcess::readStdOut() {

}

void DummyProcess::readStdErr() {

}

void TestAbstractProcess::initTestCase() {
    process = new DummyProcess;
}

void TestAbstractProcess::cleanupTestCase() {
    delete process;
}

void TestAbstractProcess::testInputFile()
{
    const QString inputFile("model.gms");
    process->setInputFile(inputFile);
    QCOMPARE(process->inputFile(), inputFile);
}

void TestAbstractProcess::testExecute()
{
    process->execute();
}

void TestAbstractProcess::testInterrupt()
{
    process->interrupt();
}

void TestAbstractProcess::testTerminate()
{
    process->terminate();
}

void TestAbstractProcess::testState()
{
    switch (process->state()) {
    case QProcess::NotRunning:
    case QProcess::Running:
    case QProcess::Starting:
        break;
    default:
        QVERIFY2(false, "Invalid process state");
        break;
    }
}

void TestAbstractProcess::testApplication()
{
    QCOMPARE(process->application(), "dummy");
}

void TestAbstractProcess::testParameters_data()
{
    QTest::addColumn<QStringList>("list");
    QTest::addColumn<QStringList>("result");

    QTest::newRow("empty") << QStringList() << QStringList();
    QTest::newRow("one") << QStringList{"a"} << QStringList{"a"};
    QTest::newRow("two") << QStringList{"a","b"} << QStringList{"a","b"};
}

void TestAbstractProcess::testParameters()
{
    QFETCH(QStringList, list);
    QFETCH(QStringList, result);

    process->setParameters(list);
    QCOMPARE(process->parameters().size(), result.size());
}

void TestAbstractProcess::testDefaultParameters()
{
    QCOMPARE(process->defaultParameters().size(), 0);
}

void TestAbstractProcess::testWorkingDirectory()
{
    const QString workspace("some/working/dir");
    process->setWorkingDirectory(workspace);
    QCOMPARE(process->workingDirectory(), workspace);
}

void TestAbstractProcess::testGroupId()
{
    const int groupId = 2;
    process->setProjectId(groupId);
    QCOMPARE(process->projectId(), groupId);
}

void TestAbstractProcess::testExitCode()
{
    process->exitCode();
}

void TestAbstractProcess::testFinishConnectable()
{
    qRegisterMetaType<QProcess::ProcessState>();
    QSignalSpy spy(process, &gams::studio::AbstractProcess::finished);
    QVERIFY(spy.isValid());
}

void TestAbstractProcess::testNewStdChannelDataConnectable()
{
    QSignalSpy spy(process, SIGNAL(newStdChannelData(const QByteArray&)));
    QVERIFY(spy.isValid());
}

void TestAbstractProcess::testStateChangedConnectable()
{
    qRegisterMetaType<QProcess::ProcessState>();
    QSignalSpy spy(process, &gams::studio::AbstractProcess::stateChanged);
    QVERIFY(spy.isValid());
}

void TestAbstractProcess::testNewProcessCallConnectable()
{
    QSignalSpy spy(process, SIGNAL(newProcessCall(const QString&, const QString&)));
    QVERIFY(spy.isValid());
}

QTEST_MAIN(TestAbstractProcess)
