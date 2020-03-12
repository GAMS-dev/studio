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
    _VersionSettings,
    _VersionStudio,

    // window settings
    _winSize,
    _winPos,
    _winState,
    _winMaximized,

    // view menu settings
    _viewProject,
    _viewOutput,
    _viewHelp,
    _viewOption,

    // general system settings
    _encodingMib,
    _projects,
    _tabs,
    _history,

    // settings of help page
    _hBookmarks,
    _hZoomFactor,

    // search widget
    _searchUseRegex,
    _searchCaseSens,
    _searchWholeWords,
    _searchScope,

    // general settings page
    _defaultWorkspace,
    _skipWelcomePage,
    _restoreTabs,
    _autosaveOnRun,
    _openLst,
    _jumpToError,
    _foregroundOnDemand,
    _historySize,

    // editor settings page
    _edFontFamily,
    _edFontSize,
    _edShowLineNr,
    _edTabSize,
    _edLineWrapEditor,
    _edLineWrapProcess,
    _edClearLog,
    _edWordUnderCursor,
    _edHighlightCurrentLine,
    _edAutoIndent,
    _edWriteLog,
    _edLogBackupCount,
    _edAutoCloseBraces,
    _edEditableMaxSizeMB,

    // MIRO settings page
    _miroInstallPath,

    // solver option editor settings
    _soOverrideExisting,
    _soAddCommentAbove,
    _soAddEOLComment,
    _soDeleteCommentsAbove,

    // user model library directory
    _userModelLibraryDir,
};

class Settings
{
public:
    enum Kind {KAll, KUi, KSys, KUser};

public:
    static void createSettings(bool ignore, bool reset, bool resetView);
    static Settings *settings();
    static void releaseSettings();

    void load(Kind kind);
    void save();

    bool toBool(SettingsKey key) const { return value(key).toBool(); }
    int toInt(SettingsKey key) const { return value(key).toInt(); }
    double toDouble(SettingsKey key) const { return value(key).toDouble(); }
    QSize toSize(SettingsKey key) const;
    QPoint toPoint(SettingsKey key) const;
    QString toString(SettingsKey key) const { return value(key).toString(); }
    QByteArray toByteArray(SettingsKey key) const { return value(key).toByteArray(); }
    QJsonObject toJsonObject(SettingsKey key) const;
    QJsonArray toJsonArray(SettingsKey key) const;
    void setBool(SettingsKey key, bool value) { setValue(key, value);}
    void setInt(SettingsKey key, int value) { setValue(key, value);}
    void setDouble(SettingsKey key, double value) { setValue(key, value);}
    void setSize(SettingsKey key, const QSize &value);
    void setPoint(SettingsKey key, const QPoint &value);
    void setString(SettingsKey key, QString value) { setValue(key, value);}
    void setByteArray(SettingsKey key, QByteArray value) { setValue(key, value);}
    bool setJsonObject(SettingsKey key, QJsonObject value);
    bool setJsonArray(SettingsKey key, QJsonArray value);

    void exportSettings(const QString &settingsPath);
    void importSettings(const QString &settingsPath);

    void updateSettingsFiles();

    void reload();
    void resetViewSettings();

    QStringList fileHistory();

private:
    typedef QMap<QString, QVariant> Data;
    struct KeyData {
        KeyData() {}
        KeyData(Settings::Kind _kind, QStringList _keys, QVariant _initial)
            : kind(_kind), keys(_keys), initial(_initial) {}
        Settings::Kind kind = KAll;
        QStringList keys;
        QVariant initial;
    };

    static Settings *mInstance;
    static const int mVersion;
    bool mCanWrite = false;
    bool mCanRead = false;

    QHash<SettingsKey, KeyData> mKeys;
    QMap<Kind, QSettings*> mSettings;
    QMap<Kind, Data> mData;


private:
    Settings(bool ignore, bool reset, bool resetView);
    ~Settings();
    void initKeys();
    KeyData keyData(SettingsKey key) { return mKeys.value(key); }

    int checkVersion();
    QString settingsPath();
    bool createSettingFiles();
    void reset(Kind kind);
    void initData(Kind kind);
    void saveFile(Kind kind);
    QVariant read(SettingsKey key, Kind kind = KAll);

    void initSettingsFiles(int version);

    QVariant value(SettingsKey key) const;
    bool setValue(SettingsKey key, QVariant value);

    bool isValidVersion(QString currentVersion);
    int compareVersion(QString currentVersion, QString otherVersion);
};

}
}

#endif // SETTINGS_H
