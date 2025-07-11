#include <QtTest>

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
    mProcess->setCheckouDuration("Some Time");
    QCOMPARE(mProcess->checkoutDuration(), "Some Time");
}

void TestGamsGetKeyProcess::test_execute_error()
{
    auto data = mProcess->execute();
    auto error = mProcess->errorMessage();
    QVERIFY(data.isEmpty());
    QVERIFY(!error.isEmpty());
}

QTEST_APPLESS_MAIN(TestGamsGetKeyProcess)

#include "tst_testgamsgetkeyprocess.moc"
