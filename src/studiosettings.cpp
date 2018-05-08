/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "studiosettings.h"
#include "mainwindow.h"
#include "gamspaths.h"
#include "searchwidget.h"

namespace gams {
namespace studio {

StudioSettings::StudioSettings(bool ignoreSettings, bool resetSettings, bool resetViews)
    : mIgnoreSettings(ignoreSettings),
      mResetSettings(resetSettings)
{
    if (ignoreSettings && !mResetSettings) {
        mAppSettings = new QSettings();
        mUserSettings = new QSettings();
    }
    else if (mAppSettings == nullptr) {
        initSettingsFiles();
    }
    if (resetViews)
        resetViewSettings();
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
}

void StudioSettings::initSettingsFiles()
{
    mAppSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "GAMS", "uistates");
    mUserSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "GAMS", "usersettings");
}

void StudioSettings::resetSettings()
{
    initSettingsFiles();
    mAppSettings->sync();
    mUserSettings->sync();
}

void StudioSettings::resetViewSettings()
{
    mAppSettings->beginGroup("mainWindow");
    mAppSettings->setValue("size", QSize(1000, 700));
    mAppSettings->setValue("pos", QPoint());
    mAppSettings->setValue("windowState", QByteArray());
    mAppSettings->endGroup();

    mAppSettings->beginGroup("viewMenu");
    mAppSettings->setValue("projectView", true);
    mAppSettings->setValue("outputView", true);
    mAppSettings->setValue("helpView", false);
    mAppSettings->setValue("optionView", true);
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
    // Main Application Settings
    // main window
    mAppSettings->beginGroup("mainWindow");
    mAppSettings->setValue("size", main->size());
    mAppSettings->setValue("pos", main->pos());
    mAppSettings->setValue("windowState", main->saveState());

    // search window
    mAppSettings->setValue("searchRegex", main->searchWidget()->regex());
    mAppSettings->setValue("searchCaseSens", main->searchWidget()->caseSens());
    mAppSettings->setValue("searchWholeWords", main->searchWidget()->wholeWords());
    mAppSettings->setValue("selectedScope", main->searchWidget()->selectedScope());

    mAppSettings->endGroup();

    // tool-/menubar
    mAppSettings->beginGroup("viewMenu");
    mAppSettings->setValue("projectView", main->projectViewVisibility());
    mAppSettings->setValue("outputView", main->outputViewVisibility());
    mAppSettings->setValue("helpView", main->helpViewVisibility());
    mAppSettings->setValue("optionView", main->optionEditorVisibility());
    mAppSettings->setValue("optionEditor", main->isOptionDefinitionChecked());
    mAppSettings->setValue("encodingMIBs", main->encodingMIBsString());

    mAppSettings->endGroup();

    // help
    mAppSettings->beginGroup("helpView");
    QMultiMap<QString, QString> bookmarkMap(main->getDockHelpView()->getBookmarkMap());
    // remove all keys in the helpView group before begin writing them
    mAppSettings->remove("");
    mAppSettings->beginWriteArray("bookmarks");
    for (int i = 0; i < bookmarkMap.size(); i++) {
        mAppSettings->setArrayIndex(i);
        mAppSettings->setValue("location", bookmarkMap.keys().at(i));
        mAppSettings->setValue("name", bookmarkMap.values().at(i));
    }
    mAppSettings->endArray();
    mAppSettings->setValue("zoomFactor", main->getDockHelpView()->getZoomFactor());
    mAppSettings->endGroup();

    // history
    mAppSettings->beginGroup("fileHistory");
    mAppSettings->beginWriteArray("lastOpenedFiles");
    for (int i = 0; i < main->history()->lastOpenedFiles.length(); i++) {

        if (main->history()->lastOpenedFiles.at(i) == "") break;
        mAppSettings->setArrayIndex(i);
        mAppSettings->setValue("file", main->history()->lastOpenedFiles.at(i));
    }
    mAppSettings->endArray();

    QMap<QString, QStringList> map(main->commandLineHistory()->allHistory());
    mAppSettings->beginWriteArray("commandLineOptions");
    for (int i = 0; i < map.size(); i++) {
        mAppSettings->setArrayIndex(i);
        mAppSettings->setValue("path", map.keys().at(i));
        mAppSettings->setValue("opt", map.values().at(i));
    }
    mAppSettings->endArray();

    QJsonObject jsonProject;
    main->fileRepository()->write(jsonProject);
    QJsonDocument saveDoc(jsonProject);
    mAppSettings->setValue("projects", saveDoc.toJson(QJsonDocument::Compact));

    QJsonObject jsonTabs;
    main->writeTabs(jsonTabs);
    saveDoc = QJsonDocument(jsonTabs);
    mAppSettings->setValue("openTabs", saveDoc.toJson(QJsonDocument::Compact));

    mAppSettings->endGroup();

    // User Settings
    mUserSettings->beginGroup("General");

    mUserSettings->setValue("defaultWorkspace", defaultWorkspace());
    mUserSettings->setValue("skipWelcome", skipWelcomePage());
    mUserSettings->setValue("restoreTabs", restoreTabs());
    mUserSettings->setValue("autosaveOnRun", autosaveOnRun());
    mUserSettings->setValue("openLst", openLst());
    mUserSettings->setValue("jumpToError", jumpToError());

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

    mUserSettings->endGroup();
    mUserSettings->beginGroup("Misc");

    mUserSettings->setValue("historySize", historySize());

    mUserSettings->endGroup();

    mUserSettings->sync();
    mAppSettings->sync();
}

