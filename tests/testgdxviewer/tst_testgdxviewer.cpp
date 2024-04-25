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
    void test_export();

private:
    void createGdxFile(QString model);

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

void TestGdxViewer::createGdxFile(QString model)
{
    QProcess *proc = new QProcess(this);
    proc->setProgram(QDir::toNativeSeparators(QDir(CommonPaths::systemDir()).absoluteFilePath("gamslib")));
    QStringList args { model, QDir::toNativeSeparators(CommonPaths::defaultWorkingDir(true)) };
    proc->setArguments(args);
    proc->start();
    proc->waitForFinished();

    proc->setProgram(QDir::toNativeSeparators(QDir(CommonPaths::systemDir()).absoluteFilePath("gams")));
    args = QStringList { model + ".gms", "gdx", model + ".gdx" };
    proc->setArguments(args);
    proc->setWorkingDirectory(QDir::toNativeSeparators(CommonPaths::defaultWorkingDir(true)));
    proc->start();
    proc->waitForFinished();
}

void TestGdxViewer::test_createGdxViewer()
{
    createGdxFile("trnsport");

    // create GdxViewer instance
    QString tmp = QDir::toNativeSeparators(QDir(CommonPaths::defaultWorkingDir(true)).absoluteFilePath("trnsport.gdx"));
    GdxViewer *gdxViewer = new GdxViewer(tmp, CommonPaths::systemDir(), QTextCodec::codecForName("utf-8"));
    delete gdxViewer;
    gdxViewer = nullptr;
}

void TestGdxViewer::test_export()
{
    createGdxFile("trnsport");

    // create GdxViewer instance
    QString tmp = QDir::toNativeSeparators(QDir(CommonPaths::defaultWorkingDir(true)).absoluteFilePath("trnsport.gdx"));
    GdxViewer *gdxViewer = new GdxViewer(tmp, CommonPaths::systemDir(), QTextCodec::codecForName("utf-8"));

    ExportModel *exportModel = new ExportModel(gdxViewer->gdxSymbolTable(), this);
    exportModel->selectAll();

    ExportDriver *exportDriver = new ExportDriver(gdxViewer, exportModel, this);
    exportDriver->saveAndExecute(QDir::toNativeSeparators(QDir(CommonPaths::defaultWorkingDir(true)).absoluteFilePath("trnsport_export.yaml")),
                                 QDir::toNativeSeparators(QDir(CommonPaths::defaultWorkingDir(true)).absoluteFilePath("trnsport_export.xlsx")),
                                 QDir::toNativeSeparators(CommonPaths::defaultWorkingDir(true)),
                                 true,
                                 "EPS",
                                 "INF",
                                 "-INF",
                                 "UNDEF",
                                 "NA");

    // wait 10s for the ExportDriver to finish
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(exportDriver, &ExportDriver::exportDone, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(20000);
    loop.exec();

    if(!timer.isActive())
        QVERIFY2(false, "Timeout in test_export");

    delete exportDriver;
    exportDriver = nullptr;
    delete exportModel;
    exportModel = nullptr;
    delete gdxViewer;
    gdxViewer = nullptr;
}

QTEST_MAIN(TestGdxViewer)

#include "tst_testgdxviewer.moc"
