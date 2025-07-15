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
#include "testsettings.h"
#include "settings.h"
#include "logger.h"
#include "editors/abstractsystemlogger.h"

using namespace gams::studio;

static const int mSleepMs = 1500;

void TestSettings::initTestCase()
{
    Settings::useRelocatedPathForTests();
}

void createVersion1Settings()
{
    QDir d; d.mkpath("./GAMS");

    // --------- create uistates.json in version 1 -----

    QFile f1("./GAMS/gams.json");
    if (f1.open(QFile::WriteOnly)) {
        f1.write(
R"({
    "version": {
        "settings": 1,
        "studio": "0.14.3.0"
    },
    "viewMenu": {
        "help": true,
        "optionEdit": false,
        "output": true,
        "project": true
    },
    "window": {
        "maximized": false,
        "pos": "0,0",
        "size": "1024,768",
        "state": ""
    }
})");
        f1.close();

    }

    // --------- create gams1.json in version 1 -----

    QFile f2("./GAMS/gams1.json");
    if (f2.open(QFile::WriteOnly)) {
        f2.write(
R"({
    "encodings": "UTF-8,System,ISO-8859-1,Shift_JIS,GB2312",
    "help": {
        "bookmarks": [
        ],
        "zoom": 1
    },
    "history": [
    ],
    "projects": {
    },
    "search": {
        "caseSens": false,
        "regex": false,
        "scope": 0,
        "wholeWords": false
    },
    "tabs": {
    },
    "userModelLibraryDir": "./GAMS/modellibs",
    "version": {
        "settings": 1,
        "studio": "0.14.3.0"
    }
})");
        f2.close();
    }

    // --------- create usersettings1.json in version 1 -----

    QFile f3("./GAMS/usersettings1.json");
    if (f3.open(QFile::WriteOnly)) {
        f3.write(
R"({
    "autosaveOnRun": true,
    "defaultWorkspace": "./GAMS/Studio/workspace",
    "editor": {
        "TabSize": 4,
        "autoCloseBraces": true,
        "autoIndent": true,
        "clearLog": false,
        "editableMaxSizeMB": 50,
        "fontFamily": "Consolas",
        "fontSize": 10,
        "highlightCurrentLine": false,
        "lineWrapEditor": false,
        "lineWrapProcess": false,
        "logBackupCount": 3,
        "showLineNr": true,
        "wordUnderCursor": false,
        "writeLog": true
    },
    "foregroundOnDemand": true,
    "historySize": 12,
    "jumpToErrors": true,
    "miro": {
        "installationLocation": ""
    },
    "openLst": false,
    "restoreTabs": true,
    "skipWelcome": false,
    "solverOption": {
        "addCommentAbove": false,
        "addEOLComment": false,
        "deleteCommentsAbove": false,
        "overrideExisting": true
    },
    "version": {
        "settings": 1,
        "studio": "0.14.3.0"
    }
})");
        f3.close();
    }

    // base-level key renamed: jumpToErrors -> jumpToError

    // group-level key renamed: miro/installationLocation -> miro/installationPath
}

void removeSettingFiles() {
    QFile f1("./GAMS/studio.json");
    if (f1.exists()) {
        Q_ASSERT(f1.remove());
    }
    QFile f2("./GAMS/usersettings.json");
    if (f2.exists()) {
        Q_ASSERT(f2.remove());
    }
}

void TestSettings::testChangeValueAtRoot()
{
    removeSettingFiles();
    Settings::createSettings(false, true, false);
    // change value, save and release settings
    bool value = Settings::settings()->toBool(skSkipWelcomePage);
    Settings::settings()->setBool(skSkipWelcomePage, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(skSkipWelcomePage));
    Settings::settings()->save();
    Settings::releaseSettings();

    // create and read settings, compare value
    Settings::createSettings(false, false, false);
    // check stored value
    Q_ASSERT(value != Settings::settings()->toBool(skSkipWelcomePage));
    Settings::releaseSettings();
}

void TestSettings::testChangeValueInGroup()
{
    removeSettingFiles();
    Settings::createSettings(false, true, false);
    // change value, save and release settings
    bool value = Settings::settings()->toBool(skViewHelp);
    Settings::settings()->setBool(skViewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(skViewHelp));
    Settings::settings()->save();
    Settings::releaseSettings();

    // create and read settings, compare value
    Settings::createSettings(false, false, false);
    // check stored value
    Q_ASSERT(value != Settings::settings()->toBool(skViewHelp));
    Settings::releaseSettings();
}

