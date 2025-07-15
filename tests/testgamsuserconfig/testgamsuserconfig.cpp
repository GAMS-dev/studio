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
#include "exception.h"
#include "file/filetype.h"
#include "common.h"
#include "commonpaths.h"
#include "option/gamsuserconfig.h"
#include "testgamsuserconfig.h"

#include <QStandardPaths>
#include <QProcess>
#include <QRegularExpression>

using gams::studio::CommonPaths;
using gams::studio::option::GamsUserConfig;
using gams::studio::FileType;
using gams::studio::FileKind;

void TestGamsUserConfig::initTestCase()
{
    systemDir = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    QVERIFY2(!systemDir.isEmpty(), "SystemDir must not be empty");
    qDebug() << QString("systemDir=[%1]").arg(CommonPaths::systemDir());
}

void TestGamsUserConfig::testUserConfigDir()
{
    qDebug() << QString("gamsUserConfigDir=[%1]").arg(CommonPaths::gamsUserConfigDir());
}

void TestGamsUserConfig::testFileType()
{
    // given
    QString testFile = CommonPaths::defaultGamsUserConfigFile();
    // when, then
    QCOMPARE( &FileType::from(testFile), &FileType::from(FileKind::Guc));

    // given
    QFileInfo fi1(CommonPaths::gamsUserConfigDir(), "notAgamsconfig.yaml");
    // when, then
    QCOMPARE( &FileType::from(fi1.fileName()), &FileType::from(FileKind::GCon));

}

void TestGamsUserConfig::testReadEmptyDefaultGamsConfigFile()
{
    // given
    CommonPaths::setSystemDir(systemDir);

    QString testFile = CommonPaths::defaultGamsUserConfigFile();
    qDebug() << QString("gamsUserConfigDir=[%1]").arg(CommonPaths::gamsUserConfigDir());
    qDebug() << QString("defaultGamsConfigFilepath=[%1]").arg(testFile);

    QFile file(testFile);
    if (file.exists())  file.remove();
    if (file.open(QFile::WriteOnly))
        file.close();

    // when
    GamsUserConfig* guc(new GamsUserConfig(testFile));

    // then
    QVERIFY(guc->isAvailable());
    QVERIFY(guc->readCommandLineParameters().size()==0);
    QVERIFY(guc->readEnvironmentVariables().size()==0);

    // cleanup
    if (guc) delete  guc;
    if (file.exists())  file.remove();
}

void TestGamsUserConfig::testReadDefaultGamsConfigFile()
{
    // given
    CommonPaths::setSystemDir(systemDir);

    QString testFile = CommonPaths::defaultGamsUserConfigFile();
    QFile file(testFile);
    if (file.exists())  file.remove();

    if (!file.open(QFile::WriteOnly | QFile::Text))
        QFAIL(QString("expected to open [%1] to write, but failed").arg(testFile).toLatin1());

    QTextStream out(&file);
    out << "---" << Qt::endl;
    out << "commandLineParameters:" << Qt::endl;
    out << "- pySetup:"             << Qt::endl;
    out << "    value: 0"           << Qt::endl;
    out << "    minVersion: 23"     << Qt::endl;
    out << "    maxVersion: 30.2"   << Qt::endl;
    out << "- action:"              << Qt::endl;
    out << "    value: ce"          << Qt::endl;
    out << "    maxVersion: 30.2"   << Qt::endl;
    out << "- CurDir:"              << Qt::endl;
    out << "    value: x"           << Qt::endl;
    out << "- CNS:"                 << Qt::endl;
    out << "    value: y"           << Qt::endl;
    out << "environmentVariables:"  << Qt::endl;
    out << "- PYTHON38:"            << Qt::endl;
    out << "    value: blah"        << Qt::endl;
    out << "..."                    << Qt::endl;
    file.close();

    // when
    GamsUserConfig* guc(new GamsUserConfig(testFile));

    // then
    QVERIFY(guc->isAvailable());
    QVERIFY(guc->readCommandLineParameters().size()==4);
    QCOMPARE(guc->readCommandLineParameters().constFirst()->key, "pySetup");
    QCOMPARE(guc->readCommandLineParameters().constFirst()->value, "0");
    QCOMPARE(guc->readCommandLineParameters().constFirst()->minVersion, "23");
    QCOMPARE(guc->readCommandLineParameters().constFirst()->maxVersion, "30.2");
    QCOMPARE(guc->readCommandLineParameters().constLast()->key, "CNS");
    QCOMPARE(guc->readCommandLineParameters().constLast()->value, "y");
    QVERIFY(guc->readEnvironmentVariables().size()==1);
    QCOMPARE(guc->readEnvironmentVariables().constFirst()->key, "PYTHON38");
    QCOMPARE(guc->readEnvironmentVariables().constFirst()->value, "blah");

    // cleanup
    if (guc) delete  guc;
    if (file.exists())  file.remove();
}

