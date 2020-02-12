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

using namespace gams::studio::miro;

void TestMiroCommon::testPath()
{
    QCOMPARE(MiroCommon::path(QString()), QString());
    QCOMPARE(MiroCommon::path("notexisting"), QString());
}

void TestMiroCommon::testPathWithRealFile()
{
    QFile file("mymiro");
    if (file.open(QIODevice::WriteOnly))
        file.close();
    QFileInfo fi(file);
    QCOMPARE(MiroCommon::path(fi.path()), fi.path());
}

void TestMiroCommon::testPathWithSpaces()
{
    QFile file("my miro");
    if (file.open(QIODevice::WriteOnly))
        file.close();
    QFileInfo fi(file);
    QCOMPARE(MiroCommon::path(fi.path()), fi.path());
}

void TestMiroCommon::testAssemblyFileName()
{
    QCOMPARE(MiroCommon::assemblyFileName(QString()), "_files.txt");
    QCOMPARE(MiroCommon::assemblyFileName("model"), "model_files.txt");
}

void TestMiroCommon::testDeployFileName()
{
    QCOMPARE(MiroCommon::deployFileName(QString()), QString());
    QCOMPARE(MiroCommon::deployFileName("mymodel"), "mymodel.miroapp");
    QCOMPARE(MiroCommon::deployFileName("mymodel.out"), "mymodel.out.miroapp");
}

QTEST_MAIN(TestMiroCommon)
