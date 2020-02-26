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
#include "studiosettings.h"
#include "mainwindow.h"
#include "commonpaths.h"
#include "search/searchdialog.h"
#include "version.h"
#include "commandlineparser.h"
#include "scheme.h"

namespace gams {
namespace studio {

bool readJsonFile(QIODevice &device, QSettings::SettingsMap &map)
{
    QJsonDocument json = QJsonDocument::fromJson( device.readAll() );
    map = json.object().toVariantMap();
    return true;
}

bool writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map)
{
    device.write( QJsonDocument( QJsonObject::fromVariantMap( map ) ).toJson() );
    return true;
}

StudioSettings::StudioSettings(bool ignoreSettings, bool resetSettings, bool resetViews)
    : mIgnoreSettings(ignoreSettings),
      mResetSettings(resetSettings)
{
    mJsonFormat = QSettings::registerFormat("json", readJsonFile, writeJsonFile);
    init();

    mData.mIgnoreSettings = ignoreSettings;
    mData.mResetSettings = resetSettings;

    if (ignoreSettings && !mResetSettings) {
        mAppSettings = new QSettings();
        mUserSettings = new QSettings();
        mColorSettings = new QFile();
        Scheme::instance()->initDefault();
    }
    else if (mAppSettings == nullptr) {
        initSettingsFiles();
        Scheme::instance()->initDefault();
    }
    if (resetViews) resetViewSettings();

    QFileInfo file(mAppSettings->fileName());
    QDir location(file.path());
    for (const QString &fileName: location.entryList({"*.lock"})) {
        QFile f(location.path() +  "/" + fileName);
        f.remove();
    }
}

void StudioSettings::init()
{
    mVersions << "Beta";
    mVersions << "0.14.3";

    QDir dir = CommonPaths::settingsDir();
    if (!dir.exists()) {
        dir.mkpath(dir.path());
        initFromPrevious();
    }

}

void StudioSettings::initFromPrevious()
{
    QDir dir = CommonPaths::settingsDir();
    int currentVersion = mVersions.size()-1;
    int recentVersion = currentVersion;
    QStringList files = dir.entryList();
    while (!files.contains(dir.filePath(QString("uistates%1.json").arg(recentVersion)))) {
        --recentVersion;
        if (!recentVersion) break;
    }
}


StudioSettings::~StudioSettings()
{
    if (mAppSettings) {
        delete mAppSettings;
        mAppSettings = nullptr;
    }

    if (mUserSettings) {
        delete mUserSettings;
        mUserSettings = nullptr;
    }

    if (mColorSettings) {
        if (mColorSettings->isOpen()) {
            mColorSettings->flush();
            mColorSettings->close();
        }
        delete mColorSettings;
        mColorSettings = nullptr;
    }
}

void StudioSettings::initSettingsFiles()
{
    mAppSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                 GAMS_ORGANIZATION_STR, "uistates");
    mUserSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                  GAMS_ORGANIZATION_STR, "usersettings");
    QString path = QFileInfo(mUserSettings->fileName()).path();
    mColorSettings = new QFile(path+"/colorsettings.json");
}

void StudioSettings::resetSettings()
{
    initSettingsFiles();
    Scheme::instance()->initDefault();
    mAppSettings->sync();
    mUserSettings->sync();
    // TODO(JM) temporarily deactivated
//    writeScheme();
}

void StudioSettings::resetViewSettings()
{
    mAppSettings->beginGroup("mainWindow");
    mAppSettings->setValue("size", QSize(1024, 768));
    mAppSettings->setValue("pos", QPoint());
    mAppSettings->setValue("windowState", QByteArray());
    mAppSettings->endGroup();

    mAppSettings->beginGroup("viewMenu");
    mAppSettings->setValue("projectView", true);
    mAppSettings->setValue("outputView", true);
    mAppSettings->setValue("helpView", false);
    mAppSettings->setValue("optionEditor", false);
    mAppSettings->endGroup();

    mAppSettings->sync();
}

bool StudioSettings::resetSettingsSwitch()
{
    return mResetSettings;
}

