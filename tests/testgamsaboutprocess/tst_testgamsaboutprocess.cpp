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

#include "gamsaboutprocess.h"

using namespace gams::studio;

class TestGamsAboutProcess : public QObject
{
    Q_OBJECT

public:
    TestGamsAboutProcess();
    ~TestGamsAboutProcess();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_getset();

private:
    GamsAboutProcess *mProcess;
};

TestGamsAboutProcess::TestGamsAboutProcess()
{

}

TestGamsAboutProcess::~TestGamsAboutProcess()
{

}

void TestGamsAboutProcess::initTestCase()
{
    mProcess = new GamsAboutProcess;
}

void TestGamsAboutProcess::cleanupTestCase()
{
    delete mProcess;
}

void TestGamsAboutProcess::test_getset()
{
    QVERIFY(mProcess->content().isEmpty());
    QVERIFY(mProcess->logMessages().isEmpty());
    QVERIFY(mProcess->curDir().isEmpty());
    QString path("/some/path");
    mProcess->setCurDir(path);
    QCOMPARE(mProcess->curDir(), path);
}

QTEST_APPLESS_MAIN(TestGamsAboutProcess)

#include "tst_testgamsaboutprocess.moc"