void StudioSettings::loadAppSettings(MainWindow *main)
{
    // main window
    mAppSettings->beginGroup("mainWindow");
    main->resize(mAppSettings->value("size", QSize(1000, 700)).toSize());
    main->move(mAppSettings->value("pos", QPoint(100, 100)).toPoint());
    main->restoreState(mAppSettings->value("windowState").toByteArray());

    setSearchUseRegex(mAppSettings->value("searchRegex", false).toBool());
    setSearchCaseSens(mAppSettings->value("searchCaseSens", false).toBool());
    setSearchWholeWords(mAppSettings->value("searchWholeWords", false).toBool());
    setSelectedScopeIndex(mAppSettings->value("selectedScope", 0).toInt());

    mAppSettings->endGroup();

    // tool-/menubar
    mAppSettings->beginGroup("viewMenu");
    main->setProjectViewVisibility(mAppSettings->value("projectView", true).toBool());
    main->setOutputViewVisibility(mAppSettings->value("outputView", true).toBool());
    main->setHelpViewVisibility(mAppSettings->value("helpView", false).toBool());
    main->setOptionEditorVisibility(mAppSettings->value("optionView", true).toBool());
    main->checkOptionDefinition(mAppSettings->value("optionEditor", false).toBool());
    main->setEncodingMIBs(mAppSettings->value("encodingMIBs", "106,0,4,17,2025").toString());

    mAppSettings->endGroup();

    // help
    mAppSettings->beginGroup("helpView");
    QMultiMap<QString, QString> bookmarkMap;
    int mapsize = mAppSettings->beginReadArray("bookmarks");
    for (int i = 0; i < mapsize; i++) {
        mAppSettings->setArrayIndex(i);
        bookmarkMap.insert(mAppSettings->value("location").toString(),
                           mAppSettings->value("name").toString());
    }
    mAppSettings->endArray();
    main->getDockHelpView()->setBookmarkMap(bookmarkMap);
    if (mAppSettings->value("zoomFactor") > 0.0)
        main->getDockHelpView()->setZoomFactor(mAppSettings->value("zoomFactor").toReal());
    else
        main->getDockHelpView()->setZoomFactor(1.0);
    mAppSettings->endGroup();

    mAppSettings->beginGroup("fileHistory");

    mAppSettings->beginReadArray("lastOpenedFiles");
    for (int i = 0; i < historySize(); i++) {
        mAppSettings->setArrayIndex(i);
        main->history()->lastOpenedFiles.append(mAppSettings->value("file").toString());
    }
    mAppSettings->endArray();

    QMap<QString, QStringList> map;
    int size = mAppSettings->beginReadArray("commandLineOptions");
    for (int i = 0; i < size; i++) {
        mAppSettings->setArrayIndex(i);
        map.insert(mAppSettings->value("path").toString(),
                   mAppSettings->value("opt").toStringList());
    }
    mAppSettings->endArray();
    mAppSettings->endGroup();

    main->commandLineHistory()->setAllHistory(map);
}