void StudioSettings::saveSettings(MainWindow *main)
{
    // return directly only if settings are ignored and not resettet
    if (mIgnoreSettings && !mResetSettings)
        return;

    if (mAppSettings == nullptr) {
        qDebug() << "ERROR: settings file missing.";
        return;
    }
    mAppSettings->beginGroup("settings");
    mAppSettings->setValue("version", QString(STUDIO_VERSION));
    mAppSettings->endGroup();

    // Main Application Settings
    // main window
    mAppSettings->beginGroup("mainWindow");
    mAppSettings->setValue("size", main->size());
    mAppSettings->setValue("pos", main->pos());
    mAppSettings->setValue("maximized", main->isMaximized());
    mAppSettings->setValue("windowState", main->saveState());

    // search window
    mAppSettings->setValue("searchRegex", main->searchDialog()->regex());
    mAppSettings->setValue("searchCaseSens", main->searchDialog()->caseSens());
    mAppSettings->setValue("searchWholeWords", main->searchDialog()->wholeWords());
    mAppSettings->setValue("selectedScope", main->searchDialog()->selectedScope());

    mAppSettings->endGroup();

    // tool-/menubar
    mAppSettings->beginGroup("viewMenu");
    mAppSettings->setValue("projectView", main->projectViewVisibility());
    mAppSettings->setValue("outputView", main->outputViewVisibility());
    mAppSettings->setValue("gamsArguments", main->gamsParameterEditor()->isEditorExtended());
    mAppSettings->setValue("helpView", main->helpViewVisibility());
    mAppSettings->setValue("encodingMIBs", main->encodingMIBsString());

    mAppSettings->endGroup();

    // help
    mAppSettings->beginGroup("helpView");
#ifdef QWEBENGINE
    QMultiMap<QString, QString> bookmarkMap(main->helpWidget()->getBookmarkMap());
    // remove all keys in the helpView group before begin writing them
    mAppSettings->remove("");
    mAppSettings->beginWriteArray("bookmarks");
    for (int i = 0; i < bookmarkMap.size(); i++) {
        mAppSettings->setArrayIndex(i);
        mAppSettings->setValue("location", bookmarkMap.keys().at(i));
        mAppSettings->setValue("name", bookmarkMap.values().at(i));
    }
    mAppSettings->endArray();
    mAppSettings->setValue("zoomFactor", main->helpWidget()->getZoomFactor());
#endif
    mAppSettings->endGroup();

    // history
    mAppSettings->beginGroup("fileHistory");
    mAppSettings->remove("lastOpenedFiles");
    mAppSettings->beginWriteArray("lastOpenedFiles");
    for (int i = 0; i < main->history()->mLastOpenedFiles.length(); i++) {

        if (main->history()->mLastOpenedFiles.at(i) == "") break;
        mAppSettings->setArrayIndex(i);
        mAppSettings->setValue("file", main->history()->mLastOpenedFiles.at(i));
    }
    mAppSettings->endArray();
    mAppSettings->endGroup();

    mAppSettings->beginGroup("json");
    QJsonObject jsonProject;
    main->projectRepo()->write(jsonProject);
    QJsonDocument saveDoc(jsonProject);
    mAppSettings->setValue("projects", saveDoc.toJson(QJsonDocument::Compact));

    QJsonObject jsonTabs;
    main->writeTabs(jsonTabs);
    saveDoc = QJsonDocument(jsonTabs);
    mAppSettings->setValue("openTabs", saveDoc.toJson(QJsonDocument::Compact));
    mAppSettings->endGroup();
    mAppSettings->sync();


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

bool StudioSettings::writeScheme()
{
    if (mColorSettings && mColorSettings->open(QIODevice::WriteOnly)) {
        QString jsonColors = Scheme::instance()->exportJsonColorSchemes();
        mColorSettings->write(jsonColors.toLatin1().data(), jsonColors.toLatin1().length());
        mColorSettings->flush();
        mColorSettings->close();
        return true;
    }
    return false;
}

void StudioSettings::loadViewStates(MainWindow *main)
{
    mAppSettings->beginGroup("settings");
    mAppSettings->value("version").toString();
    mAppSettings->endGroup();

    // main window
    mAppSettings->beginGroup("mainWindow");
    bool maximized = mAppSettings->value("maximized", false).toBool();
    if (maximized) {
        main->setWindowState(Qt::WindowMaximized);
    } else {
        main->resize(mAppSettings->value("size", QSize(1024, 768)).toSize());
        main->move(mAppSettings->value("pos", QPoint(100, 100)).toPoint());
    }
    main->restoreState(mAppSettings->value("windowState").toByteArray());
    main->ensureInScreen();

    setSearchUseRegex(mAppSettings->value("searchRegex", false).toBool());
    setSearchCaseSens(mAppSettings->value("searchCaseSens", false).toBool());
    setSearchWholeWords(mAppSettings->value("searchWholeWords", false).toBool());
    setSelectedScopeIndex(mAppSettings->value("selectedScope", 0).toInt());

    mAppSettings->endGroup();

    // tool-/menubar
    mAppSettings->beginGroup("viewMenu");
    main->setProjectViewVisibility(mAppSettings->value("projectView", true).toBool());
    main->setOutputViewVisibility(mAppSettings->value("outputView", false).toBool());
    main->setExtendedEditorVisibility(mAppSettings->value("gamsArguments", false).toBool());
    main->setHelpViewVisibility(mAppSettings->value("helpView", false).toBool());
    main->setEncodingMIBs(mAppSettings->value("encodingMIBs", "106,0,4,17,2025").toString());

    mAppSettings->endGroup();

    // help
    mAppSettings->beginGroup("helpView");
#ifdef QWEBENGINE
    QMultiMap<QString, QString> bookmarkMap;
    int mapsize = mAppSettings->beginReadArray("bookmarks");
    for (int i = 0; i < mapsize; i++) {
        mAppSettings->setArrayIndex(i);
        bookmarkMap.insert(mAppSettings->value("location").toString(),
                           mAppSettings->value("name").toString());
    }
    mAppSettings->endArray();

    main->helpWidget()->setBookmarkMap(bookmarkMap);
    if (mAppSettings->value("zoomFactor") > 0.0)
        main->helpWidget()->setZoomFactor(mAppSettings->value("zoomFactor").toReal());
    else
        main->helpWidget()->setZoomFactor(1.0);
#endif
    mAppSettings->endGroup();
}

bool StudioSettings::isValidVersion(QString currentVersion)
{
    for (const QChar &c: currentVersion)
        if (c != '.' && (c < '0' || c > '9')) return false;
    QStringList verList = currentVersion.split('.');
    if (verList.size() < 2) return false;
    for (const QString &s: verList)
        if (s.isEmpty()) return false;
    return true;
}

int StudioSettings::compareVersion(QString currentVersion, QString otherVersion)
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

void StudioSettings::loadUserSettings()
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

    // TODO(JM) temporarily deactivated: reactivate
//    readScheme();
}

