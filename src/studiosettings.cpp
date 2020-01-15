/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include "studiosettings.h"
#include "mainwindow.h"
#include "commonpaths.h"
#include "search/searchdialog.h"
#include "version.h"
#include "commandlineparser.h"

namespace gams {
namespace studio {

StudioSettings::StudioSettings(bool ignoreSettings, bool resetSettings, bool resetViews)
    : mIgnoreSettings(ignoreSettings),
      mResetSettings(resetSettings)
{
    if (ignoreSettings && !mResetSettings) {
        mAppSettings = new QSettings();
        mUserSettings = new QSettings();
        initDefaultColors();
    }
    else if (mAppSettings == nullptr) {
        initSettingsFiles();
        initDefaultColors();
    }
    if (resetViews) resetViewSettings();

    QFileInfo file(mAppSettings->fileName());
    QDir location(file.path());
    for (const QString &fileName: location.entryList({"*.lock"})) {
        QFile f(location.path() +  "/" + fileName);
        f.remove();
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
}

void StudioSettings::initSettingsFiles()
{
    mAppSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                 GAMS_ORGANIZATION_STR, "uistates");
    mUserSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                  GAMS_ORGANIZATION_STR, "usersettings");
}

void StudioSettings::initDefaultColors()
{
    if (mColorSchemes.size() == 0) {
        mColorSchemes << QHash<QString, QColor>();
    }
    mColorSchemes[0].clear();
    mColorSchemes[0].insert("Edit.currentLineBg", QColor(255, 250, 170));
    mColorSchemes[0].insert("Edit.currentWordBg", QColor(Qt::lightGray));
    mColorSchemes[0].insert("Edit.matchesBg", QColor(Qt::green).lighter(160));
}

void StudioSettings::resetSettings()
{
    initSettingsFiles();
    initDefaultColors();
    mAppSettings->sync();
    mUserSettings->sync();
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
    mUserSettings->setValue("colorScheme", exportJsonColorSchemes());
    mUserSettings->setValue("colorSchemeIndex", colorSchemeIndex());

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
    importJsonColorSchemes(mUserSettings->value("colorScheme").toByteArray());
    setColorSchemeIndex(mUserSettings->value("colorSchemeIndex", 0).toInt());

    mUserSettings->endGroup();
    mUserSettings->beginGroup("Editor");

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    if (!font.fixedPitch()) {
        QString fontFamily = findFixedFont();
        font = QFont(fontFamily);
        if (fontFamily.isNull())
            DEB() << "No fixed font found on system. Using " << font.family();
        else
            DEB() << "Fixed font found: " << font.family();
    } else {
        DEB() << "Using fixed font of system: " << font.family();
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

QString StudioSettings::miroInstallationLocation() const
{
    return mMiroInstallationLocation;
}

void StudioSettings::setMiroInstallationLocation(const QString &location)
{
    mMiroInstallationLocation = location;
}

int StudioSettings::historySize() const
{
    return mHistorySize;
}

void StudioSettings::setHistorySize(int historySize)
{
    mHistorySize = historySize;
}

bool StudioSettings::overridExistingOption() const
{
    return mOverrideExistingOption;
}

void StudioSettings::setOverrideExistingOption(bool value)
{
    mOverrideExistingOption = value;
}

bool StudioSettings::addCommentDescriptionAboveOption() const
{
    return mAddCommentAboveOption;
}

void StudioSettings::setAddCommentDescriptionAboveOption(bool value)
{
    mAddCommentAboveOption = value;
}

bool StudioSettings::addEOLCommentDescriptionOption() const
{
    return mAddEOLCommentOption;
}

void StudioSettings::setAddEOLCommentDescriptionOption(bool value)
{
    mAddEOLCommentOption = value;
}

bool StudioSettings::deleteAllCommentsAboveOption() const
{
    return mDeleteCommentsAboveOption;
}

void StudioSettings::setDeleteAllCommentsAboveOption(bool value)
{
    mDeleteCommentsAboveOption = value;
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
    return mWriteLog;
}

void StudioSettings::setWriteLog(bool writeLog)
{
    mWriteLog = writeLog;
}

int StudioSettings::nrLogBackups() const
{
    return mNrLogBackups;
}

void StudioSettings::setNrLogBackups(int nrLogBackups)
{
    mNrLogBackups = nrLogBackups;
}

bool StudioSettings::autoCloseBraces() const
{
    return mAutoCloseBraces;
}

void StudioSettings::setAutoCloseBraces(bool autoCloseBraces)
{
    mAutoCloseBraces = autoCloseBraces;
}

int StudioSettings::editableMaxSizeMB() const
{
    return mEditableMaxSizeMB;
}

void StudioSettings::setEditableMaxSizeMB(int editableMaxSizeMB)
{
    mEditableMaxSizeMB = editableMaxSizeMB;
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
    mUserModelLibraryDir = CommonPaths::userModelLibraryDir();
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

bool StudioSettings::foregroundOnDemand() const
{
    return mForegroundOnDemand;
}

void StudioSettings::setForegroundOnDemand(bool value)
{
    mForegroundOnDemand = value;
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

int StudioSettings::colorSchemeIndex()
{
    return mColorSchemeIndex;
}

void StudioSettings::setColorSchemeIndex(int value)
{
    if (value >= mColorSchemes.size())
        value = mColorSchemes.size() -1;
    mColorSchemeIndex = value;
}

QHash<QString, QColor> &StudioSettings::colorScheme()
{
    return mColorSchemes[mColorSchemeIndex];
}

QByteArray StudioSettings::exportJsonColorSchemes()
{
    QJsonArray schemes;

    for (const QHash<QString, QColor> &scheme: mColorSchemes) {
        QJsonObject json;
        QJsonObject colorObject;
        for (QString key: scheme.keys()) {
            colorObject[key] = scheme.value(key).name();
        }
        json["colorScheme"] = colorObject;
        schemes.append(json);
    }

    QJsonDocument saveDoc = QJsonDocument(schemes);
    return saveDoc.toJson(QJsonDocument::Compact);
}

void StudioSettings::importJsonColorSchemes(const QByteArray &jsonData)
{
    QJsonArray schemes = QJsonDocument::fromJson(jsonData).array();
    if (mColorSchemes.size() < schemes.size())
        mColorSchemes.append(QHash<QString, QColor>());
    for (int i = 0; i < schemes.size(); ++i) {
        QJsonObject json = schemes[i].toObject();
        if (json.contains("colorScheme") && json["colorScheme"].isObject()) {
            QJsonObject colorObject = json["colorScheme"].toObject();
            for (QString key: colorObject.keys()) {
                mColorSchemes[i].insert(key, QColor(colorObject[key].toString()));
            }
        }
    }
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
