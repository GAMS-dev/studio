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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QHash>
#include <QSettings>

class QSettings;
class QFile;

namespace gams {
namespace studio {

class MainWindow;

enum SettingsKey {
    // REMARK: version is treated differently as it is passed to ALL versionized setting files
    skVersionSettings,
    skVersionStudio,

    // window settings
    skWinSize,
    skWinPos,
    skWinState,
    skWinMaximized,

    // view menu settings
    skViewProject,
    skViewOutput,
    skViewHelp,
    skViewOption,

    // general system settings
    skDefaultCodecMib,
    skEncodingMib,
    skProjects,
    skTabs,
    skHistory,

    // settings of help page
    skHelpBookmarks,
    skHelpZoomFactor,

    // search widget
    skSearchUseRegex,
    skSearchCaseSens,
    skSearchWholeWords,
    skSearchScope,

    // general settings page
    skDefaultWorkspace,
    skSkipWelcomePage,
    skRestoreTabs,
    skAutosaveOnRun,
    skOpenLst,
    skJumpToError,
    skForegroundOnDemand,
    skHistorySize,

    // editor settings page
    skEdColorSchemeIndex,
    skEdFontFamily,
    skEdFontSize,
    skEdShowLineNr,
    skEdTabSize,
    skEdLineWrapEditor,
    skEdLineWrapProcess,
    skEdClearLog,
    skEdWordUnderCursor,
    skEdHighlightCurrentLine,
    skEdAutoIndent,
    skEdWriteLog,
    skEdLogBackupCount,
    skEdAutoCloseBraces,
    skEdEditableMaxSizeMB,

    // MIRO settings page
    skMiroInstallPath,

    // solver option editor settings
    skSoOverrideExisting,
    skSoAddCommentAbove,
    skSoAddEOLComment,
    skSoDeleteCommentsAbove,

    // user model library directory
    skUserModelLibraryDir,
};

class Settings
{
public:
    enum Scope {scAll, scUi, scSys, scUser};

public:
    static void createSettings(bool ignore, bool reset, bool resetView);
    static Settings *settings();
    static void releaseSettings();
    static int version();
    static void useRelocatedPathForTests();

    void load(Scope kind);
    void save();
    void block() { mBlock = true; }
    void unblock() { mBlock = false; }

    bool toBool(SettingsKey key) const { return value(key).toBool(); }
    int toInt(SettingsKey key) const { return value(key).toInt(); }
    double toDouble(SettingsKey key) const { return value(key).toDouble(); }
    QSize toSize(SettingsKey key) const;
    QPoint toPoint(SettingsKey key) const;
    QString toString(SettingsKey key) const { return value(key).toString(); }
    QByteArray toByteArray(SettingsKey key) const;
    QVariantMap toVariantMap(SettingsKey key) const;
    QVariantList toList(SettingsKey key) const;
    void setBool(SettingsKey key, bool value) { setValue(key, value);}
    void setInt(SettingsKey key, int value) { setValue(key, value);}
    void setDouble(SettingsKey key, double value) { setValue(key, value);}
    void setSize(SettingsKey key, const QSize &value);
    void setPoint(SettingsKey key, const QPoint &value);
    void setString(SettingsKey key, QString value) { setValue(key, value);}
    void setByteArray(SettingsKey key, QByteArray value) { setValue(key, value);}
    bool setMap(SettingsKey key, QVariantMap value);
    bool setList(SettingsKey key, QVariantList value);

    void exportSettings(const QString &settingsPath);
    void importSettings(const QString &settingsPath);

    void updateSettingsFiles();

    void reload();
    void resetViewSettings();

private:
    typedef QMap<QString, QVariant> Data;
    struct KeyData {
        KeyData() {}
        KeyData(Settings::Scope _scope, QStringList _keys, QVariant _initial)
            : scope(_scope), keys(_keys), initial(_initial) {}
        Settings::Scope scope = scAll;
        QStringList keys;
        QVariant initial;
    };

    static Settings *mInstance;
    static const int mVersion;
    static bool mUseRelocatedTestDir;
    bool mCanWrite = false;
    bool mCanRead = false;
    bool mBlock = false;

    const QHash<SettingsKey, KeyData> mKeys;
    QMap<Scope, QSettings*> mSettings;
    QMap<Scope, Data> mData;

private:
    Settings(bool ignore, bool reset, bool resetView);
    ~Settings();
    QSettings *newQSettings(QString name);
    QHash<SettingsKey, KeyData> generateKeys();
    KeyData keyData(SettingsKey key) { return mKeys.value(key); }
    bool canWrite() {return mCanWrite && !mBlock; }

    int checkVersion();
    QString settingsPath();
    bool createSettingFiles();
    void reset(Scope scope);
    void initDefault(Scope scope);
    void saveFile(Scope scope);
    QVariant read(SettingsKey key, Scope scope = scAll);

    void initSettingsFiles(int version);

    QVariant value(SettingsKey key) const;
    bool setValue(SettingsKey key, QVariant value);

    QVariant directValue(const Scope &scope, const QString &key) const;
    QVariant directValue(const Scope &scope, const QString &group, const QString &key) const;
    bool setDirectValue(const Scope &scope, const QString &key, QVariant value);
    bool setDirectValue(const Scope &scope, const QString &group, const QString &key, QVariant value);
    bool addToMap(QVariantMap &group, const QString &key, QVariant value);

    bool isValidVersion(QString currentVersion);
    int compareVersion(QString currentVersion, QString otherVersion);
};

}
}

#endif // SETTINGS_H