void StudioSettings::readScheme()
{
    Scheme::instance()->initDefault();

    if (mColorSettings && mColorSettings->open(QIODevice::ReadOnly)) {
        QByteArray jsonColors = mColorSettings->readAll();
        Scheme::instance()->importJsonColorSchemes(jsonColors);
        mColorSettings->close();
    }
}

QString StudioSettings::miroInstallationLocation() const
{
    return mData.miroInstallationLocation;
}

void StudioSettings::setMiroInstallationLocation(const QString &location)
{
    mData.miroInstallationLocation = location;
}

int StudioSettings::historySize() const
{
    return mData.historySize;
}

void StudioSettings::setHistorySize(int historySize)
{
    mData.historySize = historySize;
}

bool StudioSettings::overridExistingOption() const
{
    return mData.overrideExistingOption;
}

void StudioSettings::setOverrideExistingOption(bool value)
{
    mData.overrideExistingOption = value;
}

bool StudioSettings::addCommentDescriptionAboveOption() const
{
    return mData.addCommentAboveOption;
}

void StudioSettings::setAddCommentDescriptionAboveOption(bool value)
{
    mData.addCommentAboveOption = value;
}

bool StudioSettings::addEOLCommentDescriptionOption() const
{
    return mData.addEOLCommentOption;
}

