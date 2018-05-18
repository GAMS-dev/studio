/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "testcommonpaths.h"
#include "commonpaths.h"

#include <QDebug>

using gams::studio::CommonPaths;

void TestCommonPaths::testAbsoluteFilePathEmpty()
{
    auto result = CommonPaths::absolutFilePath("");
    QVERIFY(result.isEmpty());
}

void TestCommonPaths::testAbsoluteFilePathNullStr()
{
    auto result = CommonPaths::absolutFilePath(QString());
    QVERIFY(result.isEmpty());
}

void TestCommonPaths::testAbsoluteFilePathExisting()
{
    const QString currentDir(CommonPaths::absolutPath("."));
    const QString filePath("testcommonpaths");
    auto result = CommonPaths::absolutFilePath(filePath);
    QVERIFY(!result.compare(currentDir+"/"+filePath));
}

void TestCommonPaths::testAbsoluteFilePathNotExisting()
{
    const QString currentDir(CommonPaths::absolutPath("."));
    const QString filePath("myDir/myfile.txt");
    auto result = CommonPaths::absolutFilePath(filePath);
    QVERIFY(!result.compare(currentDir+"/"+filePath));
}

void TestCommonPaths::testAbsolutePathEmpty()
{
    auto result = CommonPaths::absolutPath("");
    QVERIFY(result.isEmpty());
}

void TestCommonPaths::testAbsolutePathNullStr()
{
    auto result = CommonPaths::absolutPath(QString());
    QVERIFY(result.isEmpty());
}

void TestCommonPaths::testAbsolutePathExisting()
{
    const QString absolutPath(CommonPaths::absolutPath("."));
    auto result = CommonPaths::absolutPath(absolutPath);
    QVERIFY(!result.compare(absolutPath));
}

void TestCommonPaths::testAbsolutePathNotExisting()
{
    const QString absolutPath(CommonPaths::absolutPath("./lala"));
    auto result = CommonPaths::absolutPath(absolutPath);
    QVERIFY(!result.compare(absolutPath));
}

QTEST_MAIN(TestCommonPaths)