void StudioSettings::loadUserSettings()
{
    mUserSettings->beginGroup("General");

    setDefaultWorkspace(mUserSettings->value("defaultWorkspace", GAMSPaths::defaultWorkingDir()).toString());
    setSkipWelcomePage(mUserSettings->value("skipWelcome", false).toBool());
    setRestoreTabs(mUserSettings->value("restoreTabs", true).toBool());
    setAutosaveOnRun(mUserSettings->value("autosaveOnRun", true).toBool());
    setOpenLst(mUserSettings->value("openLst", false).toBool());
    setJumpToError(mUserSettings->value("jumpToError", true).toBool());

    mUserSettings->endGroup();
    mUserSettings->beginGroup("Editor");

    QFont ff = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    setFontFamily(mUserSettings->value("fontFamily", ff.defaultFamily()).toString());
    setFontSize(mUserSettings->value("fontSize", 10).toInt());
    setShowLineNr(mUserSettings->value("showLineNr", true).toBool());
    setTabSize(mUserSettings->value("tabSize", 4).toInt());
    setLineWrapEditor(mUserSettings->value("lineWrapEditor", false).toBool());
    setLineWrapProcess(mUserSettings->value("lineWrapProcess", false).toBool());
    setClearLog(mUserSettings->value("clearLog", false).toBool());
    setWordUnderCursor(mUserSettings->value("wordUnderCursor", true).toBool());
    setHighlightCurrentLine(mUserSettings->value("highlightCurrentLine", true).toBool());
    setAutoIndent(mUserSettings->value("autoIndent", true).toBool());

    mUserSettings->endGroup();
    mUserSettings->beginGroup("Misc");

    setHistorySize(mUserSettings->value("historySize", 8).toInt());

    mUserSettings->endGroup();
}

int StudioSettings::historySize() const
{
    return mHistorySize;
}

void StudioSettings::setHistorySize(int historySize)
{
    mHistorySize = historySize;
}

void StudioSettings::restoreLastFilesUsed(MainWindow *main)
{
    mAppSettings->beginGroup("fileHistory");
    mAppSettings->beginReadArray("lastOpenedFiles");
    main->history()->lastOpenedFiles.clear();
    for (int i = 0; i < historySize(); i++) {
        mAppSettings->setArrayIndex(i);
        main->history()->lastOpenedFiles.append(mAppSettings->value("file").toString());
    }
    mAppSettings->endArray();
    mAppSettings->endGroup();
}

void StudioSettings::restoreTabsAndProjects(MainWindow *main)
{
    mAppSettings->beginGroup("fileHistory");
    QByteArray saveData = mAppSettings->value("projects", "").toByteArray();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    main->fileRepository()->read(loadDoc.object());

    if (restoreTabs()) {
        saveData = mAppSettings->value("openTabs", "").toByteArray();
        loadDoc = QJsonDocument::fromJson(saveData);
        main->readTabs(loadDoc.object());
    }
    mAppSettings->endGroup();
}

void StudioSettings::loadSettings(MainWindow *main)
{
    if (mResetSettings) {
        mAppSettings->clear();
        mUserSettings->clear();
    }

    loadUserSettings();
    loadAppSettings(main);

    // the location for user model libraries is not modifyable right now
    // anyhow, it is part of StudioSettings since it might become modifyable in the future
    mUserModelLibraryDir = GAMSPaths::userModelLibraryDir();
}

void StudioSettings::importSettings(const QString &path, MainWindow *main)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("You are about to overwrite your local settings. Are you sure?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int answer = msgBox.exec();

    if (answer == QMessageBox::Ok) {
        QFile backupFile(path);
        QFile settingsFile(mUserSettings->fileName());

        settingsFile.remove(); // remove old file
        backupFile.copy(settingsFile.fileName()); // import new file
        resetSettings();
        loadSettings(main);
    }
}

QString StudioSettings::defaultWorkspace() const
{
    return mDefaultWorkspace;
}

