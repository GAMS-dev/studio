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
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QSettings>
#include <QFile>
#include "settings.h"
#include "mainwindow.h"
#include "commonpaths.h"
#include "search/searchdialog.h"
#include "version.h"
#include "commandlineparser.h"
#include "scheme.h"
#include "exception.h"
#include "file/dynamicfile.h"

namespace gams {
namespace studio {

// Increase mVersion only on MAJOR changes (change/remove existing field or add to array-element)
const int Settings::mVersion = 1;
Settings *Settings::mInstance = nullptr;

// ====== Some helper functions for string conversion ======

QString pointToString(QPoint p) {
    return QString("%1,%2").arg(p.x()).arg(p.y());
}
QString sizeToString(QSize s) {
    return QString("%1,%2").arg(s.width()).arg(s.height());
}
QList<int> toIntArray(QString s) {
    QList<int> res;
    for (QString v : s.split(',')) res << v.toInt();
    return res;
}
QPoint toPoint(QString s) {
    QList<int> a = toIntArray(s);
    if (a.size() == 2) return QPoint(a.at(0), a.at(1));
    return QPoint();
}
QSize toSize(QString s) {
    QList<int> a = toIntArray(s);
    if (a.size() == 2) return QSize(a.at(0), a.at(1));
    return QSize();
}

QString findFixedFont()
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    if (font.fixedPitch())
        return font.family();
    QFontDatabase fdb;
    QStringList list = fdb.families();
    for (int i = 0; i < list.size(); ++i) {
        if (fdb.isPrivateFamily(list.at(i)))
            continue;
        if (fdb.isFixedPitch(list.at(i))) {
            return list.at(i);
        }
    }
    DEB() << "No fixed font found on system. Using " << font.family();
    return font.family();
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

Settings::Settings(bool ignore, bool reset, bool resetView)
    : mIgnoreSettings(ignore),
      mResetSettings(reset)
{
    // initialize json format and make it the default
    QSettings::Format jsonFormat = QSettings::registerFormat("json", readJsonFile, writeJsonFile);
    QSettings::setDefaultFormat(jsonFormat);

    // prepare storage
    mData.insert(KUi, Data());
    mData.insert(KSys, Data());
    mData.insert(KUser, Data());
    initKeys();

    initDefaults();
    if (!ignore) {
        // create basic non versionized application settings
        mSettings.insert(KUi, new QSettings(QSettings::defaultFormat(), QSettings::UserScope,
                                    GAMS_ORGANIZATION_STR, "uistates"));
        if (mSettings[KUi]->status()) {
            // On Error -> create a backup and mark to reset this settings file
            DynamicFile file(mSettings[KUi]->fileName(), 2);
        }
        // create versionized settings (may init from older version)
        createSettingFiles();
    }
    if (reset) {
        initDefaults();
    }
    if (resetView) resetViewSettings();

    QDir location(settingsPath());
    for (const QString &fileName: location.entryList({"*.lock"})) {
        QFile f(location.path() +  "/" + fileName);
        f.remove();
    }
}

Settings::~Settings()
{
    delete mSettings.take(KUi);
    delete mSettings.take(KSys);
    delete mSettings.take(KUser);
    // TODO(JM) Handle interface.json
}

void Settings::initKeys()
{
    // versions are kept synchronous in the different settings files
    mKeys.insert(_sVersionSettings, KeyData(KSys, {"version","settings"}, mVersion));
    mKeys.insert(_sVersionStudio, KeyData(KSys, {"version","studio"}, QString(GAMS_VERSION_STR)));

    mKeys.insert(_uVersionSettings, KeyData(KUser, {"version","settings"}, mVersion));
    mKeys.insert(_uVersionStudio, KeyData(KUser, {"version","studio"}, QString(GAMS_VERSION_STR)));

    // window settings
    mKeys.insert(_winSize, KeyData(KUi, {"window","size"}, QString("1024,768")));
    mKeys.insert(_winPos, KeyData(KUi, {"window","pos"}, QString("0,0")));
    mKeys.insert(_winState, KeyData(KUi, {"window","state"}, QString("")));
    mKeys.insert(_winMaximized, KeyData(KUi, {"window","maximized"}, false));

    // view menu settings
    mKeys.insert(_viewProject, KeyData(KUi, {"viewMenu","project"}, true));
    mKeys.insert(_viewOutput, KeyData(KUi, {"viewMenu","output"}, true));
    mKeys.insert(_viewHelp, KeyData(KUi, {"viewMenu","help"}, false));
    mKeys.insert(_viewOption, KeyData(KUi, {"viewMenu","optionEdit"}, false));

    // general system settings
    mKeys.insert(_encodingMib, KeyData(KSys, {"encodingMIBs"}, QString("106,0,4,17,2025")));

    // settings of help page
    mKeys.insert(_hBookmarks, KeyData(KSys, {"help","bookmarks"}, QJsonArray()));
    mKeys.insert(_hZoomFactor, KeyData(KSys, {"help","bookmarks"}, 1.0));

    // search widget
    mKeys.insert(_searchUseRegex, KeyData(KSys, {"search", "regex"}, false));
    mKeys.insert(_searchCaseSens, KeyData(KSys, {"search", "caseSens"}, false));
    mKeys.insert(_searchWholeWords, KeyData(KSys, {"search", "wholeWords"}, false));
    mKeys.insert(_searchScope, KeyData(KSys, {"search", "scope"}, 0));

    // general settings page
    mKeys.insert(_defaultWorkspace, KeyData(KUser, {"defaultWorkspace"}, CommonPaths::defaultWorkingDir()));
    mKeys.insert(_skipWelcomePage, KeyData(KUser, {"skipWelcome"}, false));
    mKeys.insert(_restoreTabs, KeyData(KUser, {"restoreTabs"}, true));
    mKeys.insert(_autosaveOnRun, KeyData(KUser, {"autosaveOnRun"}, true));
    mKeys.insert(_openLst, KeyData(KUser, {"openLst"}, false));
    mKeys.insert(_jumpToError, KeyData(KUser, {"jumpToError"}, true));
    mKeys.insert(_foregroundOnDemand, KeyData(KUser, {"foregroundOnDemand"}, true));
    mKeys.insert(_historySize, KeyData(KUser, {"historySize"}, 12));

    // editor settings page
    mKeys.insert(_edFontFamily, KeyData(KUser, {"editor","fontFamily"}, findFixedFont()));
    mKeys.insert(_edFontSize, KeyData(KUser, {"editor","fontSize"}, 10));
    mKeys.insert(_edShowLineNr, KeyData(KUser, {"editor","showLineNr"}, true));
    mKeys.insert(_edTabSize, KeyData(KUser, {"editor","TabSize"}, 4));
    mKeys.insert(_edLineWrapEditor, KeyData(KUser, {"editor","lineWrapEditor"}, false));
    mKeys.insert(_edLineWrapProcess, KeyData(KUser, {"editor","lineWrapProcess"}, false));
    mKeys.insert(_edClearLog, KeyData(KUser, {"editor","clearLog"}, false));
    mKeys.insert(_edWordUnderCursor, KeyData(KUser, {"editor","wordUnderCursor"}, false));
    mKeys.insert(_edHighlightCurrentLine, KeyData(KUser, {"editor","highlightCurrentLine"}, false));
    mKeys.insert(_edAutoIndent, KeyData(KUser, {"editor","autoIndent"}, true));
    mKeys.insert(_edWriteLog, KeyData(KUser, {"editor","writeLog"}, true));
    mKeys.insert(_edLogBackupCount, KeyData(KUser, {"editor","logBackupCount"}, 3));
    mKeys.insert(_edAutoCloseBraces, KeyData(KUser, {"editor","autoCloseBraces"}, true));
    mKeys.insert(_edEditableMaxSizeMB, KeyData(KUser, {"editor","editableMaxSizeMB"}, 50));

    // MIRO settings page
    mKeys.insert(_miroInstallPath, KeyData(KUser, {"miro","installationLocation"}, QString()));

    // solver option editor settings
    mKeys.insert(_soOverrideExisting, KeyData(KUser, {"solverOption","overrideExisting"}, true));
    mKeys.insert(_soAddCommentAbove, KeyData(KUser, {"solverOption","addCommentAbove"}, false));
    mKeys.insert(_soAddEOLComment, KeyData(KUser, {"solverOption","addEOLComment"}, false));
    mKeys.insert(_soDeleteCommentsAbove, KeyData(KUser, {"solverOption","deleteCommentsAbove"}, false));

    // user model library directory
    mKeys.insert(_userModelLibraryDir, KeyData(KSys, {"userModelLibraryDir"}, CommonPaths::userModelLibraryDir()));

}

int Settings::checkVersion()
{
    int res = mVersion;
    QDir dir = settingsPath();

    // Find setting files of the highest version up to mVersion
    QStringList files = dir.entryList(QDir::Files);
    while (res) {
        if (files.contains(QString("systemsettings%1.json").arg(res))) break;
        if (files.contains(QString("usersettings%1.json").arg(res))) break;
        --res;
    }
    // nothing to do if no setting file found
    if (!res) res = mVersion;

    return res;
}

bool Settings::createSettingFiles()
{
    // look for latest setting files
    int version = checkVersion();

    // create setting files of found version to read from
    initSettingsFiles(version);
    load();
    if (version == mVersion) return true;

    // Need to upgrade from older version
    while (version < mVersion) {
        switch (version) {
        case 1:
            // On increasing version from 1 to 2 -> implement mData conversion HERE
            break;
        case 2:
            // On increasing version from 2 to 3 -> implement mData conversion HERE
            break;
        default:
            break;
        }
        ++version;
    }

    // write setting files in current version
    initSettingsFiles(version);
    save();
    return true;
}

void Settings::initSettingsFiles(int version)
{
    // initializes versionized setting files
    mSettings.insert(KSys, new QSettings(QSettings::defaultFormat(),  QSettings::UserScope,
                                         GAMS_ORGANIZATION_STR, QString("systemsettings%1").arg(version)));
    mSettings.insert(KUser, new QSettings(QSettings::defaultFormat(), QSettings::UserScope,
                                          GAMS_ORGANIZATION_STR, QString("usersettings%1").arg(version)));
    // TODO(JM) Handle studioscheme.json and syntaxscheme.json
}

void Settings::initDefaultsUi()
{
    Data data;
    // UI settings ---------------------
    QJsonObject joMainWin;
    joMainWin["size"] = "1024,768";
    joMainWin["pos"] = "0,0";
    joMainWin["windowState"] = "";
    joMainWin["maximized"] = false;
    data["mainWindow"] = joMainWin;

    QJsonObject joViewMenu;
    joViewMenu["projectView"] = true;
    joViewMenu["outputView"] = true;
    joViewMenu["helpView"] = false;
    joViewMenu["optionEditor"] = false;
    data["viewMenu"] = joViewMenu;

    mData.insert(KUi, data);
}

void Settings::save(Kind kind)
{
    if (!mSettings.contains(kind)) return;
    Data::const_iterator it = mData[kind].constBegin();
    while (it != mData[kind].constEnd()) {
        mSettings[kind]->setValue(it.key(), it.value());
    }
    mSettings[kind]->sync();
}

void Settings::initDefaults()
{
    // UI settings ---------------------
    initDefaultsUi();

    QHash<SettingsKey, KeyData>::const_iterator it = mKeys.constBegin();
    while (it != mKeys.constEnd()) {
        KeyData dat = it.value();

        ++it;
    }

    // System settings -----------------

    QJsonObject joVersion;
    joVersion["settings"] = mVersion;
    joVersion["studio"] = GAMS_VERSION;
    mData[KSys].insert("version", joVersion);

    QJsonObject joSearch;
    joSearch["regex"] = false;
    joSearch["caseSens"] = false;
    joSearch["wholeWords"] = false;
    joSearch["scope"] = 0;
    mData[KSys].insert("search", joSearch);

    mData[KSys].insert("encodingMIBs", "106,0,4,17,2025");

    QJsonObject joHelp; // "helpView" empty as default

    // User settings -------------------

    mData[KUser].insert("version", joVersion);
    mData[KUser].insert("defaultWorkspace", CommonPaths::defaultWorkingDir());
    mData[KUser].insert("skipWelcome", false);
    mData[KUser].insert("restoreTabs", true);
    mData[KUser].insert("autosaveOnRun", true);
    mData[KUser].insert("openLst", false);
    mData[KUser].insert("jumpToError", true);
    mData[KUser].insert("bringOnTop", true);
    mData[KUser].insert("historySize", 12);


    QJsonObject joEditor;
    joEditor["fontFamily"] = findFixedFont();
    joEditor["fontSize"] = 10;
    joEditor["showLineNr"] = true;
    joEditor["tabSize"] = 4;
    joEditor["lineWrapEditor"] = false;
    joEditor["lineWrapProcess"] = false;
    joEditor["clearLog"] = false;
    joEditor["wordUnderCursor"] = false;
    joEditor["highlightCurrentLine"] = false;
    joEditor["autoIndent"] = true;
    joEditor["writeLog"] = true;
    joEditor["nrLogBackups"] = 3;
    joEditor["autoCloseBraces"] = true;
    joEditor["editableMaxSizeMB"] = 50;
    mData[KUser].insert("editor", joEditor);

    QJsonObject joSolverOption;
    joSolverOption["overrideExisting"] = true;
    joSolverOption["addCommentAbove"] = false;
    joSolverOption["addEOLComment"] = false;
    joSolverOption["deleteCommentsAbove"] = false;
    mData[KUser].insert("solverOption", joSolverOption);

    QJsonObject joMiro;
    joMiro["installationLocation"] = "";
    mData[KUser].insert("miro", joMiro);

    // create default values for current mVersion
    Scheme::instance()->initDefault();
}


void Settings::bind(MainWindow* main)
{
    mMain = main;
}

void Settings::reloadSettings()
{
    mUiSettings->sync();
    mSystemSettings->sync();
    mUserSettings->sync();
    // TODO(JM) Handle interface.json
    Scheme::instance()->initDefault();
}

void Settings::resetViewSettings()
{
    initDefaultsUi();
    mUiSettings->sync();
}

bool Settings::resetSettingsSwitch()
{
    return mResetSettings;
}

void Settings::fetchData()
{
    // Updates mData from values in MainWindow

    // UI data ---------------------

    QJsonObject joMainWin = mData.value(KUi).value("MainWindow").toJsonObject();
    joMainWin["size"] = sizeToString(mMain->size());
    joMainWin["pos"] = pointToString(mMain->pos());
    joMainWin["windowState"] = mMain->saveState().data();
    joMainWin["maximized"] = mMain->isMaximized();
    mData[KUi].insert("mainWindow", joMainWin);

    QJsonObject joViewMenu = mData.value(KUi).value("viewMenu").toJsonObject();
    joViewMenu["projectView"] = mMain->projectViewVisibility();
    joViewMenu["outputView"] = mMain->outputViewVisibility();
    joViewMenu["helpView"] = mMain->helpViewVisibility();
    joViewMenu["optionEditor"] = mMain->optionEditorVisibility();
     mData[KUi].insert("viewMenu", joViewMenu);

     // system data --------------------

     mData[KSys].insert("encodingMIBs", mMain->encodingMIBsString());

     QJsonObject joSearch = mData.value(KSys).value("search").toJsonObject();
     joSearch["regex"] = mMain->searchDialog()->regex();
     joSearch["caseSens"] = mMain->searchDialog()->caseSens();
     joSearch["wholeWords"] = mMain->searchDialog()->wholeWords();
     joSearch["scope"] = mMain->searchDialog()->selectedScope();
     mData[KSys].insert("search", joSearch);

#ifdef QWEBENGINE
     QJsonObject joHelp = mData.value(KSys).value("help").toJsonObject();
     QJsonArray joBookmarks;
     // TODO(JM) Check with Jeed if this can be moved from multimap to map
     QMultiMap<QString, QString> bookmarkMap(mMain->helpWidget()->getBookmarkMap());
     for (int i = 0; i < bookmarkMap.size(); i++) {
         QJsonObject joBookmark;
         joBookmark["location"] = bookmarkMap.keys().at(i);
         joBookmark["name"] = bookmarkMap.values().at(i);
         joBookmarks << joBookmark;
     }
     joHelp["bookmarks"] = joBookmarks;
     joHelp["zoomFactor"] = mMain->helpWidget()->getZoomFactor();
     mData[KSys].insert("help", joHelp);
#endif

     QJsonArray joOpenFiles;
     for (const QString &file : mMain->history()->mLastOpenedFiles) {
         if (file.isEmpty()) break;
         QJsonObject joOpenFile;
         joOpenFile["file"] = file;
         joOpenFiles << joOpenFile;
     }
     mData[KSys].insert("lastOpenedFiles", joOpenFiles);

     QJsonObject joProjects;
     mMain->projectRepo()->write(joProjects);
     mData[KSys].insert("projects", joProjects);

     QJsonObject joTabs;
     mMain->writeTabs(joTabs);
     mData[KSys].insert("openTabs", joTabs);
}

void Settings::save()
{
    fetchData();

    // ignore-settings argument -> no settings assigned
    if (mSettings.isEmpty())
        return;

    save(KUi);
    save(KSys);
    save(KUser);

    // TODO(JM) temporarily deactivated
//    writeScheme();
}

void Settings::loadViewStates()
{
    mSystemSettings->beginGroup("settings");
    mSystemSettings->value("version").toString();
    mSystemSettings->endGroup();

    // main window
    mSystemSettings->beginGroup("mainWindow");
    bool maximized = mSystemSettings->value("maximized", false).toBool();
    if (maximized) {
        mMain->setWindowState(Qt::WindowMaximized);
    } else {
        mMain->resize(mSystemSettings->value("size", QSize(1024, 768)).toSize());
        mMain->move(mSystemSettings->value("pos", QPoint(100, 100)).toPoint());
    }
    mMain->restoreState(mSystemSettings->value("windowState").toByteArray());
    mMain->ensureInScreen();

    setSearchUseRegex(mSystemSettings->value("searchRegex", false).toBool());
    setSearchCaseSens(mSystemSettings->value("searchCaseSens", false).toBool());
    setSearchWholeWords(mSystemSettings->value("searchWholeWords", false).toBool());
    setSelectedScopeIndex(mSystemSettings->value("selectedScope", 0).toInt());

    mSystemSettings->endGroup();

    // tool-/menubar
    mSystemSettings->beginGroup("viewMenu");
    mMain->setProjectViewVisibility(mSystemSettings->value("projectView", true).toBool());
    mMain->setOutputViewVisibility(mSystemSettings->value("outputView", false).toBool());
    mMain->setExtendedEditorVisibility(mSystemSettings->value("gamsArguments", false).toBool());
    mMain->setHelpViewVisibility(mSystemSettings->value("helpView", false).toBool());
    mMain->setEncodingMIBs(mSystemSettings->value("encodingMIBs", "106,0,4,17,2025").toString());

    mSystemSettings->endGroup();

    // help
    mSystemSettings->beginGroup("helpView");
#ifdef QWEBENGINE
    QMultiMap<QString, QString> bookmarkMap;
    int mapsize = mSystemSettings->beginReadArray("bookmarks");
    for (int i = 0; i < mapsize; i++) {
        mSystemSettings->setArrayIndex(i);
        bookmarkMap.insert(mSystemSettings->value("location").toString(),
                           mSystemSettings->value("name").toString());
    }
    mSystemSettings->endArray();

    mMain->helpWidget()->setBookmarkMap(bookmarkMap);
    if (mSystemSettings->value("zoomFactor") > 0.0)
        mMain->helpWidget()->setZoomFactor(mSystemSettings->value("zoomFactor").toReal());
    else
        mMain->helpWidget()->setZoomFactor(1.0);
#endif
    mSystemSettings->endGroup();
}

bool Settings::isValidVersion(QString currentVersion)
{
    for (const QChar &c: currentVersion)
        if (c != '.' && (c < '0' || c > '9')) return false;
    QStringList verList = currentVersion.split('.');
    if (verList.size() < 2) return false;
    for (const QString &s: verList)
        if (s.isEmpty()) return false;
    return true;
}

int Settings::compareVersion(QString currentVersion, QString otherVersion)
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

//void checkFixedFont()
//{
//    QFontDatabase fdb;
//    QStringList list = fdb.families();
//    for (int i = 0; i < list.size(); ++i) {
//        if (fdb.isPrivateFamily(list.at(i)))
//            continue;
//        QFontMetrics metrics(QFont(list.at(i)));
//        DEB() << list.at(i) << "   fixed:" << (fdb.isFixedPitch(list.at(i)) ? "TRUE":"FALSE")
//              << "  width('W'=='l'):" << (metrics.width("W") == metrics.width("l") ? "TRUE":"FALSE")
//              << (fdb.isFixedPitch(list.at(i)) != (metrics.width("W") == metrics.width("l")) ? "  !!" : "");
//    }
//}

void Settings::loadUserIniSettings()
{
    mUserSettings->beginGroup("General");

    setDefaultWorkspace(mUserSettings->value("defaultWorkspace", CommonPaths::defaultWorkingDir()).toString());
    setSkipWelcomePage(mUserSettings->value("skipWelcome", false).toBool());
    setRestoreTabs(mUserSettings->value("restoreTabs", true).toBool());
    setAutosaveOnRun(mUserSettings->value("autosaveOnRun", true).toBool());
    setOpenLst(mUserSettings->value("openLst", false).toBool());
    setJumpToError(mUserSettings->value("jumpToError", true).toBool());
    setForegroundOnDemand(mUserSettings->value("bringOnTop",true).toBool());

    mUserSettings->endGroup();
    mUserSettings->beginGroup("Editor");

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    if (!font.fixedPitch()) {
        QString fontFamily = findFixedFont();
        font = QFont(fontFamily);
        if (fontFamily.isNull()) {
            DEB() << "No fixed font found on system. Using " << font.family();
        }
    }

    setFontFamily(mUserSettings->value("fontFamily", font.family()).toString());
    setFontSize(mUserSettings->value("fontSize", 10).toInt());
    setShowLineNr(mUserSettings->value("showLineNr", true).toBool());
    setTabSize(mUserSettings->value("tabSize", 4).toInt());
    setLineWrapEditor(mUserSettings->value("lineWrapEditor", false).toBool());
    setLineWrapProcess(mUserSettings->value("lineWrapProcess", false).toBool());
    setClearLog(mUserSettings->value("clearLog", false).toBool());
    setWordUnderCursor(mUserSettings->value("wordUnderCursor", false).toBool());
    setHighlightCurrentLine(mUserSettings->value("highlightCurrentLine", false).toBool());
    setAutoIndent(mUserSettings->value("autoIndent", true).toBool());
    setWriteLog(mUserSettings->value("writeLog", true).toBool());
    setNrLogBackups(mUserSettings->value("nrLogBackups", 3).toInt());
    setAutoCloseBraces(mUserSettings->value("autoCloseBraces", true).toBool());
    setEditableMaxSizeMB(mUserSettings->value("editableMaxSizeMB", 50).toInt());

    mUserSettings->endGroup();
    mUserSettings->beginGroup("Misc");

    setHistorySize(mUserSettings->value("historySize", 12).toInt());

    setOverrideExistingOption( mUserSettings->value("solverOptionOverrideExisting", overridExistingOption()).toBool() );
    setAddCommentDescriptionAboveOption( mUserSettings->value("solverOptionAddCommentAbove", addCommentDescriptionAboveOption()).toBool() );
    setAddEOLCommentDescriptionOption( mUserSettings->value("solverOptionAddEOLComment", addEOLCommentDescriptionOption()).toBool() );
    setDeleteAllCommentsAboveOption( mUserSettings->value("solverOptionDeleteCommentAbove", deleteAllCommentsAboveOption()).toBool() );

    mUserSettings->endGroup();
    mUserSettings->beginGroup("MIRO");

    setMiroInstallationLocation(mUserSettings->value("miroInstallationLocation", miroInstallationLocation()).toString());

    mUserSettings->endGroup();
}

QVariant Settings::value(SettingsKey key) const
{
    KeyData dat = mKeys.value(key);
    if (dat.keys.length() == 1)
        return mData[dat.kind].value(dat.keys.at(0));
    if (dat.keys.length() == 2) {
        QJsonObject jo = mData[dat.kind].value(dat.keys.at(0)).toJsonObject();
        return jo.value(dat.keys.at(1));
    }
    return QVariant();
}

bool Settings::toBool(SettingsKey key) const
{
    return value(key).toBool();
}

int Settings::toInt(SettingsKey key) const
{
    return value(key).toInt();
}

QString Settings::toString(SettingsKey key) const
{
    return value(key).toString();
}

bool Settings::setValue(SettingsKey key, QVariant value)
{
    KeyData dat = mKeys.value(key);
    if (dat.keys.length() == 1) {
        mData[dat.kind].insert(dat.keys.at(0), value);
        return true;
    }
    if (dat.keys.length() == 2) {
        QJsonObject jo = mData[dat.kind].value(dat.keys.at(0)).toJsonObject();
        switch (value.type()) {
        case QVariant::Double: jo[dat.keys.at(1)] = value.toDouble(); break;
        case QVariant::ULongLong:
        case QVariant::LongLong: jo[dat.keys.at(1)] = value.toLongLong(); break;
        case QVariant::UInt:
        case QVariant::Int: jo[dat.keys.at(1)] = value.toInt(); break;
        case QVariant::Bool: jo[dat.keys.at(1)] = value.toBool(); break;
        case QVariant::ByteArray:
        case QVariant::String: jo[dat.keys.at(1)] = value.toString(); break;
        default: return false;
        }
        mData[dat.kind].insert(dat.keys.at(0), jo);
        return true;
    }
    return false;
}

bool Settings::setValue(SettingsKey key, QJsonObject value)
{
    KeyData dat = mKeys.value(key);
    if (dat.keys.length() == 1) {
        mData[dat.kind].insert(dat.keys.at(0), value);
        return true;
    }
    if (dat.keys.length() == 2) {
        QJsonObject jo = mData[dat.kind].value(dat.keys.at(0)).toJsonObject();
        jo[dat.keys.at(1)] = value;
        mData[dat.kind].insert(dat.keys.at(0), jo);
        return true;
    }
    return false;
}


QStringList Settings::fileHistory()
{
    QStringList res;
    QJsonArray joLastOpenedFiles = mData[KSys].value("lastOpenedFiles").toJsonArray();
    for (QJsonValue jRef: joLastOpenedFiles) {
        res << jRef["name"].toString();
    }
    return res;
}

QString Settings::settingsPath()
{
    if (!mUiSettings) {
        DEB() << "ERROR: Settings file must be initialized before using settingsPath()";
        return QString();
    }
    return QFileInfo(mUiSettings->fileName()).path();
}

bool Settings::restoreTabsAndProjects()
{
    bool res = true;
    mSystemSettings->beginGroup("json");
    QByteArray saveData = mSystemSettings->value("projects", "").toByteArray();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    mMain->projectRepo()->read(loadDoc.object());

    if (restoreTabs()) {
        saveData = mSystemSettings->value("openTabs", "").toByteArray();
        loadDoc = QJsonDocument::fromJson(saveData);
        res = mMain->readTabs(loadDoc.object());
    }
    mSystemSettings->endGroup();
    return res;
}

void Settings::load()
{
    if (mResetSettings) {
        mUiSettings->clear();
        mSystemSettings->clear();
        mUserSettings->clear();
    }

    loadUserIniSettings();
    loadViewStates();

    // the location for user model libraries is not modifyable right now
    // anyhow, it is part of StudioSettings since it might become modifyable in the future
    mData[KUser].insert("userModelLibraryDir", CommonPaths::userModelLibraryDir());
}

void Settings::importSettings(const QString &path)
{
    QFile backupFile(path);
    QFile settingsFile(mUserSettings->fileName());

    settingsFile.remove(); // remove old file
    backupFile.copy(settingsFile.fileName()); // import new file
    reloadSettings();
    load();
}

QString Settings::defaultWorkspace() const
{
    return mData.value("defaultWorkspace").toString();
}

bool Settings::skipWelcomePage() const
{
    return mData.value("skipWelcomePage").toBool();
}

bool Settings::restoreTabs() const
{
    return mData.value("restoreTabs").toBool();
}

bool Settings::autosaveOnRun() const
{
    return mData.value("autosaveOnRun").toBool();
}

bool Settings::foregroundOnDemand() const
{
    return mData.value("foregroundOnDemand").toBool();
}

void Settings::setForegroundOnDemand(bool value)
{
    mData.insert("foregroundOnDemand", value);
}

bool Settings::openLst() const
{
    return mData.value("openLst").toBool();
}

void Settings::setOpenLst(bool value)
{
    mData.insert("openLst", value);
}

bool Settings::jumpToError() const
{
    return mData.value("jumpToError").toBool();
}

void Settings::setJumpToError(bool value)
{
    mData.insert("jumpToError", value);
}

int Settings::fontSize() const
{
    return mData.value("fontSize").toInt();
}

void Settings::setFontSize(int value)
{
    mData.insert("fontSize", value);
}

bool Settings::showLineNr() const
{
    return mData.value("showLineNr").toBool();
}

void Settings::setShowLineNr(bool value)
{
    mData.insert("showLineNr", value);
}

int Settings::tabSize() const
{
    return mData.value("tabSize").toInt();
}

void Settings::setTabSize(int value)
{
    mData.insert("tabSize", value);
}

bool Settings::lineWrapEditor() const
{
    return mData.value("lineWrapEditor").toBool();
}

void Settings::setLineWrapEditor(bool value)
{
    mData.insert("lineWrapEditor", value);
}

bool Settings::lineWrapProcess() const
{
    return mData.value("lineWrapProcess").toBool();
}

void Settings::setLineWrapProcess(bool value)
{
    mData.insert("lineWrapProcess", value);
}

QString Settings::fontFamily() const
{
    return mData.value("fontFamily").toString();
}

void Settings::setFontFamily(const QString &value)
{
    mData.insert("fontFamily", value);
}

bool Settings::clearLog() const
{
    return mData.value("clearLog").toBool();
}

void Settings::setClearLog(bool value)
{
    mData.insert("clearLog", value);
}

bool Settings::searchUseRegex() const
{
    return mData.value("searchUseRegex").toBool();
}

void Settings::setSearchUseRegex(bool searchUseRegex)
{
    mData.insert("searchUseRegex", searchUseRegex);
}

bool Settings::searchCaseSens() const
{
    return mData.value("searchCaseSens").toBool();
}

void Settings::setSearchCaseSens(bool searchCaseSens)
{
    mData.insert("searchCaseSens", searchCaseSens);
}

bool Settings::searchWholeWords() const
{
    return mData.value("searchWholeWords").toBool();
}

void Settings::setSearchWholeWords(bool searchWholeWords)
{
    mData.insert("searchWholeWords", searchWholeWords);
}

int Settings::selectedScopeIndex() const
{
    return mData.value("selectedScopeIndex").toInt();
}

void Settings::setSelectedScopeIndex(int selectedScopeIndex)
{
    mData.insert("selectedScopeIndex", selectedScopeIndex);
}

bool Settings::wordUnderCursor() const
{
    return mData.value("wordUnderCursor").toBool();
}

void Settings::setWordUnderCursor(bool wordUnderCursor)
{
    mData.insert("wordUnderCursor", wordUnderCursor);
}

QString Settings::userModelLibraryDir() const
{
    return mData.value("userModelLibraryDir").toString();
}

bool Settings::highlightCurrentLine() const
{
    return mData.value("highlightCurrentLine").toBool();
}

void Settings::setHighlightCurrentLine(bool highlightCurrentLine)
{
    mData.insert("highlightCurrentLine", highlightCurrentLine);
}

bool Settings::autoIndent() const
{
    return mData.value("autoIndent").toBool();
}

void Settings::setAutoIndent(bool autoIndent)
{
    mData.insert("autoIndent", autoIndent);
}

void Settings::exportSettings(const QString &path)
{
    QFile originFile(mUserSettings->fileName());
    originFile.copy(path);
}

}
}
