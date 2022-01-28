/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
    skVersionSysSettings,
    skVersionUserSettings,
    skVersionGamsStudio,

    // window settings
    skWinSize,
    skWinPos,
    skWinState,
    skWinMaxSizes,
    skWinNormSizes,
    skWinMaximized,
    skWinFullScreen,

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

    // general settings page
    skDefaultWorkspace,
    skSkipWelcomePage,
    skRestoreTabs,
    skAutosaveOnRun,
    skOpenLst,
    skJumpToError,
    skForegroundOnDemand,
    skHistorySize,
    skOpenInCurrent,

    // editor settings page
    skEdAppearance,
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
    skEdCompleterAutoOpen,
    skEdCompleterCasing,
    skEdFoldedDcoOnOpen,
    skEdSmartTooltipHelp,

    // MIRO settings page
    skMiroInstallPath,

    // misc page
    skNeosAutoConfirm,
    skNeosAcceptTerms,
    skNeosForceGdx,
    skNeosShortPrio,
    skUserGamsTypes,
    skAutoReloadTypes,

    // solver option editor settings
    skSoOverrideExisting,
    skSoAddCommentAbove,
    skSoAddEOLComment,
    skSoDeleteCommentsAbove,

    // GAMS Engine settings
    skEngineUrl,
    skEngineUser,
    skEngineUserToken,
    skEngineStoreUserToken,
    skEngineAuthExpire,
    skEngineIsSelfCert,
    skEngineNamespace,
    skEngineUserInstance,
    skEngineForceGdx,

    // user model library directory
    skUserModelLibraryDir,

    // syntax colors
    skUserThemes,

    skSettingsKeyCount,
};

class Settings
{
public:
    enum Scope {scSys,      // System setting (shared with all versions)
                scSysX,     // System setting (for current version)
                scUser,     // User settings (shared with all versions)
                scUserX,    // User settings (for current version)
                scTheme,    // Color theme settings (shared with all versions)
               };

    struct ScopePair {
        ScopePair(Scope _base, Scope _versionized) : base(_base), versionized(_versionized) {}
        Scope base;
        Scope versionized;
    };

public:
    static void createSettings(bool ignore, bool reset, bool resetView);
    static Settings *settings();
    static void releaseSettings();

    static int version(Scope scope);
    static void useRelocatedPathForTests();
    static QList<SettingsKey> viewKeys();
    static QString themeFileName(QString baseName);

    void loadFile(Scope scopeFilter);
    void save();
    void block() { mBlock = true; }
    void unblock() { mBlock = false; }

    bool toBool(SettingsKey key) const { return value(key).toBool(); }
    int toInt(SettingsKey key) const { return value(key).toInt(); }
    double toDouble(SettingsKey key) const { return value(key).toDouble(); }
    QSize toSize(SettingsKey key) const;
    QPoint toPoint(SettingsKey key) const;
    QList<int> toIntList(SettingsKey key) const;
    QString toString(SettingsKey key) const { return value(key).toString(); }
    QByteArray toByteArray(SettingsKey key) const;
    QVariantMap toMap(SettingsKey key) const;
    QVariantList toList(SettingsKey key) const;
    void setBool(SettingsKey key, bool value) { setValue(key, value);}
    void setInt(SettingsKey key, int value) { setValue(key, value);}
    void setDouble(SettingsKey key, double value) { setValue(key, value);}
    void setSize(SettingsKey key, const QSize &value);
    void setPoint(SettingsKey key, const QPoint &value);
    void setIntList(SettingsKey key, const QList<int> &value);
    void setString(SettingsKey key, QString value) { setValue(key, value);}
    void setByteArray(SettingsKey key, QByteArray value) { setValue(key, value);}
    bool setMap(SettingsKey key, QVariantMap value);
    bool setList(SettingsKey key, QVariantList value);

    void importSettings(const QString &path);
    void exportSettings(const QString &path);

    void exportTheme(const QVariant &vTheme, QString fileName);
    QVariantMap importTheme(const QString &filepath);

    void updateSettingsFiles();

    void reload();
    void resetKeys(QList<SettingsKey> keys);

private:
    typedef QVariantMap Data;
    struct KeyData {
        KeyData() {}
        KeyData(Settings::Scope _scope, QStringList _keys, QVariant _initial)
            : scope(_scope), keys(_keys), initial(_initial) {}
        Settings::Scope scope;
        QStringList keys;
        QVariant initial;
    };

    static Settings *mInstance;
    static const QHash<Scope, int> mVersion;
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
    bool safelyAdd(QHash<SettingsKey, KeyData> &hash, SettingsKey key, Scope scope, QStringList jsKey, QVariant defaultValue);
    KeyData keyData(SettingsKey key) { return mKeys.value(key); }
    bool canWrite() {return mCanWrite && !mBlock; }

    int usableVersion(ScopePair scopes);
    void loadVersionData(ScopePair scopes);
    QString settingsPath();
    void initDefault();
    void addVersionInfo(Scope scope, QVariantMap &map);
    void saveFile(Scope scope);
    void loadMap(Scope scope, QVariantMap map);
    QVariant read(SettingsKey key);
    void saveThemes();
    void loadThemes();

    QVariant value(SettingsKey key) const;
    bool setValue(SettingsKey key, QVariant value);

    QVariant directValue(const Scope &scope, const QString &key) const;
    QVariant directValue(const Scope &scope, const QString &group, const QString &key) const;
    bool setDirectValue(const Scope &scope, const QString &key, QVariant value);
    bool setDirectValue(const Scope &scope, const QString &group, const QString &key, QVariant value);
    bool addToMap(QVariantMap &group, const QString &key, QVariant value);
    QString keyText(SettingsKey key);

    bool isValidVersion(QString currentVersion);
    int compareVersion(QString currentVersion, QString otherVersion);
};

}
}

#endif // SETTINGS_H
