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

// Increase this only on major changes (change or remove of existing field)
const int Settings::mVersion = 1;
Settings *Settings::mInstance = nullptr;

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

Settings::Settings(bool ignore, bool reset, bool resetView)
    : mIgnoreSettings(ignore),
      mResetSettings(reset)
{
    // initialize json format and make it the default
    QSettings::Format jsonFormat = QSettings::registerFormat("json", readJsonFile, writeJsonFile);
    QSettings::setDefaultFormat(jsonFormat);

    // basic non versionized application settings
    mUiSettings = new QSettings(QSettings::defaultFormat(), QSettings::UserScope,
                                GAMS_ORGANIZATION_STR, "uistates");
    if (mUiSettings->status()) {
        // On Error -> create a backup and mark to reset this settings file
        DynamicFile file(mUiSettings->fileName(), 2);
    }

    initDefaults();
    if (!ignore) {
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

void Settings::initDefaults()
{
    // create default values for current mVersion
    Scheme::instance()->initDefault();

}


Settings::~Settings()
{
    delete mSystemSettings;
    mSystemSettings = nullptr;
    delete mUserSettings;
    mUserSettings = nullptr;
    // TODO(JM) Handle interface.json
}

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

void Settings::bind(MainWindow* main)
{
    mMain = main;
}

void Settings::initSettingsFiles(int version)
{
    mSystemSettings = new QSettings(QSettings::defaultFormat(), QSettings::UserScope,
                                 GAMS_ORGANIZATION_STR, QString("systemsettings%1").arg(version));
    mUserSettings = new QSettings(QSettings::defaultFormat(), QSettings::UserScope,
                                  GAMS_ORGANIZATION_STR, QString("usersettings%1").arg(version));
    // TODO(JM) Handle studioscheme.json and syntaxscheme.json
}

void Settings::reloadSettings()
{
    // TODO(JM)
    Scheme::instance()->initDefault();
    mSystemSettings->sync();
    mUserSettings->sync();
    // TODO(JM) Handle interface.json
}

void Settings::resetViewSettings()
{
    mSystemSettings->beginGroup("mainWindow");
    mSystemSettings->setValue("size", QSize(1024, 768));
    mSystemSettings->setValue("pos", QPoint());
    mSystemSettings->setValue("windowState", QByteArray());
    mSystemSettings->endGroup();

    mSystemSettings->beginGroup("viewMenu");
    mSystemSettings->setValue("projectView", true);
    mSystemSettings->setValue("outputView", true);
    mSystemSettings->setValue("helpView", false);
    mSystemSettings->setValue("optionEditor", false);
    mSystemSettings->endGroup();

    mSystemSettings->sync();
}

bool Settings::resetSettingsSwitch()
{
    return mResetSettings;
}

void Settings::save()
{
    // return directly only if settings are ignored and not resettet
    if (mIgnoreSettings && !mResetSettings)
        return;

    if (mSystemSettings == nullptr) {
        qDebug() << "ERROR: settings file missing.";
        return;
    }
    mSystemSettings->beginGroup("settings");
    mSystemSettings->setValue("version", QString(STUDIO_VERSION));
    mSystemSettings->endGroup();

    // Main Application Settings
    // main window
    mSystemSettings->beginGroup("mainWindow");
    mSystemSettings->setValue("size", mMain->size());
    mSystemSettings->setValue("pos", mMain->pos());
    mSystemSettings->setValue("maximized", mMain->isMaximized());
    mSystemSettings->setValue("windowState", mMain->saveState());

    // search window
    mSystemSettings->setValue("searchRegex", mMain->searchDialog()->regex());
    mSystemSettings->setValue("searchCaseSens", mMain->searchDialog()->caseSens());
    mSystemSettings->setValue("searchWholeWords", mMain->searchDialog()->wholeWords());
    mSystemSettings->setValue("selectedScope", mMain->searchDialog()->selectedScope());

    mSystemSettings->endGroup();

    // tool-/menubar
    mSystemSettings->beginGroup("viewMenu");
    mSystemSettings->setValue("projectView", mMain->projectViewVisibility());
    mSystemSettings->setValue("outputView", mMain->outputViewVisibility());
    mSystemSettings->setValue("gamsArguments", mMain->gamsParameterEditor()->isEditorExtended());
    mSystemSettings->setValue("helpView", mMain->helpViewVisibility());
    mSystemSettings->setValue("encodingMIBs", mMain->encodingMIBsString());

    mSystemSettings->endGroup();

    // help
    mSystemSettings->beginGroup("helpView");
#ifdef QWEBENGINE
    QMultiMap<QString, QString> bookmarkMap(mMain->helpWidget()->getBookmarkMap());
    // remove all keys in the helpView group before begin writing them
    mSystemSettings->remove("");
    mSystemSettings->beginWriteArray("bookmarks");
    for (int i = 0; i < bookmarkMap.size(); i++) {
        mSystemSettings->setArrayIndex(i);
        mSystemSettings->setValue("location", bookmarkMap.keys().at(i));
        mSystemSettings->setValue("name", bookmarkMap.values().at(i));
    }
    mSystemSettings->endArray();
    mSystemSettings->setValue("zoomFactor", mMain->helpWidget()->getZoomFactor());
#endif
    mSystemSettings->endGroup();

    // history
    mSystemSettings->beginGroup("fileHistory");
    mSystemSettings->remove("lastOpenedFiles");
    mSystemSettings->beginWriteArray("lastOpenedFiles");
    for (int i = 0; i < mMain->history()->mLastOpenedFiles.length(); i++) {

        if (mMain->history()->mLastOpenedFiles.at(i) == "") break;
        mSystemSettings->setArrayIndex(i);
        mSystemSettings->setValue("file", mMain->history()->mLastOpenedFiles.at(i));
    }
    mSystemSettings->endArray();
    mSystemSettings->endGroup();

    mSystemSettings->beginGroup("json");
    QJsonObject jsonProject;
    mMain->projectRepo()->write(jsonProject);
    QJsonDocument saveDoc(jsonProject);
    mSystemSettings->setValue("projects", saveDoc.toJson(QJsonDocument::Compact));

    QJsonObject jsonTabs;
    mMain->writeTabs(jsonTabs);
    saveDoc = QJsonDocument(jsonTabs);
    mSystemSettings->setValue("openTabs", saveDoc.toJson(QJsonDocument::Compact));
    mSystemSettings->endGroup();
    mSystemSettings->sync();


    // User Settings
    mUserSettings->beginGroup("General");

    mUserSettings->setValue("defaultWorkspace", defaultWorkspace());
    mUserSettings->setValue("skipWelcome", skipWelcomePage());
    mUserSettings->setValue("restoreTabs", restoreTabs());
    mUserSettings->setValue("autosaveOnRun", autosaveOnRun());
    mUserSettings->setValue("openLst", openLst());
    mUserSettings->setValue("jumpToError", jumpToError());
    mUserSettings->setValue("setStudioOnTop",foregroundOnDemand());

    mUserSettings->endGroup();
    mUserSettings->beginGroup("Editor");

    mUserSettings->setValue("fontFamily", fontFamily());
    mUserSettings->setValue("fontSize", fontSize());
    mUserSettings->setValue("showLineNr", showLineNr());
    mUserSettings->setValue("tabSize", tabSize());
    mUserSettings->setValue("lineWrapEditor", lineWrapEditor());
    mUserSettings->setValue("lineWrapProcess", lineWrapProcess());
    mUserSettings->setValue("clearLog", clearLog());
    mUserSettings->setValue("wordUnderCursor", wordUnderCursor());
    mUserSettings->setValue("highlightCurrentLine", highlightCurrentLine());
    mUserSettings->setValue("autoIndent", autoIndent());
    mUserSettings->setValue("writeLog", writeLog());
    mUserSettings->setValue("nrLogBackups", nrLogBackups());
    mUserSettings->setValue("autoCloseBraces", autoCloseBraces());

    mUserSettings->endGroup();
    mUserSettings->beginGroup("Misc");

    mUserSettings->setValue("historySize", historySize());

    mUserSettings->setValue("solverOptionOverrideExisting", overridExistingOption());
    mUserSettings->setValue("solverOptionAddCommentAbove", addCommentDescriptionAboveOption());
    mUserSettings->setValue("solverOptionAddEOLComment", addEOLCommentDescriptionOption());
    mUserSettings->setValue("solverOptionDeleteCommentAbove", deleteAllCommentsAboveOption());

    mUserSettings->endGroup();
    mUserSettings->beginGroup("MIRO");

    mUserSettings->setValue("miroInstallationLocation", miroInstallationLocation());

    mUserSettings->endGroup();

    mUserSettings->sync();

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

QString findFixedFont()
{
    QFontDatabase fdb;
    QStringList list = fdb.families();
    for (int i = 0; i < list.size(); ++i) {
        if (fdb.isPrivateFamily(list.at(i)))
            continue;
        if (fdb.isFixedPitch(list.at(i)))
            return  list.at(i);
    }
    return QString();
}

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

QString Settings::miroInstallationLocation() const
{
    return mData.value("miroInstallationLocation").toString();
}

void Settings::setMiroInstallationLocation(const QString &location)
{
    mData.insert("miroInstallationLocation", location);
}

int Settings::historySize() const
{
    return mData.value("historySize").toInt();
}

void Settings::setHistorySize(int historySize)
{
    mData.insert("historySize", historySize);
}

bool Settings::overridExistingOption() const
{
    return mData.value("overrideExistingOption").toBool();
}

void Settings::setOverrideExistingOption(bool value)
{
    mData.insert("overrideExistingOption", value);
}

bool Settings::addCommentDescriptionAboveOption() const
{
    return mData.value("addCommentAboveOption").toBool();
}

void Settings::setAddCommentDescriptionAboveOption(bool value)
{
    mData.insert("addCommentAboveOption", value);
}

bool Settings::addEOLCommentDescriptionOption() const
{
    return mData.value("addEOLCommentOption").toBool();
}

void Settings::setAddEOLCommentDescriptionOption(bool value)
{
    mData.insert("addEOLCommentOption", value);
}

bool Settings::deleteAllCommentsAboveOption() const
{
    return mData.value("deleteCommentsAboveOption").toBool();
}

void Settings::setDeleteAllCommentsAboveOption(bool value)
{
    mData.insert("deleteCommentsAboveOption", value);
}

void Settings::restoreLastFilesUsed()
{
    mSystemSettings->beginGroup("fileHistory");
    mSystemSettings->beginReadArray("lastOpenedFiles");
    mMain->history()->mLastOpenedFiles.clear();
    for (int i = 0; i < historySize(); i++) {
        mSystemSettings->setArrayIndex(i);
        mMain->history()->mLastOpenedFiles.append(mSystemSettings->value("file").toString());
    }
    mSystemSettings->endArray();
    mSystemSettings->endGroup();
}

bool Settings::writeLog() const
{
    return mData.value("writeLog").toBool();
}

void Settings::setWriteLog(bool writeLog)
{
    mData.insert("writeLog", writeLog);
}

int Settings::nrLogBackups() const
{
    return mData.value("nrLogBackups").toInt();
}

void Settings::setNrLogBackups(int nrLogBackups)
{
    mData.insert("nrLogBackups", nrLogBackups);
}

bool Settings::autoCloseBraces() const
{
    return mData.value("autoCloseBraces").toBool();
}

void Settings::setAutoCloseBraces(bool autoCloseBraces)
{
    mData.insert("autoCloseBraces", autoCloseBraces);
}

int Settings::editableMaxSizeMB() const
{
    return mData.value("editableMaxSizeMB").toInt();
}

void Settings::setEditableMaxSizeMB(int editableMaxSizeMB)
{
    mData.insert("editableMaxSizeMB", editableMaxSizeMB);
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
        mSystemSettings->clear();
        mUserSettings->clear();
    } else {
        checkAndUpdateSettings();
    }

    loadUserIniSettings();
    loadViewStates();

    // the location for user model libraries is not modifyable right now
    // anyhow, it is part of StudioSettings since it might become modifyable in the future
    mData.insert("userModelLibraryDir", CommonPaths::userModelLibraryDir());
}

void Settings::checkAndUpdateSettings()
{
    if (!mSystemSettings->contains("settings/version")) return;
    QString settingsVersion = mSystemSettings->value("settings/version").toString();
    if (!isValidVersion(settingsVersion)) {
        DEB() << "Invalid version in settings: " << settingsVersion;
        return;
    }
    if (compareVersion(settingsVersion, "0.10.6") <= 0) {
        mUserSettings->beginGroup("Editor");
        mUserSettings->remove("editableMaxSizeMB");
        mUserSettings->endGroup();
    }
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

void Settings::setDefaultWorkspace(const QString &value)
{
    QDir workspace(value);

    if (!workspace.exists())
        workspace.mkpath(".");


    mData.insert("defaultWorkspace", value);
}

bool Settings::skipWelcomePage() const
{
    return mData.value("skipWelcomePage").toBool();
}

void Settings::setSkipWelcomePage(bool value)
{
    mData.insert("skipWelcomePage", value);
}

bool Settings::restoreTabs() const
{
    return mData.value("restoreTabs").toBool();
}

void Settings::setRestoreTabs(bool value)
{
    mData.insert("restoreTabs", value);
}

bool Settings::autosaveOnRun() const
{
    return mData.value("autosaveOnRun").toBool();
}

void Settings::setAutosaveOnRun(bool value)
{
    mData.insert("autosaveOnRun", value);
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
