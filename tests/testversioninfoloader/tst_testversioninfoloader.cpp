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
#include <QtTest>

#include "versioninfoloader.h"

using namespace gams::studio::support;

class TestVersionInfoLoader : public QObject
{
    Q_OBJECT

public:
    TestVersionInfoLoader();
    ~TestVersionInfoLoader();

private slots:
    void test_default_constructor();
    void test_requestRemoteData();
    void test_get_set();
};

TestVersionInfoLoader::TestVersionInfoLoader()
{

}

TestVersionInfoLoader::~TestVersionInfoLoader()
{

}

void TestVersionInfoLoader::test_default_constructor()
{
    VersionInfoLoader loader;
    QCOMPARE(loader.remoteDistribVersion(), 0);
    QCOMPARE(loader.remoteDistribVersionString(), QString());
    QCOMPARE(loader.remoteStudioVersion(), 0);
    QCOMPARE(loader.remoteStudioVersionString(), QString());
    QCOMPARE(loader.errorString(), QString());
    QVERIFY(loader.distribVersions().isEmpty());
}

void TestVersionInfoLoader::test_requestRemoteData()
{
    VersionInfoLoader loader;
    QSignalSpy spy(&loader, &VersionInfoLoader::finished);
    loader.requestRemoteData();
    spy.wait(6000);
    QCOMPARE(spy.count(), 1);
    QVERIFY(loader.remoteDistribVersion() > 0);
    QVERIFY(loader.remoteDistribVersionString() != QString());
    QVERIFY(loader.remoteStudioVersion() > 0);
    QVERIFY(loader.remoteStudioVersionString() != QString());
    QCOMPARE(loader.errorString(), QString());
    QVERIFY(!loader.distribVersions().isEmpty());
}

void TestVersionInfoLoader::test_get_set()
{
    VersionInfoLoader loader;
    loader.setRemoteDistribVersion(42);
    QCOMPARE(loader.remoteDistribVersion(), 42);
    loader.setRemoteDistribVersionString("42.0.0 Alpha");
    QCOMPARE(loader.remoteDistribVersionString(), "42.0.0 Alpha");
    loader.setRemoteStudioVersion(16);
    QCOMPARE(loader.remoteStudioVersion(), 16);
    loader.setRemoteStudioVersionString("1.16.0");
    QCOMPARE(loader.remoteStudioVersionString(), "1.16.0");
    QCOMPARE(loader.errorString(), QString());
    QVERIFY(loader.distribVersions().isEmpty());
}

QTEST_GUILESS_MAIN(TestVersionInfoLoader)

#include "tst_testversioninfoloader.moc"
