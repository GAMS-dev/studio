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
#include "testsettings.h"
#include "settings.h"
#include "logger.h"

using namespace gams::studio;

static const int mSleepMs = 1500;

void TestSettings::initTestCase()
{
    Settings::useRelocatedPathForTests();
}

void removeSettingFiles() {
    QFile f1("./GAMS/uistates.json");
    if (f1.exists()) {
        Q_ASSERT(f1.remove());
    }
    QFile f2("./GAMS/systemsettings1.json");
    if (f2.exists()) {
        Q_ASSERT(f2.remove());
    }
    QFile f3("./GAMS/usersettings1.json");
    if (f3.exists()) {
        Q_ASSERT(f3.remove());
    }
}

void TestSettings::testChangeValueAtRoot()
{
    removeSettingFiles();
    Settings::createSettings(false, true, false);
    // change value, save and release settings
    bool value = Settings::settings()->toBool(_skipWelcomePage);
    Settings::settings()->setBool(_skipWelcomePage, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(_skipWelcomePage));
    Settings::settings()->save();
    Settings::releaseSettings();

    // create and read settings, compare value
    Settings::createSettings(false, false, false);
    // check stored value
    Q_ASSERT(value != Settings::settings()->toBool(_skipWelcomePage));
    Settings::releaseSettings();
}

void TestSettings::testChangeValueInGroup()
{
    removeSettingFiles();
    Settings::createSettings(false, true, false);
    // change value, save and release settings
    bool value = Settings::settings()->toBool(_viewHelp);
    Settings::settings()->setBool(_viewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(_viewHelp));
    Settings::settings()->save();
    Settings::releaseSettings();

    // create and read settings, compare value
    Settings::createSettings(false, false, false);
    // check stored value
    Q_ASSERT(value != Settings::settings()->toBool(_viewHelp));
    Settings::releaseSettings();
}

void TestSettings::testReadSettingsIgnore()
{
    removeSettingFiles();
    Settings::createSettings(false, true, false);
    // change value, save and release settings
    bool value = Settings::settings()->toBool(_viewHelp);
    Settings::settings()->setBool(_viewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(_viewHelp));
    Settings::settings()->save();
    Settings::releaseSettings();

    // create settings with "ignore" flag, compare value
    Settings::createSettings(true, false, false);
    // check stored value
    Q_ASSERT(value == Settings::settings()->toBool(_viewHelp));
    Settings::releaseSettings();
}

void TestSettings::testReadSettingsIgnoreReset()
{
    removeSettingFiles();
    Settings::createSettings(false, true, false);
    // change value, save and release settings
    bool value = Settings::settings()->toBool(_viewHelp);
    Settings::settings()->setBool(_viewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(_viewHelp));
    Settings::settings()->save();
    Settings::releaseSettings();

    // create settings with "ignore,reset" flag, compare value
    Settings::createSettings(true, true, false);
    // check stored value
    Q_ASSERT(value == Settings::settings()->toBool(_viewHelp));
    Settings::releaseSettings();
}

void TestSettings::testReadSettingsReset()
{
    removeSettingFiles();
    Settings::createSettings(false, true, false);
    // change value, save and release settings
    bool value = Settings::settings()->toBool(_viewHelp);
    Settings::settings()->setBool(_viewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(_viewHelp));
    Settings::settings()->save();
    Settings::releaseSettings();

    // create settings with "reset" flag, compare value
    Settings::createSettings(false, true, false);
    // check stored value
    Q_ASSERT(value == Settings::settings()->toBool(_viewHelp));
    Settings::releaseSettings();
}

void TestSettings::testWriteSettingsIgnore()
{
    removeSettingFiles();

    // open settings with ignore flag
    Settings::createSettings(true, false, false);

    // change value, save and release settings
    bool value = Settings::settings()->toBool(_viewHelp);
    Settings::settings()->setBool(_viewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(_viewHelp));
    Settings::settings()->save(); // With "ignore" flag this shouldn't write the file
    Settings::releaseSettings();

    // create common settings to compare value
    Settings::createSettings(false, false, false);
    // check stored value
    Q_ASSERT(value == Settings::settings()->toBool(_viewHelp));
    Settings::releaseSettings();
}

void TestSettings::testWriteSettingsIgnoreReset()
{
    removeSettingFiles();

    Settings::createSettings(true, true, false);
    // change value, save and release settings
    bool value = Settings::settings()->toBool(_viewHelp);
    Settings::settings()->setBool(_viewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(_viewHelp));
    Settings::settings()->save(); // With "ignore,reset" flags this shouldn't write the file

    Settings::releaseSettings();

    // create common settings to compare value
    Settings::createSettings(false, false, false);
    // check stored value
    Q_ASSERT(value == Settings::settings()->toBool(_viewHelp));
    Settings::releaseSettings();
}

void TestSettings::testWriteSettingsReset()
{
    removeSettingFiles();

    Settings::createSettings(false, true, false);

    // change value, save and release settings
    bool value = Settings::settings()->toBool(_viewHelp);
    Settings::settings()->setBool(_viewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(_viewHelp));
    Settings::settings()->save(); // With "reset" flag this should write the file
    Settings::releaseSettings();
    QTest::qSleep(mSleepMs);
    // create common settings to compare value
    Settings::createSettings(false, false, false);
    // check stored value
    Q_ASSERT(value != Settings::settings()->toBool(_viewHelp));
    Settings::releaseSettings();
}

void TestSettings::testIgnoreIfNoFilesExist()
{
    removeSettingFiles();
    QFile file("./GAMS/uistates.json");
    // try creating settings files with ignore flag
    Settings::createSettings(true, false, false);
    Settings::settings()->save();
    Settings::releaseSettings();
    Q_ASSERT(!file.exists());
}

QTEST_MAIN(TestSettings)
