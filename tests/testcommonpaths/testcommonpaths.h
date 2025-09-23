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
#ifndef TESTCOMMONPATHS_H
#define TESTCOMMONPATHS_H

#include <QTest>

class TestCommonPaths : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testSystemDir();

    void testSetSystemDirNull();
    void testSetSystemDirEmpty();
    void testSetSystemDirCustom();
    void testSetSystemDirNoGAMS();
    void testSetSystemDirAPPIMAGE();

    void testGamsDocumentsDir();
    void testUserDocumentDir();
    void testUserModelLibraryDir();
    void testDefaultWorkingDir();
    void testGamsConnectSchemaDir();

    void testIsSystemDirValid();
    void testIsSystemDirInValid();

    void testAbsoluteFilePathEmpty();
    void testAbsoluteFilePathNullStr();
    void testAbsoluteFilePathExisting();
    void testAbsoluteFilePathNotExisting();
    void testAbsoluteFilePathFromRelativePath();

    void testAbsolutePathEmpty();
    void testAbsolutePathNullStr();
    void testAbsolutePathExisting();
    void testAbsolutePathNotExisting();

    void testConfigFile();
    void testLicenseFile();

    void testGamsLicenseFilePath_data();
    void testGamsLicenseFilePath();

private:
    QString getExpectedPath();
};

#endif // TESTCOMMONPATHS_H
