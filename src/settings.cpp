/**
 * GAMS Studio
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
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QSize>
#include <QPoint>
#include <QFontDatabase>
#include "gdxviewer/numericalformatcontroller.h"
#include "logger.h"
#include "settings.h"
#include "commonpaths.h"
#include "version.h"
#include "exception.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"

namespace gams {
namespace studio {

// Increase mVersion only on MAJOR changes (change/remove existing field or add to array-element)
const QHash<Settings::Scope, int> Settings::mVersion = {{Settings::scSysX, 1},
                                                        {Settings::scUserX, 2}};
Settings *Settings::mInstance = nullptr;
bool Settings::mUseRelocatedTestDir = false;
const QString CThemePrefix("usertheme_");
const QString CThemeSufix("json");

// ====== Some helper functions ======

QString pointToString(QPoint p) {
    return QString("%1,%2").arg(p.x()).arg(p.y());
}
QString sizeToString(QSize s) {
    return QString("%1,%2").arg(s.width()).arg(s.height());
}
QList<int> toIntArray(const QString &s) {
    QList<int> res;
    const QStringList list = s.split(',');
    res.reserve(list.size());
    for (const QString &v: list) res << v.toInt();
    return res;
}
QPoint stringToPoint(const QString &s) {
    QList<int> a = toIntArray(s);
    if (a.size() == 2) return QPoint(a.at(0), a.at(1));
    return QPoint();
}
QSize stringToSize(const QString& s) {
    QList<int> a = toIntArray(s);
    if (a.size() == 2) return QSize(a.at(0), a.at(1));
    return QSize();
}

QString findFixedFont()
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    if (QFontInfo(font).fixedPitch())
        return QFontInfo(font).family();

    QStringList list = QFontDatabase::families();
    for (int i = 0; i < list.size(); ++i) {
        if (QFontDatabase::isPrivateFamily(list.at(i)))
            continue;
        if (QFontDatabase::isFixedPitch(list.at(i))) {
            return list.at(i);
        }
    }
    DEB() << "No fixed font found on system. Using " << font.family();
    return font.family();
}

Settings::ScopePair scopePair(Settings::Scope scope)
{
    int iScope = scope;
    if (iScope >= Settings::scTheme) return Settings::ScopePair(scope, scope);
    if (iScope % 2) return Settings::ScopePair(static_cast<Settings::Scope>(scope-1), scope);
    return Settings::ScopePair(scope, static_cast<Settings::Scope>(scope+1));
}

// ====== reader and writer for JSON files ======

bool readJsonFile(QIODevice &device, QSettings::SettingsMap &map)
{
    QJsonParseError parseResult;
    QJsonDocument json = QJsonDocument::fromJson(device.readAll(), &parseResult);
    if (parseResult.error) {
        DEB() << "JSON parse error at " << parseResult.offset << ": " << parseResult.errorString();
        return false;
    }
    map = json.object().toVariantMap();
    return true;
}

bool writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map)
{
    device.write(QJsonDocument(QJsonObject::fromVariantMap(map)).toJson());
    return true;
}

// ====== Start of Settings methods ======

void Settings::createSettings(bool ignore, bool reset, bool resetView)
{
    if (mInstance) {
        DEB() << "Tried to create multiple settings, suppressed.";
    } else
        mInstance = new Settings(ignore, reset, resetView);
}

Settings *Settings::settings()
{
    return mInstance;
}

void Settings::releaseSettings()
{
    delete mInstance;
    mInstance = nullptr;
}

int Settings::version(Scope scope)
{
    return mVersion[scopePair(scope).versionized];
}

void Settings::useRelocatedPathForTests()
{
    if (mInstance) {
        DEB() << "Can't relocate after instance has been created";
    } else {
        mUseRelocatedTestDir = true;
    }
}

Settings::Settings(bool ignore, bool reset, bool resetView)
    : mCanWrite(!ignore),
      mCanRead(!ignore && !reset),
      mKeys(generateKeys())
{
    // initialize storage
    mData.insert(scSys, Data());
    mData.insert(scUserX, Data());
    mData.insert(scTheme, Data());

    // initialize json format and make it the default
    QSettings::Format jsonFormat = QSettings::registerFormat(CThemeSufix, readJsonFile, writeJsonFile);
    QSettings::setDefaultFormat(jsonFormat);
    if (mUseRelocatedTestDir)
        QSettings::setPath(jsonFormat, QSettings::UserScope, ".");

    initDefault();

    // QSettings only needed if we want to write
    if (mCanWrite || mCanRead) {
        // create basic non versionized application settings
        QSettings *settings = newQSettings("studio");
        DEB() << "Settings path: " << QDir::toNativeSeparators(settings->fileName());
        if (settings) {
            // only if the basic settings file has been created ...
            mSettings.insert(scSys, settings);
            loadFile(scSys);
            settings = newQSettings("usersettings");
            mSettings.insert(scUser, settings);
            loadFile(scUser);

            // each theme has one file
            loadFile(scTheme);

            QDir location(settingsPath());
            const QStringList list = location.entryList({"*.lock"});
            for (const QString &fileName: list) {
                QFile f(location.path() +  "/" + fileName);
                f.remove();
            }
        } else {
            mCanWrite = false;
            mCanRead = false;
            DEB() << "Could not create settings files, switched to --ignore-settings";
        }
    }
    if (resetView) {
        resetKeys(viewKeys());
    }
}

Settings::~Settings()
{
    QMap<Scope, QSettings*>::iterator iSettings = mSettings.begin();
    while (iSettings != mSettings.end()) {
        QSettings *set = iSettings.value();
        set->sync();
        iSettings = mSettings.erase(iSettings);
        delete set;
        ++iSettings;
    }
}

QSettings *Settings::newQSettings(const QString& name)
{
    QSettings *res = nullptr;
    res = new QSettings(QSettings::defaultFormat(), QSettings::UserScope, GAMS_ORGANIZATION_STR, name);
    return res;
}

void Settings::checkSettings()
{
    QMap<Scope, QSettings*>::iterator iSettings = mSettings.begin();
    while (iSettings != mSettings.end()) {
        QSettings *settings = iSettings.value();
        settings->sync();
        if (settings->status()) {
            if (settings->status() == QSettings::FormatError)
                mInitWarnings << QString("Format error in settings file %1").arg(settings->fileName());
        }
        if (!QFile::exists(settings->fileName()))
            mInitWarnings << QString("Can't create settings file %1").arg(settings->fileName());
        else if (!settings->isWritable())
            mInitWarnings << QString("Can't write settings file %1").arg(settings->fileName());
        ++iSettings;
    }
}

QString settingsKeyName(SettingsKey key) {
    QString res = '[' + QString::number(key) + ']';
    return res;
}

bool Settings::safelyAdd(QHash<SettingsKey, Settings::KeyData> &hash, SettingsKey key, Scope scope, const QStringList &jsKey, const QVariant &defaultValue)
{
    bool res = true;
    if (hash.contains(key)) {
        res = false;
        DEB() << "WARNING: redefinition of SettingsKey" << settingsKeyName(key);
    }
    QHash<SettingsKey, Settings::KeyData>::const_iterator it = hash.constBegin();
    while (it != hash.constEnd()) {
        const QStringList &jsKey2 = it.value().keys;
        if (jsKey.size() == jsKey2.size()) {
            bool equal = true;
            for (int i = 0; i < jsKey.length(); ++i) {
                if (jsKey.at(i) != jsKey2.at(i)) equal = false;
            }
            if (equal) {
                res = false;
                DEB() << "WARNING: SettingsKey" << settingsKeyName(key) << " reuses json-key " << jsKey.join("/")
                      << " - previously used by SettingsKey" << settingsKeyName(it.key());
            }
        }

        ++it;
    }
    hash.insert(key, KeyData(scope, jsKey, defaultValue));
    return res;
}

QHash<SettingsKey, Settings::KeyData> Settings::generateKeys()
{
    QHash<SettingsKey, Settings::KeyData> res;

    safelyAdd(res, skVersionSysSettings, scSys, {"version","studiosettings"}, version(scSys));
    safelyAdd(res, skVersionGamsStudio, scSys, {"version","gamsstudio"}, QString(GAMS_VERSION_STR));
    safelyAdd(res, skVersionUserSettings, scUser, {"version","usersettings"}, version(scUser));

    // window settings
    safelyAdd(res, skWinSize, scSys, {"window","size"}, QString("1024,768"));
    safelyAdd(res, skWinPos, scSys, {"window","pos"}, QString("0,0"));
    safelyAdd(res, skWinState, scSys, {"window","state"}, QByteArray(""));
    safelyAdd(res, skWinMaxSizes, scSys, {"window","_maxSizes"}, QString("-1,-1,-1,-1"));
    safelyAdd(res, skWinNormSizes, scSys, {"window","_normSizes"}, QString("-1,-1,-1,-1"));
    safelyAdd(res, skWinMaximized, scSys, {"window","maximized"}, false);
    safelyAdd(res, skWinFullScreen, scSys, {"window","fullScreen"}, false);

    // view menu settings
    safelyAdd(res, skViewProject, scSys, {"viewMenu","project"}, true);
    safelyAdd(res, skViewOutput, scSys, {"viewMenu","output"}, true);
    safelyAdd(res, skViewHelp, scSys, {"viewMenu","help"}, false);
    safelyAdd(res, skViewOption, scSys, {"viewMenu","optionEdit"}, false);

    // general system settings
    safelyAdd(res, skDefaultCodecMib, scSys, {"defaultCodecMib"}, 106);
    safelyAdd(res, skDefaultEncoding, scSys, {"defaultEncoding"}, "UTF-8");
    safelyAdd(res, skEncodingMibs, scSys, {"encodingMIBs"}, QString("106,0,4,17,2025"));
    safelyAdd(res, skEncodings, scSys, {"encodings"}, "");
    safelyAdd(res, skProjects, scSys, {"projects"}, QJsonArray());
    safelyAdd(res, skCurrentFocusProject, scSys, {"currentFocusProject"}, -1);
    safelyAdd(res, skShowProjectFilter, scSys, {"showProjectFilter"}, true);
    safelyAdd(res, skProjectFilter, scSys, {"projectFilter"}, "");
    safelyAdd(res, skProjectSort, scSys, {"projectSort"}, 0);
    safelyAdd(res, skCurrentFileOpenFilter, scSys, {"currentFileOpenFilter"}, "");
    safelyAdd(res, skTabs, scSys, {"tabs"}, QJsonObject());
    safelyAdd(res, skBookmarks, scSys, {"fileBookmarks"}, QJsonArray());
    safelyAdd(res, skFoldedLines, scSys, {"foldedLines"}, QJsonArray());
    safelyAdd(res, skPinViewTabIndex, scSys, {"pinView", "tabIndex"}, -1);
    safelyAdd(res, skPinViewSize, scSys, {"pinView", "size"}, QString("10,10"));
    safelyAdd(res, skPinOrientation, scSys, {"pinView", "orientation"}, 1);
    safelyAdd(res, skPinScollLock, scSys, {"pinView", "scrollLock"}, false);
    safelyAdd(res, skHistory, scSys, {"history"}, QJsonArray());
    safelyAdd(res, skLastRun, scSys, {"lastRun"}, 0);
    safelyAdd(res, skRunOptions, scSys, {"runOptions"}, 3);
    safelyAdd(res, skSupressWebEngine, scSys, {"supressWebEngine"}, false);

    // user model library directory
    safelyAdd(res, skUserModelLibraryDir, scUser, {"userModelLibraryDir"}, CommonPaths::userModelLibraryDir());
//    safelyAdd(res, skUserModelLibraryHistory, scUser, {"userModelLibraryHistory"}, QString());

    // settings of help page
    safelyAdd(res, skHelpBookmarks, scSys, {"help","bookmarks"}, QJsonArray());
    safelyAdd(res, skHelpZoomFactor, scSys, {"help","zoom"}, 1.0);

    // search widget
    safelyAdd(res, skSearchUseRegex, scSys, {"search", "regex"}, false);
    safelyAdd(res, skSearchCaseSens, scSys, {"search", "caseSens"}, false);
    safelyAdd(res, skSearchWholeWords, scSys, {"search", "wholeWords"}, false);

    // general settings page
    safelyAdd(res, skDefaultWorkspace, scUser, {"defaultWorkspace"}, "");
    safelyAdd(res, skSkipWelcomePage, scUser, {"skipWelcome"}, false);
    safelyAdd(res, skRestoreTabs, scUser, {"restoreTabs"}, true);
    safelyAdd(res, skAutosaveOnRun, scUser, {"autosaveOnRun"}, true);
    safelyAdd(res, skOpenLst, scUser, {"openLst"}, false);
    safelyAdd(res, skJumpToError, scUser, {"jumpToErrors"}, true);
    safelyAdd(res, skForegroundOnDemand, scUser, {"foregroundOnDemand"}, true);
    safelyAdd(res, skOpenInCurrent, scUser, {"openInCurrent"}, false);
    safelyAdd(res, skOptionsPerMainFile, scUser, {"optionsPerMainFile"}, true);
    safelyAdd(res, skDynamicMainFile, scUser, {"dynamicMainFile"}, false);
    safelyAdd(res, skHistorySize, scUser, {"historySize"}, 20);
    safelyAdd(res, skEnableLog, scUser, {"enableLog"}, true);

    // project settings page
    safelyAdd(res, skProSingleProject, scUser, {"project","singleProject"}, false);
    safelyAdd(res, skProGspByFileCount, scUser, {"project","GspByFileCount"}, 6);
    safelyAdd(res, skProGspNeedsMain, scUser, {"project","GspNeedsMain"}, true);

    // editor settings page
    safelyAdd(res, skEdAppearance, scUser, {"editor","appearance"}, 0);
    safelyAdd(res, skEdInitFont, scUser, {"editor","initFont"}, true);
    safelyAdd(res, skEdFontFamily, scUser, {"editor","fontFamily"}, "");
    safelyAdd(res, skEdFontSize, scUser, {"editor","fontSize"}, 8);
    safelyAdd(res, skEdShowLineNr, scUser, {"editor","showLineNr"}, true);
    safelyAdd(res, skEdTabSize, scUser, {"editor","TabSize"}, 4);
    safelyAdd(res, skEdHighlightBound, scUser, {"editor","HighlightBound"}, 1500);
    safelyAdd(res, skEdHighlightMaxLines, scUser, {"editor","HighlightMaxLines"}, 10000);
    safelyAdd(res, skEdLineWrapEditor, scUser, {"editor","lineWrapEditor"}, false);
    safelyAdd(res, skEdLineWrapProcess, scUser, {"editor","lineWrapProcess"}, false);
    safelyAdd(res, skEdClearLog, scUser, {"editor","clearLog"}, false);
    safelyAdd(res, skEdProfilerCols, scUser, {"editor","ProfilerCols"}, 7);
    safelyAdd(res, skEdProfilerPrevCols, scUser, {"editor","ProfilerPrevCols"}, 7);
    safelyAdd(res, skEdWordUnderCursor, scUser, {"editor","wordUnderCursor"}, false);
    safelyAdd(res, skEdHighlightCurrentLine, scUser, {"editor","highlightCurrentLine"}, false);
    safelyAdd(res, skEdAutoIndent, scUser, {"editor","autoIndent"}, true);
    safelyAdd(res, skEdWriteLog, scUser, {"editor","writeLog"}, true);
    safelyAdd(res, skEdLogBackupCount, scUser, {"editor","logBackupCount"}, 0);
    safelyAdd(res, skEdAutoCloseBraces, scUser, {"editor","autoCloseBraces"}, true);
    safelyAdd(res, skEdEditableMaxSizeMB, scUser, {"editor","editableMaxSizeMB"}, 50);
    safelyAdd(res, skEdCompleterAutoOpen, scUser, {"editor","completerAutoShow"}, true);
    safelyAdd(res, skEdCompleterCasing, scUser, {"editor","completerCasing"}, 0);
    safelyAdd(res, skEdFoldedDcoOnOpen, scUser, {"editor","foldedDcoOnOpen"}, false);
    safelyAdd(res, skEdSmartTooltipHelp, scUser, {"editor","smartTooltipHelp"}, true);

    //GDX Viewer
    safelyAdd(res, skGdxDefaultSymbolView, scUser, {"gdxViewer","gdxDefaultSymbolView"}, 0);
    safelyAdd(res, skGdxStates, scSys, {"gdxViewer","states"}, QJsonObject());
    safelyAdd(res, skGdxDecSepCopy, scUser, {"gdxViewer","gdxDecSepCopy"}, 0);
    safelyAdd(res, skGdxCustomDecSepCopy, scUser, {"gdxViewer","gdxCustomDecSepCopy"}, "");
    safelyAdd(res, skGdxDefaultShowLevel, scUser, {"gdxViewer","gdxDefaultShowLevel"}, true);
    safelyAdd(res, skGdxDefaultShowMarginal, scUser, {"gdxViewer","gdxDefaultShowMarginal"}, true);
    safelyAdd(res, skGdxDefaultShowLower, scUser, {"gdxViewer","gdxDefaultShowLower"}, true);
    safelyAdd(res, skGdxDefaultShowUpper, scUser, {"gdxViewer","gdxDefaultShowUpper"}, true);
    safelyAdd(res, skGdxDefaultShowScale, scUser, {"gdxViewer","gdxDefaultShowScale"}, true);
    safelyAdd(res, skGdxDefaultSqueezeDefaults, scUser, {"gdxViewer","gdxDefaultSqueezeDefaults"}, false);
    safelyAdd(res, skGdxDefaultSqueezeZeroes, scUser, {"gdxViewer","gdxDefaultSqueezeZeroes"}, true);
    safelyAdd(res, skGdxDefaultFormat, scUser, {"gdxViewer","gdxDefaultFormat"}, 0);
    safelyAdd(res, skGdxDefaultPrecision, scUser, {"gdxViewer","gdxDefaultPrecision"}, 6);
    safelyAdd(res, skGdxDefaultRestoreSqueezeZeroes, scUser, {"gdxViewer","gdxDefaultRestoreSqueezeZeroes"}, false);

    // MIRO settings page
    safelyAdd(res, skMiroInstallPath, scUser, {"miro","installationLocation"}, QString());

    // misc page
    safelyAdd(res, skNeosAutoConfirm, scUser, {"neos","autoConfirm"}, false);
    safelyAdd(res, skNeosAcceptTerms, scUser, {"neos","acceptTerms"}, false);
    safelyAdd(res, skNeosForceGdx, scSys, {"neos","forceGdx"}, true);
    safelyAdd(res, skNeosShortPrio, scUser, {"neos","priotity"}, true);
    safelyAdd(res, skUserGamsTypes, scUser, {"misc","userFileTypes"}, QString());
    safelyAdd(res, skAutoReloadTypes, scUser, {"misc","autoReloadTypes"}, true);
    safelyAdd(res, skSwitchDefaultWorkDir, scUser, {"misc","switchDefaultWorkDir"}, false);
    safelyAdd(res, skCleanUpWorkspace, scUser, {"misc","cleanUpWorkspace"}, false);
    safelyAdd(res, skCleanUpWorkspaceFilter, scUser, {"misc","cleanUpWorkspaceFilter"}, QVariantMap());
    safelyAdd(res, skCleanUpWorkspaceDirectories, scUser, {"misc","cleanUpWorkspaceDirectories"}, QVariantMap());

    // solver option editor settings
    safelyAdd(res, skSoOverrideExisting, scUser, {"solverOption","overrideExisting"}, true);
    safelyAdd(res, skSoAddCommentAbove, scUser, {"solverOption","addCommentAbove"}, false);
    safelyAdd(res, skSoAddEOLComment, scUser, {"solverOption","addEOLComment"}, false);
    safelyAdd(res, skSoDeleteCommentsAbove, scUser, {"solverOption","deleteCommentsAbove"}, false);

    // GAMS Engine settings
    safelyAdd(res, skEngineUrl, scSys, {"engine","page"}, "");
    safelyAdd(res, skEngineAuthMethod, scSys, {"engine","authMethod"}, 0);
    safelyAdd(res, skEngineUser, scSys, {"engine","user"}, "");
    safelyAdd(res, skEngineUserToken, scSys, {"engine","userToken"}, "");
    safelyAdd(res, skEngineStoreUserToken, scUser, {"engine","storeUserToken"}, false);
    safelyAdd(res, skEngineSsoName, scSys, {"engine","ssoName"}, "");
    safelyAdd(res, skEngineAuthExpire, scUser, {"engine","authExpire"}, 60*24*7);
    safelyAdd(res, skEngineIsSelfCert, scSys, {"engine","isSelfCert"}, false);
    safelyAdd(res, skEngineNamespace, scSys, {"engine","namespace"}, "");
    safelyAdd(res, skEngineUserInstance, scSys, {"engine","userInstance"}, "");
    safelyAdd(res, skEngineForceGdx, scSys, {"engine","forceGdx"}, true);

    // syntax color settings
    safelyAdd(res, skUserThemes, scTheme, {"theme"}, QJsonArray());

    // Check gams update
    safelyAdd(res, skAutoUpdateCheck, scSys, {"update", "autoUpdateCheck"}, -1);
    safelyAdd(res, skUpdateInterval, scSys, {"update", "updateInterval"}, UpdateCheckInterval::Daily);
    safelyAdd(res, skLastUpdateCheckDate, scSys, {"update", "lastUpdateDate"}, QDate());
    safelyAdd(res, skNextUpdateCheckDate, scSys, {"update", "nextUpdateDate"}, QDate());

    // Check if all enum values of SettingsKey have been assigned
    for (int i = 0 ; i < skSettingsKeyCount ; ++i) {
        if (!res.contains(static_cast<SettingsKey>(i))) {
            DEB() << "WARNING: Unused SettingsKey [" << QString::number(i) << "]";
        }
    }

    return res;
}

void Settings::initDefault()
{
    for (QHash<SettingsKey, KeyData>::const_iterator di = mKeys.constBegin() ; di != mKeys.constEnd() ; ++di) {
        const KeyData &dat = di.value();
        setValue(di.key(), dat.initial);
    }
}

void Settings::addVersionInfo(Settings::Scope scope, QVariantMap &map)
{
    ScopePair scopes = scopePair(scope);
    QVariantMap ver;

    KeyData dat = mKeys.value(skVersionGamsStudio);
    ver.insert(dat.keys.last(), QString(GAMS_VERSION_STR));

    // TODO(JM) need to be extended for new versions - find a generic solution
    dat = mKeys.value(scopes.versionized==scSysX ? skVersionSysSettings : skVersionUserSettings);
    ver.insert(dat.keys.last(), mVersion.value(scopes.versionized));

    map.insert(dat.keys.first(), ver);
}

void Settings::reload()
{
    loadFile(scSys);
    loadFile(scUser);
    loadFile(scTheme);
}

QList<SettingsKey> Settings::viewKeys()
{
    return QList<SettingsKey> {
        skWinPos, skWinSize, skWinState, skWinMaximized, skWinFullScreen,
        skViewHelp, skViewOption, skViewOutput, skViewProject
    };
}

void Settings::resetKeys(const QList<SettingsKey> &keys)
{
    for (const SettingsKey &key : keys) {
        KeyData dat = mKeys.value(key);
        setValue(key, dat.initial);
    }
}

void Settings::save()
{
    // ignore-settings argument -> no settings assigned
    if (!canWrite()) return;
    for (auto it = mSettings.constBegin() ; it != mSettings.constEnd() ; ++it) {
        saveFile(it.key());
    }
    // Themes are stored dynamically in multiple files
    saveFile(Scope::scTheme);
}

void Settings::block()
{
    mBlock = true;
}

void Settings::unblock()
{
    mBlock = false;
}

QSize Settings::toSize(SettingsKey key) const
{
    return stringToSize(toString(key));
}

QPoint Settings::toPoint(SettingsKey key) const
{
    return stringToPoint(toString(key));
}

QList<int> Settings::toIntList(SettingsKey key) const
{
    return toIntArray(toString(key));
}

QByteArray Settings::toByteArray(SettingsKey key) const
{
    QString str = value(key).toString();
    return QByteArray::fromHex(str.toLatin1());
}

QVariantMap Settings::toMap(SettingsKey key) const
{
    return value(key).toMap();
}

QVariantList Settings::toList(SettingsKey key) const
{
    return value(key).toList();
}

QDate Settings::toDate(SettingsKey key) const
{
    return QDate::fromString(value(key).toString(), Qt::ISODate);
}

void Settings::setSize(SettingsKey key, const QSize &value)
{
    setValue(key, sizeToString(value));
}

void Settings::setPoint(SettingsKey key, const QPoint &value)
{
    setValue(key, pointToString(value));
}

void Settings::setIntList(SettingsKey key, const QList<int> &value)
{
    QString stringVal;
    for (const int &val : value) {
        if (!stringVal.isEmpty()) stringVal += ',';
        stringVal += QString::number(val);
    }
    setValue(key, stringVal);
}

bool Settings::setMap(SettingsKey key, const QVariantMap &value)
{
    return setValue(key, value);
}

bool Settings::setList(SettingsKey key, const QVariantList &value)
{
    return setValue(key, value);
}

bool Settings::setDate(SettingsKey key, QDate value)
{
    return setValue(key, value.toString(Qt::ISODate));
}

bool Settings::isValidVersion(const QString &currentVersion)
{
    for (const QChar &c: currentVersion)
        if (c != '.' && (c < '0' || c > '9')) return false;
    const QStringList verList = currentVersion.split('.');
    if (verList.size() < 2) return false;
    for (const QString &s: verList)
        if (s.isEmpty()) return false;
    return true;
}

int Settings::compareVersion(const QString &currentVersion, const QString &otherVersion)
{
    QStringList curntList = currentVersion.split('.');
    QStringList otherList = otherVersion.split('.');
    for (int i = 0; i < qMax(curntList.size(), otherList.size()); ++i) {
        if (i == curntList.size()) return -1;
        if (i == otherList.size()) return 1;
        bool a,b;
        int res = curntList.at(i).toInt(&a) - otherList.at(i).toInt(&b);
        if (a && !b) return 2;
        if (b && !a) return -2;
        if (res) return res/qAbs(res);
    }
    return 0;
}

QVariant Settings::value(SettingsKey key) const
{
    KeyData dat = mKeys.value(key);
    if (!mData.contains(dat.scope))
        return QVariant();
    if (dat.keys.length() == 1)
        return mData[dat.scope].value(dat.keys.at(0));
    if (dat.keys.length() == 2) {
        QJsonObject jo = mData[dat.scope].value(dat.keys.at(0)).toJsonObject();
        return jo.value(dat.keys.at(1));
    }
    return QVariant();
}

bool Settings::setValue(SettingsKey key, const QVariant &value)
{
    KeyData dat = mKeys.value(key);
    if (dat.keys.length() == 1)
        return setDirectValue(dat.scope, dat.keys.at(0), value);
    if (dat.keys.length() == 2)
        return setDirectValue(dat.scope, dat.keys.at(0), dat.keys.at(1), value);
    return false;
}

QVariant Settings::directValue(const Scope &scope, const QString &key) const
{
    if (!mData.contains(scope)) return QVariant();
    return mData[scope].value(key);
}

QVariant Settings::directValue(const Settings::Scope &scope, const QString &group, const QString &key) const
{
    if (!mData.contains(scope)) return QVariant();
    QJsonObject jo = mData[scope].value(group).toJsonObject();
    return jo.value(key);
}

bool Settings::setDirectValue(const Settings::Scope &scope, const QString &key, const QVariant &value)
{
    if (!mData.contains(scope)) // ensure that entry for the scope exists
        mData.insert(scope, Data());
    mData[scope].insert(key, value);
    return true;
}

bool Settings::setDirectValue(const Settings::Scope &scope, const QString &group, const QString &key, const QVariant &value)
{
    if (!mData.contains(scope)) // ensure that entry for the scope exists
        mData.insert(scope, Data());
    if (!mData[scope].contains(group))
        mData[scope].insert(group, QJsonObject());
    QVariant varGroup = mData[scope].value(group);
    if (!varGroup.canConvert<QJsonObject>() && !varGroup.canConvert<QVariantMap>())
        return false;
    QVariantMap jo = varGroup.toMap();
    if (!addToMap(jo, key, value)) return false;
    mData[scope].insert(group, jo);
    return true;
}

bool Settings::addToMap(QVariantMap &group, const QString &key, const QVariant &value)
{
    switch (QMetaType::Type(value.typeId())) {
    case QMetaType::Double: group[key] = value.toDouble(); break;
    case QMetaType::ULongLong:
    case QMetaType::LongLong: group[key] = value.toLongLong(); break;
    case QMetaType::UInt:
    case QMetaType::Int: group[key] = value.toInt(); break;
    case QMetaType::Bool: group[key] = value.toBool(); break;
    case QMetaType::QByteArray: group[key] = QString(value.toByteArray().toHex()); break;
    case QMetaType::QString: group[key] = value.toString(); break;
    case QMetaType::QJsonObject: group[key] = value.toJsonObject(); break;
    case QMetaType::QJsonArray: group[key] = value.toJsonArray(); break;
    case QMetaType::QVariantList: group[key] = value.toList(); break;
    case QMetaType::QVariantMap: group[key] = value.toMap(); break;
    default: return false;
    }
    return true;
}

QString Settings::keyText(SettingsKey key)
{
    KeyData dat = mKeys.value(key);
    return dat.keys.join("/");
}

QString Settings::settingsPath()
{
    if (QSettings *settings = mSettings[scSys]) {
        return QFileInfo(settings->fileName()).path();
    }
    DEB() << "ERROR: Settings file must be initialized before using settingsPath()";
    return QString();
}

void Settings::saveFile(Scope scope)
{
    if (!canWrite()) return;

    if (scope >= scTheme) {
        saveThemes();
        return;
    }

    ScopePair scopes = scopePair(scope);

    if (!mSettings.contains(scopes.base)) {
        DEB() << "WARNING: missing initialization for scope [" << QString::number(scopes.base) << "]";
        return; // for safety
    }
    QSettings *settings = mSettings.value(scopes.base);

    // store base settings
    QVariantMap baseDat;
    Data src = mData.value(scopes.base);
    for (Data::const_iterator it = src.constBegin() ; it != src.constEnd() ; ++it) {
        baseDat.insert(it.key(), it.value());
    }

    addVersionInfo(scope, baseDat);
    settings->setValue("base", baseDat);

    // store versionized settings
    QVariantMap verDat;
    src = mData.value(scopes.versionized);
    for (Data::const_iterator it = src.constBegin() ; it != src.constEnd() ; ++it) {
        verDat.insert(it.key(), it.value());
    }
    addVersionInfo(scope, verDat);
    settings->setValue(QString("v%1").arg(version(scopes.versionized)), verDat);

    settings->sync();
}

void Settings::loadMap(Scope scope, const QVariantMap &map)
{
    for (QVariantMap::const_iterator it = map.constBegin() ; it != map.constEnd() ; ++it) {
        QVariant var = it.value();
        if (var.isNull()) continue;
        if (var.canConvert<QJsonObject>() || var.canConvert<QVariantMap>()) {
            // copy all elements
            QJsonObject joSrc = var.toJsonObject();
            QJsonObject joDest = mData.value(scope).value(it.key()).toJsonObject();
            const QStringList srcKeys = joSrc.keys();
            for (const QString &joKey : srcKeys) {
                joDest[joKey] = joSrc[joKey];
            }
            var = joDest;
        }
        setDirectValue(scope, it.key(), var);
    }
}

QVariant Settings::read(SettingsKey key)
{
    KeyData dat = keyData(key);
    QSettings *qs = mSettings.value(dat.scope, nullptr);
    if (!qs) return QVariant();
    if (dat.keys.size() == 1) return  qs->value(dat.keys[0]);
    if (dat.keys.size() == 2) {
        QJsonObject jo = qs->value(dat.keys[0]).toJsonObject();
        return jo[dat.keys[1]].toVariant();
    }
    return QVariant();
}

void Settings::saveThemes()
{
    const QVariantList themes = toList(skUserThemes);

    // remove previous theme files
    QDir dir(settingsPath());
    const auto entries = dir.entryInfoList(QDir::Filter::Files, QDir::Name | QDir::IgnoreCase);
    for (const QFileInfo &fileInfo : entries) {
        QString fName = fileInfo.fileName();
        if (!fName.startsWith(CThemePrefix, Qt::CaseInsensitive) || !fName.endsWith(CThemeSufix, Qt::CaseInsensitive)) {
            continue;
        }
        dir.remove(fileInfo.fileName());
    }

    // create current theme files
    for (int i = 0 ; i < themes.count() ; ++i) {
        const QVariant &vTheme = themes.at(i);
        QVariantMap theme = vTheme.toMap();
        QString name = theme.value("name").toString();
        QString fName = CThemePrefix + name.toLower();
        QSettings themeSettings(QSettings::defaultFormat(), QSettings::UserScope, GAMS_ORGANIZATION_STR, fName);
        themeSettings.setValue("name", name);
        themeSettings.setValue("base", theme.value("base"));
        themeSettings.setValue("theme", theme.value("theme"));
        themeSettings.sync();
    }
}

void Settings::exportTheme(const QVariant &vTheme, const QString &fileName)
{
    QVariantMap theme = vTheme.toMap();
    QString name = theme.value("name").toString();
//    QString fName = path + '/' + CThemePrefix + name.toLower();
    QSettings themeSettings(fileName, QSettings::defaultFormat());
    themeSettings.setValue("name", name);
    themeSettings.setValue("base", theme.value("base"));
    themeSettings.setValue("theme", theme.value("theme"));
    themeSettings.sync();
}

void Settings::loadThemes()
{
    QVariantList themes;
    QStringList themeNames;

    QDir dir(settingsPath());
    const auto entries = dir.entryInfoList(QDir::Filter::Files, QDir::Name | QDir::IgnoreCase);
    for (const QFileInfo &fileInfo : entries) {
        QString fName = fileInfo.fileName();
        if (!fName.startsWith(CThemePrefix, Qt::CaseInsensitive) || !fName.endsWith(CThemeSufix, Qt::CaseInsensitive)) {
            continue;
        }
        QSettings themeSettings(QSettings::defaultFormat(), QSettings::UserScope, GAMS_ORGANIZATION_STR, fileInfo.completeBaseName());
        QString name = themeSettings.value("name").toString();
        bool ok;
        int base = themeSettings.value("base").toInt(&ok);
        if (!ok) base = 0;
        QVariantMap data = themeSettings.value("theme").toMap();
        if (data.isEmpty()) {
            SysLogLocator::systemLog()->append("Skipping empty theme '" + name + "' from " + fName);
            continue;
        }
        if (fName.compare(CThemePrefix+name+'.'+CThemeSufix, Qt::CaseInsensitive) != 0) {
            SysLogLocator::systemLog()->append("Skipping theme name doesn't match filename in " + fName);
            continue;
        }
        int ind = themeNames.indexOf(name.toLower());
        if (ind >= 0) {
            SysLogLocator::systemLog()->append("Skipping theme duplicate from " + fName);
            continue;
        }

        QVariantMap theme;
        theme.insert("name", name);
        theme.insert("base", base);
        theme.insert("theme", data);
        themes << theme;
        themeNames << name.toLower();
    }
    setList(skUserThemes, themes);
}

QVariantMap Settings::importTheme(const QString &filepath)
{
    QSettings themeSettings(filepath, QSettings::defaultFormat());
    QString name = themeSettings.value("name").toString();
    bool ok;
    int base = themeSettings.value("base").toInt(&ok);
    if (!ok) base = 0;
    QVariantMap data = themeSettings.value("theme").toMap();
    if (data.isEmpty()) return QVariantMap();

    QVariantMap theme;
    theme.insert("name", name);
    theme.insert("base", base);
    theme.insert("theme", data);
    return theme;
}


int Settings::usableVersion(ScopePair scopes)
{
    int res = version(scopes.versionized);
    QSettings *settings = mSettings.value(scopes.base);
    if (settings) {
        while (res) {
            if (settings->contains(QString("v%1").arg(res))) break;
            --res;
        }
    }
    // nothing to do if no versionized part exists
    if (!res) res = version(scopes.versionized);
    return res;
}

void Settings::loadVersionData(ScopePair scopes)
{
    int scopeVersion = version(scopes.versionized);
    int foundVersion = usableVersion(scopes);

    QSettings *settings = mSettings.value(scopes.base);

    // load found version data
    if (settings->contains(QString("v%1").arg(foundVersion))) {
        QVariantMap dat = settings->value(QString("v%1").arg(foundVersion)).toMap();
        loadMap(scopes.versionized, dat);
    }

    // upgrade version data if neccesary
    while (foundVersion < scopeVersion) {
        if (scopes.base == scSys) {

            // --------- version handling for scSys ---------
            switch (foundVersion) {
            case 1: { // On increasing version from 1 to 2 -> implement mData conversion HERE

                break;
            }
            case 2: { // On increasing version from 2 to 3 -> implement mData conversion HERE

                break;
            }
            default:
                break;
            }

        } else if (scopes.base == scUser) {

            // --------- version handling for scUser ---------
            switch (foundVersion) {
            case 1: { // On increasing version from 1 to 2 -> implement mData conversion HERE
                if (!setValue(skEdAppearance, directValue(scUser, "editor", "colorSchemeIndex").toInt()))
                    DEB() << "Error on upgrading value to version " << (foundVersion+1) << " for " << keyText(skEdAppearance);
                break;
            }
            case 2: { // On increasing version from 2 to 3 -> implement mData conversion HERE

                break;
            }
            default:
                break;
            }

        } else if (scopes.base == scTheme) {

            // --------- version handling for scSyntax ---------
            switch (foundVersion) {
            case 1: { // On increasing version from 1 to 2 -> implement mData conversion HERE

                break;
            }
            case 2: { // On increasing version from 2 to 3 -> implement mData conversion HERE

                break;
            }
            default:
                break;
            }

        } else {
            DEB() << "Warning: Missing version handling for scope " << scopes.base;
        }
        ++foundVersion;
    }
}

void Settings::loadFile(Scope scope)
{
    if (!mCanRead) return;

    if (scope == scTheme) {
        loadThemes();
        return;
    }

    ScopePair scopes = scopePair(scope);
    // load settings content into mData
    for (QMap<Scope, QSettings*>::const_iterator si = mSettings.constBegin() ; si != mSettings.constEnd() ; ++si) {
        if (!si.value() || scopes.base != si.key()) continue;
        QSettings *settings = si.value();
        // sync settings of this scope
        settings->sync();

        QVariantMap dat = settings->value("base").toMap();
        loadMap(scopes.base, dat);
        loadVersionData(scopes);
    }

//    Theme::instance()->initDefault();
}

void Settings::importSettings(const QString &path)
{
    if (!mSettings.value(scUser)) return;
    QFile backupFile(path);
    QFile settingsFile(mSettings.value(scUser)->fileName());
    settingsFile.remove(); // remove old file
    backupFile.copy(settingsFile.fileName()); // import new file
    reload();
}

void Settings::exportSettings(const QString &path)
{
    if (!mSettings.value(scUser)) return;
    QFile originFile(mSettings.value(scUser)->fileName());
    if (QFile::exists(path))
        QFile::remove(path);
    if (!originFile.copy(path)) {
        SysLogLocator::systemLog()->append("Error exporting settings to " + path, LogMsgType::Error);
    }
}

QString Settings::themeFileName(const QString &baseName)
{
    return CThemePrefix+baseName.toLower()+".json";
}

QStringList Settings::takeInitWarnings()
{
    QStringList res = mInitWarnings;
    mInitWarnings.clear();
    return res;
}

}
}