void StudioSettings::setAddEOLCommentDescriptionOption(bool value)
{
    mData.addEOLCommentOption = value;
}

bool StudioSettings::deleteAllCommentsAboveOption() const
{
    return mData.deleteCommentsAboveOption;
}

void StudioSettings::setDeleteAllCommentsAboveOption(bool value)
{
    mData.deleteCommentsAboveOption = value;
}

void StudioSettings::restoreLastFilesUsed(MainWindow *main)
{
    mAppSettings->beginGroup("fileHistory");
    mAppSettings->beginReadArray("lastOpenedFiles");
    main->history()->mLastOpenedFiles.clear();
    for (int i = 0; i < historySize(); i++) {
        mAppSettings->setArrayIndex(i);
        main->history()->mLastOpenedFiles.append(mAppSettings->value("file").toString());
    }
    mAppSettings->endArray();
    mAppSettings->endGroup();
}

bool StudioSettings::writeLog() const
{
    return mData.writeLog;
}

void StudioSettings::setWriteLog(bool writeLog)
{
    mData.writeLog = writeLog;
}

int StudioSettings::nrLogBackups() const
{
    return mData.nrLogBackups;
}

void StudioSettings::setNrLogBackups(int nrLogBackups)
{
    mData.nrLogBackups = nrLogBackups;
}

bool StudioSettings::autoCloseBraces() const
{
    return mData.autoCloseBraces;
}

void StudioSettings::setAutoCloseBraces(bool autoCloseBraces)
{
    mData.autoCloseBraces = autoCloseBraces;
}

int StudioSettings::editableMaxSizeMB() const
{
    return mData.editableMaxSizeMB;
}

void StudioSettings::setEditableMaxSizeMB(int editableMaxSizeMB)
{
    mData.editableMaxSizeMB = editableMaxSizeMB;
}

bool StudioSettings::restoreTabsAndProjects(MainWindow *main)
{
    bool res = true;
    mAppSettings->beginGroup("json");
    QByteArray saveData = mAppSettings->value("projects", "").toByteArray();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    main->projectRepo()->read(loadDoc.object());

    if (restoreTabs()) {
        saveData = mAppSettings->value("openTabs", "").toByteArray();
        loadDoc = QJsonDocument::fromJson(saveData);
        res = main->readTabs(loadDoc.object());
    }
    mAppSettings->endGroup();
    return res;
}

void StudioSettings::loadSettings(MainWindow *main)
{
    if (mResetSettings) {
        mAppSettings->clear();
        mUserSettings->clear();
    } else {
        checkAndUpdateSettings();
    }

    loadUserSettings();
    loadViewStates(main);

    // the location for user model libraries is not modifyable right now
    // anyhow, it is part of StudioSettings since it might become modifyable in the future
    mData.userModelLibraryDir = CommonPaths::userModelLibraryDir();
}