void StudioSettings::setDefaultWorkspace(const QString &value)
{
    QDir workspace(value);

    if (!workspace.exists())
        workspace.mkpath(".");

    mDefaultWorkspace = value;
}

bool StudioSettings::skipWelcomePage() const
{
    return mSkipWelcomePage;
}

void StudioSettings::setSkipWelcomePage(bool value)
{
    mSkipWelcomePage = value;
}

bool StudioSettings::restoreTabs() const
{
    return mRestoreTabs;
}

void StudioSettings::setRestoreTabs(bool value)
{
    mRestoreTabs = value;
}

bool StudioSettings::autosaveOnRun() const
{
    return mAutosaveOnRun;
}

void StudioSettings::setAutosaveOnRun(bool value)
{
    mAutosaveOnRun = value;
}

bool StudioSettings::openLst() const
{
    return mOpenLst;
}

void StudioSettings::setOpenLst(bool value)
{
    mOpenLst = value;
}

bool StudioSettings::jumpToError() const
{
    return mJumpToError;
}

void StudioSettings::setJumpToError(bool value)
{
    mJumpToError = value;
}

int StudioSettings::fontSize() const
{
    return mFontSize;
}

void StudioSettings::setFontSize(int value)
{
    mFontSize = value;
}

bool StudioSettings::showLineNr() const
{
    return mShowLineNr;
}

void StudioSettings::setShowLineNr(bool value)
{
    mShowLineNr = value;
}

int StudioSettings::tabSize() const
{
    return mTabSize;
}

void StudioSettings::setTabSize(int value)
{
    mTabSize = value;
}

bool StudioSettings::lineWrapEditor() const
{
    return mLineWrapEditor;
}

void StudioSettings::setLineWrapEditor(bool value)
{
    mLineWrapEditor = value;
}

bool StudioSettings::lineWrapProcess() const
{
    return mLineWrapProcess;
}

void StudioSettings::setLineWrapProcess(bool value)
{
    mLineWrapProcess = value;
}

QString StudioSettings::fontFamily() const
{
    return mFontFamily;
}

void StudioSettings::setFontFamily(const QString &value)
{
    mFontFamily = value;
}

bool StudioSettings::clearLog() const
{
    return mClearLog;
}

void StudioSettings::setClearLog(bool value)
{
    mClearLog = value;
}

bool StudioSettings::searchUseRegex() const
{
    return mSearchUseRegex;
}

void StudioSettings::setSearchUseRegex(bool searchUseRegex)
{
    mSearchUseRegex = searchUseRegex;
}

bool StudioSettings::searchCaseSens() const
{
    return mSearchCaseSens;
}

void StudioSettings::setSearchCaseSens(bool searchCaseSens)
{
    mSearchCaseSens = searchCaseSens;
}

bool StudioSettings::searchWholeWords() const
{
    return mSearchWholeWords;
}

void StudioSettings::setSearchWholeWords(bool searchWholeWords)
{
    mSearchWholeWords = searchWholeWords;
}

int StudioSettings::selectedScopeIndex() const
{
    return mSelectedScopeIndex;
}

void StudioSettings::setSelectedScopeIndex(int selectedScopeIndex)
{
    mSelectedScopeIndex = selectedScopeIndex;
}

bool StudioSettings::wordUnderCursor() const
{
    return mWordUnderCursor;
}

void StudioSettings::setWordUnderCursor(bool wordUnderCursor)
{
    mWordUnderCursor = wordUnderCursor;
}

QString StudioSettings::userModelLibraryDir() const
{
    return mUserModelLibraryDir;
}

bool StudioSettings::highlightCurrentLine() const
{
    return mHighlightCurrentLine;
}

void StudioSettings::setHighlightCurrentLine(bool highlightCurrentLine)
{
    mHighlightCurrentLine = highlightCurrentLine;
}

bool StudioSettings::autoIndent() const
{
    return mAutoIndent;
}

void StudioSettings::setAutoIndent(bool autoIndent)
{
    mAutoIndent = autoIndent;
}

void StudioSettings::exportSettings(const QString &path)
{
    QFile originFile(mUserSettings->fileName());
    originFile.copy(path);
}

}
}
