#include <QtTest>
#include <commonpaths.h>

// add necessary includes here
#include <settings.h>
#include <gdxviewer/gdxviewer.h>

using namespace gams::studio;
using namespace gams::studio::gdxviewer;

class TestGdxViewer : public QObject
{
    Q_OBJECT

public:
    TestGdxViewer();
    ~TestGdxViewer();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_createGdxViewer();

};

TestGdxViewer::TestGdxViewer()
{

}

TestGdxViewer::~TestGdxViewer()
{

}

void TestGdxViewer::initTestCase()
{
    CommonPaths::setSystemDir();
    Settings::createSettings(true, false, false);
}

void TestGdxViewer::cleanupTestCase()
{

}

void TestGdxViewer::test_createGdxViewer()
{
    // create GDX file
    QProcess *proc = new QProcess(this);
    proc->setProgram(QDir::toNativeSeparators(QDir(CommonPaths::systemDir()).absoluteFilePath("gamslib")));
    QStringList args { "trnsport", QDir::toNativeSeparators(CommonPaths::defaultWorkingDir()) };
    proc->setArguments(args);
    proc->start();
    proc->waitForFinished();

    proc->setProgram(QDir::toNativeSeparators(QDir(CommonPaths::systemDir()).absoluteFilePath("gams")));
    args = QStringList { "trnsport.gms",  "gdx=trnsport.gdx" };
    proc->setArguments(args);
    proc->setWorkingDirectory(QDir::toNativeSeparators(CommonPaths::defaultWorkingDir()));
    proc->start();
    proc->waitForFinished();

    // create GdxViewer instance
    int codecMib = Settings::settings()->toInt(skDefaultCodecMib);
    QTextCodec *mCodec = QTextCodec::codecForMib(codecMib);
    QString tmp = QDir::toNativeSeparators(QDir(CommonPaths::defaultWorkingDir()).absoluteFilePath("trnsport.gdx"));
    GdxViewer *gdxViewer = new GdxViewer(tmp, CommonPaths::systemDir(), mCodec);
    delete gdxViewer;
    gdxViewer = nullptr;
}

QTEST_MAIN(TestGdxViewer)

#include "tst_testgdxviewer.moc"
