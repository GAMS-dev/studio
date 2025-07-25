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
#ifndef TESTGAMSOPTION_H
#define TESTGAMSOPTION_H

#include <QtTest/QTest>

#include "help/helpdata.h"

using namespace gams::studio;

class TestDocLocation : public QObject
{
    Q_OBJECT

private slots:
    void testSolverAnchor_data();
    void testSolverAnchor();

    void testUrlLocalFile();

    void testLocalFileToOnlineUrl_data();
    void testLocalFileToOnlineUrl();

    void testOnlineUrlToLocalFile_data();
    void testOnlineUrlToLocalFile();
};

#endif // TESTGAMSOPTION_H
