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
#ifndef TESTMIROCOMMON_H
#define TESTMIROCOMMON_H

#include <QtTest/QTest>

class TestMiroCommon : public QObject
{
    Q_OBJECT
public:
    TestMiroCommon(QObject *parent = nullptr);
    ~TestMiroCommon();

private slots:
    void testPath_data();
    void testPath();

    void testConfDirectory_data();
    void testConfDirectory();

    void testDataDirectory_data();
    void testDataDirectory();

    void testDataContractFileName_data();
    void testDataContractFileName();

    void testAssemblyFileName_data();
    void testAssemblyFileName();

    void testAssemblyFileName2_data();
    void testAssemblyFileName2();

    void testDeployFileName_data();
    void testDeployFileName();

    void testUnifiedAssemblyFileContent_data();
    void testUnifiedAssemblyFileContent();

    void testWriteAssemblyFile_data();
    void testWriteAssemblyFile();

private:
    QDir mCurDir;
    QFile mFile1;
    QFile mFile2;
    QFile mUnifiedAssemblyFileContentFile;
    QFile mUnifiedAssemblyFileContentFileData;

};

#endif // TESTMIROCOMMON_H
