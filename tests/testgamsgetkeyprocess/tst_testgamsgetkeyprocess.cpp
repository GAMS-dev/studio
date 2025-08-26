/**
 * GAMS Studio
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
#include <QTest>
#include <QSignalSpy>

#include "commonpaths.h"
#include "gamsgetkeyprocess.h"

using namespace gams::studio;

class TestGamsGetKeyProcess : public QObject
{
    Q_OBJECT

public:
    TestGamsGetKeyProcess();
    ~TestGamsGetKeyProcess();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_getset();
    void test_execute_error();

private:
    GamsGetKeyProcess *mProcess;
};

TestGamsGetKeyProcess::TestGamsGetKeyProcess()
{
    CommonPaths::setSystemDir(GAMS_DISTRIB_PATH);
}

TestGamsGetKeyProcess::~TestGamsGetKeyProcess()
{

}

void TestGamsGetKeyProcess::initTestCase()
{
    mProcess = new GamsGetKeyProcess;
}

void TestGamsGetKeyProcess::cleanupTestCase()
{
    delete mProcess;
}

void TestGamsGetKeyProcess::test_getset()
{
    mProcess->setAlpId("OK");
    QCOMPARE(mProcess->alpId(), "OK");
    mProcess->setCheckoutDuration("Some Time");
    QCOMPARE(mProcess->checkoutDuration(), "Some Time");
    mProcess->setCheckoutDuration(QString());
    mProcess->setOnPremSever("some string data");
    QCOMPARE(mProcess->onPremSever(), "some string data");
    QVERIFY(!mProcess->verboseOutput());
    mProcess->setVerboseOutput(true);
    QVERIFY(mProcess->verboseOutput());
    mProcess->setVerboseOutput(false);
    QVERIFY(mProcess->content().isEmpty());
    QVERIFY(mProcess->logMessages().isEmpty());
    QVERIFY(mProcess->onPremCertPath().isEmpty());
    mProcess->setOnPremCertPath("/home/user/Documents/GAMS");
    QCOMPARE(mProcess->onPremCertPath(), "/home/user/Documents/GAMS");
}

void TestGamsGetKeyProcess::test_execute_error()
{
    QSignalSpy spy(mProcess, SIGNAL(finished(int)));
    mProcess->setAlpId(QString());
    mProcess->setCheckoutDuration(QString());
    mProcess->setOnPremSever(QString());
    mProcess->setOnPremCertPath(QString());
    mProcess->execute();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QVERIFY(args.at(0).typeId() == QMetaType::Int);
    QVERIFY(mProcess->content().isEmpty());
    QVERIFY(!mProcess->logMessages().isEmpty());
    mProcess->clearState();
    QVERIFY(mProcess->content().isEmpty());
    QVERIFY(mProcess->logMessages().isEmpty());
}

QTEST_APPLESS_MAIN(TestGamsGetKeyProcess)

#include "tst_testgamsgetkeyprocess.moc"
