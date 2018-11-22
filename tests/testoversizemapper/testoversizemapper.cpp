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
#include "testoversizemapper.h"
#include "editors/textmapper.h"
#include "logger.h"

#include <QtGlobal>
#include <QStandardPaths>

using gams::studio::OversizeMapper;

void TestOversizeMapper::testBlockSize1Limit()
{
    OversizeMapper om;
    qint64 maxInt = std::numeric_limits<int>::max();

    om.setSize(maxInt-1);
    QCOMPARE(om.bytesInBlock, 1);
    QCOMPARE(om.map(0,0), 0);
    QCOMPARE(om.map(1,0), 1);

    om.setSize(maxInt);
    QCOMPARE(om.bytesInBlock, 1);
    QCOMPARE(om.map(0,0), 0);
    QCOMPARE(om.map(1,0), 1);

    om.setSize(maxInt+1);
    QCOMPARE(om.bytesInBlock, 2);
    QCOMPARE(om.map(0,0), 0);
    QCOMPARE(om.map(1,0), 2);
}

void TestOversizeMapper::testBlockSize2Limit()
{
    OversizeMapper om;
    qint64 maxInt = std::numeric_limits<int>::max();
    om.setSize((maxInt*2)-1);
    QCOMPARE(om.bytesInBlock, 2);
    QCOMPARE(om.map(0,0), 0);
    QCOMPARE(om.map(1,0), 2);

    om.setSize((maxInt*2));
    QCOMPARE(om.bytesInBlock, 2);
    QCOMPARE(om.map(0,0), 0);
    QCOMPARE(om.map(1,0), 2);

    om.setSize((maxInt*2)+1);
    QCOMPARE(om.bytesInBlock, 3);
    QCOMPARE(om.map(0,0), 0);
    QCOMPARE(om.map(1,0), 3);
}

QTEST_MAIN(TestOversizeMapper)
