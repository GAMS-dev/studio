/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "testmirocommon.h"
#include "miro/mirocommon.h"

#include <algorithm>

using namespace std;
using namespace gams::studio::miro;

TestMiroCommon::TestMiroCommon(QObject *parent)
    : QObject(parent),
      mFile1("mymiro"),
      mFile2("my miro"),
      mUnifiedAssemblyFileContentFile("unified_files.txt"),
      mUnifiedAssemblyFileContentFileData("data_files.txt")
{
    if (mFile1.open(QIODevice::WriteOnly))
        mFile1.close();
    if (mFile2.open(QIODevice::WriteOnly))
        mFile2.close();
    if (mUnifiedAssemblyFileContentFile.open(QIODevice::WriteOnly))
        mUnifiedAssemblyFileContentFile.close();
    if (mUnifiedAssemblyFileContentFileData.open(QIODevice::WriteOnly)) {
        mUnifiedAssemblyFileContentFileData.write("data_files.txt\ndata_files.txt\ndata\ndata\nconf_files");
        mUnifiedAssemblyFileContentFileData.close();
    }
}

TestMiroCommon::~TestMiroCommon()
{
    mFile1.remove();
    mFile2.remove();
    mUnifiedAssemblyFileContentFile.remove();
}

void TestMiroCommon::testPath_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<QString>("result");

    QTest::newRow("empty") << QString() << QString();
    QTest::newRow("notexisting") << "notexisting" << QString();

    QFileInfo fi1(mFile1);
    QTest::newRow("real file") << fi1.path() << fi1.path();

    QFileInfo fi2(mFile2);
    QTest::newRow("spaces in path") << fi2.path() << fi2.path();
}

void TestMiroCommon::testPath()
{
    QFETCH(QString, data);
    QFETCH(QString, result);
    QCOMPARE(MiroCommon::path(data), result);
}

void TestMiroCommon::testAssemblyFileName_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<QString>("result");

    QTest::addRow("empty") << QString() << "_files.txt";
    QTest::addRow("model name") << "model" << "model_files.txt";
}

void TestMiroCommon::testAssemblyFileName()
{
    QFETCH(QString, data);
    QFETCH(QString, result);
    QCOMPARE(MiroCommon::assemblyFileName(data), result);
}

void TestMiroCommon::testAssemblyFileName2_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("location");
    QTest::addColumn<QString>("result");

    QTest::addRow("empty") << QString() << mCurDir.absolutePath() << QString();
    QTest::addRow("model name") << "model" << mCurDir.absolutePath() << "model_files.txt";
}

void TestMiroCommon::testAssemblyFileName2()
{
    QFETCH(QString, name);
    QFETCH(QString, location);
    QFETCH(QString, result);
    QCOMPARE(MiroCommon::assemblyFileName(location, name),
             mCurDir.absoluteFilePath(MiroCommon::assemblyFileName(name)));
}

void TestMiroCommon::testDeployFileName_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<QString>("result");

    QTest::addRow("empty") << QString() << QString();
    QTest::addRow("model name") << "mymodel" << "mymodel.miroapp";
    QTest::addRow("model with ending") << "mymodel.out" << "mymodel.out.miroapp";
}

void TestMiroCommon::testDeployFileName()
{
    QFETCH(QString, data);
    QFETCH(QString, result);
    QCOMPARE(MiroCommon::deployFileName(data), result);
}

void TestMiroCommon::testUnifiedAssemblyFileContent_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("mainGms");
    QTest::addColumn<QStringList>("result");

    QTest::addRow("empty") << QString() << QString() << QStringList();

    QTest::addRow("no fileName") << QString() << "model.gms"
                                 << QStringList { "model.gms" };

    QTest::addRow("notexisting") << "notexisting_files.txt" << "model.gms"
                                 << QStringList { "model.gms" };

    QTest::addRow("notexisting2") << "notexisting_files.txt" << QString()
                                  << QStringList();

    QTest::addRow("empty file") << mUnifiedAssemblyFileContentFile.fileName()
                                << "model.gms"
                                << QStringList();

    QTest::addRow("data file") << mUnifiedAssemblyFileContentFileData.fileName()
                               << "model.gms"
                               << QStringList { "data_files.txt", "data", "conf_files" };
}

void TestMiroCommon::testUnifiedAssemblyFileContent()
{
    QFETCH(QString, fileName);
    QFETCH(QString, mainGms);
    QFETCH(QStringList, result);

    auto content = MiroCommon::unifiedAssemblyFileContent(fileName, mainGms);
    sort(content.begin(), content.end());
    sort(result.begin(), result.end());
    QCOMPARE(content, result);
}

void TestMiroCommon::testWriteAssemblyFile_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QStringList>("selectedFiles");
    QTest::addColumn<bool>("result");

    QTest::addRow("empty") << QString() << QStringList() << false;

    QTest::addRow("no fileName") << QString() << QStringList { "data_folder" }
                                 << false;

    QTest::addRow("empty list") << "model_files.txt" << QStringList() << true;

    QTest::addRow("write content") << "model_files.txt"
                                   << QStringList { "data_folder" } << true;

    QTest::addRow("content") << "model_files2.txt"
                             << QStringList { "data_folder", "model.gms" }
                             << true;
}

void TestMiroCommon::testWriteAssemblyFile()
{
    QFETCH(QString, fileName);
    QFETCH(QStringList, selectedFiles);
    QFETCH(bool, result);
    QCOMPARE(MiroCommon::writeAssemblyFile(fileName, selectedFiles), result);

    if (QString(QTest::currentDataTag()) == "content") {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QStringList data;
            for (auto item: QString(file.readAll()).split('\n', Qt::SkipEmptyParts))
                data << item.trimmed();
            sort(data.begin(), data.end());
            sort(selectedFiles.begin(), selectedFiles.end());
            QCOMPARE(selectedFiles, data);
            file.close();
        } else {
            qDebug() << "Could not open file: " << fileName;
        }
    }

    if (!fileName.isEmpty())
        QFile::remove(fileName);
}

QTEST_MAIN(TestMiroCommon)