void StudioSettings::checkAndUpdateSettings()
{
    if (!mAppSettings->contains("settings/version")) return;
    QString settingsVersion = mAppSettings->value("settings/version").toString();
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

void StudioSettings::importSettings(const QString &path, MainWindow *main)
{
    QFile backupFile(path);
    QFile settingsFile(mUserSettings->fileName());

    settingsFile.remove(); // remove old file
    backupFile.copy(settingsFile.fileName()); // import new file
    resetSettings();
    loadSettings(main);
}

QString StudioSettings::defaultWorkspace() const
{
    return mData.defaultWorkspace;
}

void StudioSettings::setDefaultWorkspace(const QString &value)
{
    QDir workspace(value);

    if (!workspace.exists())
        workspace.mkpath(".");

    mData.defaultWorkspace = value;
}

bool StudioSettings::skipWelcomePage() const
{
    return mData.skipWelcomePage;
}

void StudioSettings::setSkipWelcomePage(bool value)
{
    mData.skipWelcomePage = value;
}

bool StudioSettings::restoreTabs() const
{
    return mData.restoreTabs;
}

void StudioSettings::setRestoreTabs(bool value)
{
    mData.restoreTabs = value;
}

bool StudioSettings::autosaveOnRun() const
{
    return mData.autosaveOnRun;
}

void StudioSettings::setAutosaveOnRun(bool value)
{
    mData.autosaveOnRun = value;
}

bool StudioSettings::foregroundOnDemand() const
{
    return mData.foregroundOnDemand;
}

void StudioSettings::setForegroundOnDemand(bool value)
{
    mData.foregroundOnDemand = value;
}

bool StudioSettings::openLst() const
{
    return mData.openLst;
}

void StudioSettings::setOpenLst(bool value)
{
    mData.openLst = value;
}

bool StudioSettings::jumpToError() const
{
    return mData.jumpToError;
}

void StudioSettings::setJumpToError(bool value)
{
    mData.jumpToError = value;
}

int StudioSettings::fontSize() const
{
    return mData.fontSize;
}

void StudioSettings::setFontSize(int value)
{
    mData.fontSize = value;
}

bool StudioSettings::showLineNr() const
{
    return mData.showLineNr;
}

void StudioSettings::setShowLineNr(bool value)
{
    mData.showLineNr = value;
}

int StudioSettings::tabSize() const
{
    return mData.tabSize;
}

void StudioSettings::setTabSize(int value)
{
    mData.tabSize = value;
}

bool StudioSettings::lineWrapEditor() const
{
    return mData.lineWrapEditor;
}

void StudioSettings::setLineWrapEditor(bool value)
{
    mData.lineWrapEditor = value;
}

bool StudioSettings::lineWrapProcess() const
{
    return mData.lineWrapProcess;
}

void StudioSettings::setLineWrapProcess(bool value)
{
    mData.lineWrapProcess = value;
}

QString StudioSettings::fontFamily() const
{
    return mData.fontFamily;
}

void StudioSettings::setFontFamily(const QString &value)
{
    mData.fontFamily = value;
}

bool StudioSettings::clearLog() const
{
    return mData.clearLog;
}

void StudioSettings::setClearLog(bool value)
{
    mData.clearLog = value;
}

bool StudioSettings::searchUseRegex() const
{
    return mData.searchUseRegex;
}

void StudioSettings::setSearchUseRegex(bool searchUseRegex)
{
    mData.searchUseRegex = searchUseRegex;
}

bool StudioSettings::searchCaseSens() const
{
    return mData.searchCaseSens;
}

void StudioSettings::setSearchCaseSens(bool searchCaseSens)
{
    mData.searchCaseSens = searchCaseSens;
}

bool StudioSettings::searchWholeWords() const
{
    return mData.searchWholeWords;
}

void StudioSettings::setSearchWholeWords(bool searchWholeWords)
{
    mData.searchWholeWords = searchWholeWords;
}

int StudioSettings::selectedScopeIndex() const
{
    return mData.selectedScopeIndex;
}

void StudioSettings::setSelectedScopeIndex(int selectedScopeIndex)
{
    mData.selectedScopeIndex = selectedScopeIndex;
}

bool StudioSettings::wordUnderCursor() const
{
    return mData.wordUnderCursor;
}

void StudioSettings::setWordUnderCursor(bool wordUnderCursor)
{
    mData.wordUnderCursor = wordUnderCursor;
}

QString StudioSettings::userModelLibraryDir() const
{
    return mData.userModelLibraryDir;
}

bool StudioSettings::highlightCurrentLine() const
{
    return mData.highlightCurrentLine;
}

void StudioSettings::setHighlightCurrentLine(bool highlightCurrentLine)
{
    mData.highlightCurrentLine = highlightCurrentLine;
}

bool StudioSettings::autoIndent() const
{
    return mData.autoIndent;
}

void StudioSettings::setAutoIndent(bool autoIndent)
{
    mData.autoIndent = autoIndent;
}

void StudioSettings::exportSettings(const QString &path)
{
    QFile originFile(mUserSettings->fileName());
    originFile.copy(path);
}

}
}
