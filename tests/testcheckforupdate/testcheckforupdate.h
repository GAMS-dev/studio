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
#ifndef TESTCHECKFORUPDATE_H
#define TESTCHECKFORUPDATE_H

#include <QTest>
#include <QSignalSpy>

namespace gams {
namespace studio {
namespace support {
class CheckForUpdate;
}
}
}

class TestCheckForUpdateWrapper : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testConstructor();
    void testCheckForUpdate();

    void testLocalDistribVersion();
    void testLocalDistribVersionShort();

    void testRemoteDistribVersion();

    void testIsLocalDistribLatest();

    void testLocalStudioVersion();

    void testVersionInformationShort();

private:
    gams::studio::support::CheckForUpdate* mC4U;
};

#endif // TESTCHECKFORUPDATE_H