void TestSettings::testReadSettingsIgnore()
{
    removeSettingFiles();
    Settings::createSettings(false, true, false);
    // change value, save and release settings
    bool value = Settings::settings()->toBool(skViewHelp);
    Settings::settings()->setBool(skViewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(skViewHelp));
    Settings::settings()->save();
    Settings::releaseSettings();

    // create settings with "ignore" flag, compare value
    Settings::createSettings(true, false, false);
    // check stored value
    Q_ASSERT(value == Settings::settings()->toBool(skViewHelp));
    Settings::releaseSettings();
}

void TestSettings::testReadSettingsIgnoreReset()
{
    removeSettingFiles();
    Settings::createSettings(false, true, false);
    // change value, save and release settings
    bool value = Settings::settings()->toBool(skViewHelp);
    Settings::settings()->setBool(skViewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(skViewHelp));
    Settings::settings()->save();
    Settings::releaseSettings();

    // create settings with "ignore,reset" flag, compare value
    Settings::createSettings(true, true, false);
    // check stored value
    Q_ASSERT(value == Settings::settings()->toBool(skViewHelp));
    Settings::releaseSettings();
}

void TestSettings::testReadSettingsReset()
{
    removeSettingFiles();
    Settings::createSettings(false, true, false);
    // change value, save and release settings
    bool value = Settings::settings()->toBool(skViewHelp);
    Settings::settings()->setBool(skViewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(skViewHelp));
    Settings::settings()->save();
    Settings::releaseSettings();

    // create settings with "reset" flag, compare value
    Settings::createSettings(false, true, false);
    // check stored value
    Q_ASSERT(value == Settings::settings()->toBool(skViewHelp));
    Settings::releaseSettings();
}

void TestSettings::testWriteSettingsIgnore()
{
    removeSettingFiles();

    // open settings with ignore flag
    Settings::createSettings(true, false, false);

    // change value, save and release settings
    bool value = Settings::settings()->toBool(skViewHelp);
    Settings::settings()->setBool(skViewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(skViewHelp));
    Settings::settings()->save(); // With "ignore" flag this shouldn't write the file
    Settings::releaseSettings();

    // create common settings to compare value
    Settings::createSettings(false, false, false);
    // check stored value
    Q_ASSERT(value == Settings::settings()->toBool(skViewHelp));
    Settings::releaseSettings();
}

void TestSettings::testWriteSettingsIgnoreReset()
{
    removeSettingFiles();

    Settings::createSettings(true, true, false);
    // change value, save and release settings
    bool value = Settings::settings()->toBool(skViewHelp);
    Settings::settings()->setBool(skViewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(skViewHelp));
    Settings::settings()->save(); // With "ignore,reset" flags this shouldn't write the file

    Settings::releaseSettings();

    // create common settings to compare value
    Settings::createSettings(false, false, false);
    // check stored value
    Q_ASSERT(value == Settings::settings()->toBool(skViewHelp));
    Settings::releaseSettings();
}

void TestSettings::testWriteSettingsReset()
{
    removeSettingFiles();

    Settings::createSettings(false, true, false);

    // change value, save and release settings
    bool value = Settings::settings()->toBool(skViewHelp);
    Settings::settings()->setBool(skViewHelp, !value);
    // check in-memory value
    Q_ASSERT(value != Settings::settings()->toBool(skViewHelp));
    Settings::settings()->save(); // With "reset" flag this should write the file
    Settings::releaseSettings();
    QTest::qWait(mSleepMs);
    // create common settings to compare value
    Settings::createSettings(false, false, false);
    // check stored value
    Q_ASSERT(value != Settings::settings()->toBool(skViewHelp));
    Settings::releaseSettings();
}

void TestSettings::testIgnoreIfNoFilesExist()
{
    removeSettingFiles();
    QFile file("./GAMS/gams.json");
    // try creating settings files with ignore flag
    Settings::createSettings(true, false, false);
    Settings::settings()->save();
    Settings::releaseSettings();
    Q_ASSERT(!file.exists());
}

QTEST_MAIN(TestSettings)