void TestGamsUserConfig::testGamsRunningDefaultConfigFile()
{
    // given
    CommonPaths::setSystemDir(systemDir);
    QDir gamsSysDir(CommonPaths::systemDir());
    QString tempDir = QDir::currentPath();
    QDir gamsPath = QDir(gamsSysDir).filePath("gams");
    QDir gamslibPath = QDir(gamsSysDir).filePath("gamslib");
    QDir gmsOutputPath = QDir(tempDir).filePath("trnsport.gms");
    QDir gdxOutputPath = QDir(tempDir).filePath("mygdxfromconfig.gdx");

    QString testFile = CommonPaths::defaultGamsUserConfigFile();
    qDebug() << QString("gamsUserConfigDir=[%1]").arg(CommonPaths::gamsUserConfigDir());
    qDebug() << QString("defaultGamsConfigFilepath=[%1]").arg(testFile);
    qDebug() << "running test in QDir::currentPath()=" << tempDir;

    QFile gmsOutputFile(gmsOutputPath.path());
    if (gmsOutputFile.exists())  gmsOutputFile.remove();
    QFile gdxOutputFile(gdxOutputPath.path());
    if (gdxOutputFile.exists())  gdxOutputFile.remove();
    QFile gamsConfigFile(testFile);
    if (gamsConfigFile.exists())  gamsConfigFile.remove();

    if (!gamsConfigFile.open(QFile::WriteOnly | QFile::Text))
        QFAIL(QString("expected to open [%1] to write, but failed").arg(testFile).toLatin1());

    QTextStream out(&gamsConfigFile);
    out << "---"                            << Qt::endl;
    out << "commandLineParameters:"         << Qt::endl;
    out << "- GDX:"                         << Qt::endl;
    out << "    value: mygdxfromconfig.gdx" << Qt::endl;
    out << "..."                            << Qt::endl;
    gamsConfigFile.close();

    // when
    QProcess* gamsLibProc = new QProcess(this);
    gamsLibProc->setWorkingDirectory(tempDir);
    gamsLibProc->setArguments( {"1"} ); // get trnsport.gms
    gamsLibProc->setProgram(gamslibPath.path());
    gamsLibProc->start();
    if (gamsLibProc->waitForFinished()) {
        QCOMPARE( gamsLibProc->exitCode(), 0);
        QVERIFY(gmsOutputFile.exists());
        QVERIFY(gmsOutputFile.size() > 0);

        QProcess* gamsProc = new QProcess(this);
        gamsProc->setWorkingDirectory(tempDir);
        gamsProc->setArguments( {"trnsport.gms"} );
        gamsProc->setProgram(gamsPath.path());
        gamsProc->start();
        // then
        if (gamsProc->waitForFinished()) {
            QDir lstPath = QDir(tempDir).filePath("trnsport.lst");
            QFile lstFile(lstPath.path());
            QVERIFY(lstFile.exists());
            qDebug() << "workdir=" << tempDir;
            qDebug() << "lstFile=" << QFileInfo(lstFile).absoluteFilePath();
            if (gamsProc->exitCode() != 0) {
                if (lstFile.open(QIODevice::ReadOnly)) {
                    QTextStream in(&lstFile);
                    int line = 0;
                    while (!in.atEnd())
                        qDebug() << "line " << ++line << ":"  << in.readLine();
                    lstFile.close();
                }
            }
            QCOMPARE( gamsProc->exitCode(), 0);
            QVERIFY(gdxOutputFile.exists());
            QVERIFY(gdxOutputFile.size() > 0);
        } else {
            QFAIL(QString("expected to run [%1] successfully, but failed").arg(gamsProc->program()).toLatin1());
        }
    } else {
        QFAIL(QString("expected to run [%1] successfully, but failed").arg(gamsLibProc->program()).toLatin1());
    }
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

QRegularExpression TestGamsUserConfig::mRexVersion("^[1-9][0-9](\\.([0-9])(\\.([0-9]))?)?$");

void TestGamsUserConfig::testVersionFormat()
{
    QFETCH(QString, version);
    QFETCH(bool, valid);

    QCOMPARE( mRexVersion.match(version).hasMatch() , valid);
}

QTEST_MAIN(TestGamsUserConfig)
