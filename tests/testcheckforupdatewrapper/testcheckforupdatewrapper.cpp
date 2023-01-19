/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include "testcheckforupdatewrapper.h"
#include "checkforupdatewrapper.h"
#include "commonpaths.h"

using namespace gams::studio;
using namespace gams::studio::support;

void TestCheckForUpdateWrapper::initTestCase()
{
    CommonPaths::setSystemDir(GAMS_DISTRIB_PATH);
}

void TestCheckForUpdateWrapper::testCheckForUpdateWrapper()
{
    CheckForUpdateWrapper c4uWrapper;
}

void TestCheckForUpdateWrapper::testIsValid()
{
    CheckForUpdateWrapper c4uWrapper;
    QVERIFY(c4uWrapper.isValid());
}

void TestCheckForUpdateWrapper::testMessage()
{
    CheckForUpdateWrapper c4uWrapper;
    QVERIFY(c4uWrapper.message().isEmpty());
}

void TestCheckForUpdateWrapper::testClearMessage()
{
    CheckForUpdateWrapper c4uWrapper;
    c4uWrapper.clearMessages();
    QVERIFY(c4uWrapper.message().isEmpty());
}

void TestCheckForUpdateWrapper::testCheckForUpdate()
{
    CheckForUpdateWrapper c4uWrapper;
    QVERIFY(!c4uWrapper.checkForUpdate().isEmpty());
}

void TestCheckForUpdateWrapper::testcheckForUpdateShort()
{
    CheckForUpdateWrapper c4uWrapper;
    if (c4uWrapper.isValid()) {
        if (c4uWrapper.usingLatestGams())
            QVERIFY(c4uWrapper.checkForUpdateShort().isEmpty());
        else
            QVERIFY(!c4uWrapper.checkForUpdateShort().isEmpty());
    } else {
        QVERIFY(c4uWrapper.checkForUpdateShort().isEmpty());
    }
}

void TestCheckForUpdateWrapper::testCurrentDistribVersion()
{
    CheckForUpdateWrapper c4uWrapper;
    QVERIFY(c4uWrapper.currentDistribVersion() > 0);
}

void TestCheckForUpdateWrapper::testCurrentDistribVersionShort()
{
    CheckForUpdateWrapper c4uWrapper;
    QString result = c4uWrapper.currentDistribVersionShort();
    QRegExp regexp("^\\d+\\.\\d$");
    QVERIFY(regexp.exactMatch(result));
}

void TestCheckForUpdateWrapper::testLastDistribVersion()
{
    CheckForUpdateWrapper c4uWrapper;
    QVERIFY(c4uWrapper.lastDistribVersion()>0);
}

void TestCheckForUpdateWrapper::testLastDistribVersionShort()
{
    CheckForUpdateWrapper c4uWrapper;
    c4uWrapper.checkForUpdate(); // call to fill str below
    QString result = c4uWrapper.lastDistribVersionShort();
    qDebug() << result;
    QRegExp regexp("^\\d+\\.\\d$");
    QVERIFY(regexp.exactMatch(result));
}

void TestCheckForUpdateWrapper::testDistribIsLast()
{
    CheckForUpdateWrapper c4uWrapper;
    if (c4uWrapper.currentDistribVersion() == c4uWrapper.lastDistribVersion())
        QVERIFY(c4uWrapper.distribIsLatest());
    else
        QVERIFY(!c4uWrapper.distribIsLatest());
}

void TestCheckForUpdateWrapper::testStudioVersion()
{
    CheckForUpdateWrapper c4uWrapper;
    QString expected = QString(STUDIO_VERSION).replace(".", "");
    QString result = QString::number(c4uWrapper.studioVersion());
    QVERIFY(expected.contains(result));
}

void TestCheckForUpdateWrapper::testDistribVersionString()
{
    auto version = CheckForUpdateWrapper::distribVersionString();
    QRegExp regexp("^\\d+\\.\\d\\.\\d\\s?[\\w\\W]*");
    QVERIFY(regexp.exactMatch(version));
}

QTEST_MAIN(TestCheckForUpdateWrapper)
