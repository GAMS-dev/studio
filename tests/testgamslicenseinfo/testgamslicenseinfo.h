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
#ifndef TESTGAMSLICENSEINFO_H
#define TESTGAMSLICENSEINFO_H

#include <QTest>

class TestGamsLicenseInfo : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testGamsLicenseInfo();

    void testSolvers();

    void testSolverId();
    void testSolverIdLowerCase();
    void testSolverIdMixedCase();
    void testSolverIdInvalid();

    void testSolverName();
    void testSolverNameZeroIndex();
    void testSolverNameNegativeIndex();
    void testSolverNameOutOfRange();

    void testSolverNames();

    void testModelTypeNames();

    void testSolverCapability();
    void testSolverCapabilityInvalidSolver();
    void testSolverCapabilityInvalidModelType();
    void testSolverCapabilityInvalidSolverNegative();
    void testSolverCapabilityInvalidModelTypeNegative();
    void testSolverCapabilityBothInvalid();
    void testSolverCapabilityBothInvalidNegative();

    void testSolverLicense();

    void testIsLicenseValid();
    void testIsLicenseValidText();

    void testGamsDataLocations();
    void testGamsConfigLocations();

    void testLocalDistribVersion();
    void testLocalDistribVersionString();
    void testLocalDistribVersionStringShort();

    void testLicenseFromFile_simpleErrorCases();

    void testLicenseFromFile_licenseWithLines();

    void testLicenseFromFile_licenseContinuesLine();

    void testLicenseFromFile_randomSpaces();

    void testLicenseFromFile_BOM();

    void testLicenseDirectory();
    void testLicenseLocation();

private:
    QStringList testLicense();

    QString testLicenseWithSpaces();
};

#endif // TESTGAMSLICENSEINFO_H
