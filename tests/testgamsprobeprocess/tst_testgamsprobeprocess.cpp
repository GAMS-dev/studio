#include <QtTest>

#include "commonpaths.h"
#include "gamsprobeprocess.h"

using namespace gams::studio;

class TestGamsprobeProcess : public QObject
{
    Q_OBJECT

public:
    TestGamsprobeProcess();
    ~TestGamsprobeProcess();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_execute();

private:
    GamsprobeProcess *mProcess = nullptr;
};

TestGamsprobeProcess::TestGamsprobeProcess()
{
    CommonPaths::setSystemDir(GAMS_DISTRIB_PATH);
}

TestGamsprobeProcess::~TestGamsprobeProcess()
{

}

void TestGamsprobeProcess::initTestCase()
{
    mProcess = new GamsprobeProcess;
}

void TestGamsprobeProcess::cleanupTestCase()
{
    delete mProcess;
}

void TestGamsprobeProcess::test_execute()
{
    auto data = mProcess->execute();
    auto error = mProcess->errorMessage();
    QVERIFY(!data.isEmpty());
    QVERIFY(error.isEmpty());
}

QTEST_APPLESS_MAIN(TestGamsprobeProcess)

#include "tst_testgamsprobeprocess.moc"
