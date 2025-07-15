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
#include "testcheckforupdate.h"
#include "checkforupdate.h"
#include "commonpaths.h"

#include <QRegularExpression>

using namespace gams::studio;
using namespace gams::studio::support;

void TestCheckForUpdateWrapper::initTestCase()
{
    CommonPaths::setSystemDir(GAMS_DISTRIB_PATH);
    mC4U = new CheckForUpdate(false, this);
}

void TestCheckForUpdateWrapper::testConstructor()
{
    CheckForUpdate c4u(true);
    QSignalSpy spy(&c4u, &CheckForUpdate::versionInformationAvailable);
    spy.wait(3000);

    QCOMPARE(spy.count(), 1);
    QVERIFY(!c4u.versionInformation().isEmpty());
    if (c4u.localDistribVersion() < c4u.remoteDistribVersion())
        QVERIFY(!c4u.versionInformationShort().isEmpty());
    else
        QVERIFY(c4u.versionInformationShort().isEmpty());
    QVERIFY(c4u.localDistribVersion() > 0);
    QVERIFY(!c4u.localDistribVersionShort().isEmpty());
    QVERIFY(c4u.remoteDistribVersion() > 0);
    QVERIFY(c4u.localStudioVersion() > 0);
}

void TestCheckForUpdateWrapper::testCheckForUpdate()
{
    CheckForUpdate c4u(true);
    QSignalSpy spy(&c4u, &CheckForUpdate::versionInformationAvailable);
    spy.wait(3000);

    QCOMPARE(spy.count(), 1);
    QVERIFY(!c4u.versionInformation().isEmpty());
    if (c4u.localDistribVersion() < c4u.remoteDistribVersion())
        QVERIFY(!c4u.versionInformationShort().isEmpty());
    else
        QVERIFY(c4u.versionInformationShort().isEmpty());
    QVERIFY(c4u.localDistribVersion() > 0);
    QVERIFY(!c4u.localDistribVersionShort().isEmpty());
    QVERIFY(c4u.remoteDistribVersion() > 0);
    QVERIFY(c4u.localStudioVersion() > 0);

    c4u.checkForUpdate(true);
    spy.wait(3000);

    QCOMPARE(spy.count(), 2);
    QVERIFY(!c4u.versionInformation().isEmpty());
    if (c4u.localDistribVersion() < c4u.remoteDistribVersion())
        QVERIFY(!c4u.versionInformationShort().isEmpty());
    else
        QVERIFY(c4u.versionInformationShort().isEmpty());
    QVERIFY(c4u.localDistribVersion() > 0);
    QVERIFY(!c4u.localDistribVersionShort().isEmpty());
    QVERIFY(c4u.remoteDistribVersion() > 0);
    QVERIFY(c4u.localStudioVersion() > 0);
}

void TestCheckForUpdateWrapper::testLocalDistribVersion()
{
    QVERIFY(mC4U->localDistribVersion() > 0);
}

void TestCheckForUpdateWrapper::testLocalDistribVersionShort()
{
    QString result = mC4U->localDistribVersionShort();
    static QRegularExpression regexp("^\\d+\\.\\d$");
    QVERIFY(regexp.match(result).hasMatch());
}

void TestCheckForUpdateWrapper::testRemoteDistribVersion()
{
    QVERIFY(mC4U->remoteDistribVersion()>0);
}

void TestCheckForUpdateWrapper::testIsLocalDistribLatest()
{
    if (mC4U->localDistribVersion() == mC4U->remoteDistribVersion())
        QVERIFY(mC4U->isLocalDistribLatest());
    else
        QVERIFY(!mC4U->isLocalDistribLatest());
}

void TestCheckForUpdateWrapper::testLocalStudioVersion()
{
    QString expected = QString(STUDIO_VERSION_STRING).replace(".", "");
    QString result = QString::number(mC4U->localStudioVersion());
    QVERIFY(expected.contains(result));
}

void TestCheckForUpdateWrapper::testVersionInformationShort()
{
    static QRegularExpression regexp("^.*\\d+\\.\\d.\\d.*$");
    auto text = mC4U->versionInformationShort();
    if (!text.isEmpty())
        QVERIFY(regexp.match(text).hasMatch());
}

QTEST_GUILESS_MAIN(TestCheckForUpdateWrapper)
