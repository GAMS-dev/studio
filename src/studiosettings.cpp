/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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


StudioSettings::StudioSettings(MainWindow *main)
    : mMain(main)
{
}

void StudioSettings::saveSettings()
{
    if (mAppSettings == nullptr) {
        qDebug() << "ERROR: settings file missing.";
        return;
    }
    // Main Application Settings
    // window
    mAppSettings->beginGroup("mainWindow");
    mAppSettings->setValue("size", mMain->size());
    mAppSettings->setValue("pos", mMain->pos());
    mAppSettings->setValue("windowState", mMain->saveState());

    if (mMain->searchWidget()) {
        mAppSettings->setValue("searchRegex", mMain->searchWidget()->regex());
        mAppSettings->setValue("searchCaseSens", mMain->searchWidget()->caseSens());
        mAppSettings->setValue("searchWholeWords", mMain->searchWidget()->wholeWords());
        mAppSettings->setValue("selectedScope", mMain->searchWidget()->selectedScope());
    }
    mAppSettings->endGroup();

    // tool-/menubar
    mAppSettings->beginGroup("viewMenu");
    mAppSettings->setValue("projectView", mMain->projectViewVisibility());
    mAppSettings->setValue("outputView", mMain->outputViewVisibility());
    mAppSettings->setValue("optionEditor", mMain->optionEditorVisibility());

    mAppSettings->endGroup();

    // history
    mAppSettings->beginGroup("fileHistory");
    mAppSettings->beginWriteArray("lastOpenedFiles");
    for (int i = 0; i < mMain->history()->lastOpenedFiles.size(); i++) {
        mAppSettings->setArrayIndex(i);
        mAppSettings->setValue("file", mMain->history()->lastOpenedFiles.at(i));
    }
    mAppSettings->endArray();

    QMap<QString, QStringList> map(mMain->commandLineHistory()->allHistory());
    mAppSettings->beginWriteArray("commandLineOptions");
    for (int i = 0; i < map.size(); i++) {
        mAppSettings->setArrayIndex(i);
        mAppSettings->setValue("path", map.keys().at(i));
        mAppSettings->setValue("opt", map.values().at(i));
    }
    mAppSettings->endArray();

    QJsonObject json;
    mMain->fileRepository()->write(json);
    QJsonDocument saveDoc(json);
    mAppSettings->setValue("projects", saveDoc.toJson(QJsonDocument::Compact));
//    FileSystemContext* root = mMain->fileRepository()->treeModel()->serialize();
//    mAppSettings->endGroup();

    mAppSettings->beginWriteArray("openedTabs");
    QWidgetList editList = mMain->fileRepository()->editors();
    for (int i = 0; i < editList.size(); i++) {
        mAppSettings->setArrayIndex(i);
        FileContext *fc = mMain->fileRepository()->fileContext(editList.at(i));
        mAppSettings->setValue("location", fc->location());
    }

    mAppSettings->endArray();
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
    mUserSettings->setValue("replaceTabsWithSpaces", replaceTabsWithSpaces());
    mUserSettings->setValue("tabSize", tabSize());
    mUserSettings->setValue("lineWrapEditor", lineWrapEditor());
    mUserSettings->setValue("lineWrapProcess", lineWrapProcess());
    mUserSettings->setValue("clearLog", clearLog());
    mUserSettings->setValue("wordUnderCursor", wordUnderCursor());

    mUserSettings->endGroup();

    mAppSettings->sync();
}

void StudioSettings::loadSettings()
{
    if (mAppSettings == nullptr)
        mAppSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "GAMS", "uistates");

    // window
    mAppSettings->beginGroup("mainWindow");
    mMain->resize(mAppSettings->value("size", QSize(1024, 768)).toSize());
    mMain->move(mAppSettings->value("pos", QPoint(100, 100)).toPoint());
    mMain->restoreState(mAppSettings->value("windowState").toByteArray());

    setSearchUseRegex(mAppSettings->value("searchRegex", false).toBool());
    setSearchCaseSens(mAppSettings->value("searchCaseSens", false).toBool());
    setSearchWholeWords(mAppSettings->value("searchWholeWords", false).toBool());
    setSelectedScopeIndex(mAppSettings->value("selectedScope", 0).toInt());

    mAppSettings->endGroup();

    // tool-/menubar
    mAppSettings->beginGroup("viewMenu");
    mMain->setProjectViewVisibility(mAppSettings->value("projectView").toBool());
    mMain->setOutputViewVisibility(mAppSettings->value("outputView").toBool());
    mMain->setOptionEditorVisibility(mAppSettings->value("optionEditor").toBool());

    mAppSettings->endGroup();

    // history
    mAppSettings->beginGroup("fileHistory");

    mAppSettings->beginReadArray("lastOpenedFiles");
    for (int i = 0; i < mMain->history()->MAX_FILE_HISTORY; i++) {
        mAppSettings->setArrayIndex(i);
        mMain->history()->lastOpenedFiles.append(mAppSettings->value("file").toString());
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
    mMain->commandLineHistory()->setAllHistory(map);

    QByteArray saveData = mAppSettings->value("projects", "").toByteArray();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    mMain->fileRepository()->read(loadDoc.object());

    mAppSettings->endGroup();

    if (mUserSettings == nullptr)
        mUserSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "GAMS", "usersettings");

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
    setReplaceTabsWithSpaces(mUserSettings->value("replaceTabsWithSpaces", false).toBool());
    setTabSize(mUserSettings->value("tabSize", 4).toInt());
    setLineWrapEditor(mUserSettings->value("lineWrapEditor", false).toBool());
    setLineWrapProcess(mUserSettings->value("lineWrapProcess", false).toBool());
    setClearLog(mUserSettings->value("clearLog", false).toBool());
    setWordUnderCursor(mUserSettings->value("wordUnderCursor", true).toBool());

    mUserSettings->endGroup();

    if(!restoreTabs())
        return;

    mAppSettings->beginGroup("fileHistory");
    size = mAppSettings->beginReadArray("openedTabs");
    for (int i = 0; i < size; i++) {
        mAppSettings->setArrayIndex(i);
        QString value = mAppSettings->value("location").toString();
        if(QFileInfo(value).exists())
            mMain->openFile(value);
    }
    mAppSettings->endArray();
    mAppSettings->endGroup();

    // the location for user model libraries is not modifyable right now
    // anyhow, it is part of StudioSettings since it might become modifyable in the future
    mUserModelLibraryDir = GAMSPaths::userModelLibraryDir();
}

QString StudioSettings::defaultWorkspace() const
{
    return mDefaultWorkspace;
}

void StudioSettings::setDefaultWorkspace(const QString &value)
{
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

bool StudioSettings::replaceTabsWithSpaces() const
{
    return mReplaceTabsWithSpaces;
}

void StudioSettings::setReplaceTabsWithSpaces(bool value)
{
    mReplaceTabsWithSpaces = value;
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

void StudioSettings::updateEditorFont(QString fontFamily, int fontSize)
{
    QFont font(fontFamily, fontSize);
    foreach (QWidget* edit, mMain->openEditors()) {
        edit->setFont(font);
    }
}

void StudioSettings::redrawEditors()
{
    QPlainTextEdit::LineWrapMode wrapModeEditor;
    if(lineWrapEditor())
        wrapModeEditor = QPlainTextEdit::WidgetWidth;
    else
        wrapModeEditor = QPlainTextEdit::NoWrap;

    QWidgetList editList = mMain->fileRepository()->editors();
    for (int i = 0; i < editList.size(); i++) {
        QPlainTextEdit* ed = FileSystemContext::toPlainEdit(editList.at(i));
        if (ed) {
            ed->blockCountChanged(0); // force redraw for line number area
            ed->setLineWrapMode(wrapModeEditor);
        }
    }

    QPlainTextEdit::LineWrapMode wrapModeProcess;
    if(lineWrapProcess())
        wrapModeProcess = QPlainTextEdit::WidgetWidth;
    else
        wrapModeProcess = QPlainTextEdit::NoWrap;

    QList<QPlainTextEdit*> logList = mMain->openLogs();
    for (int i = 0; i < logList.size(); i++) {
        if (logList.at(i))
            logList.at(i)->setLineWrapMode(wrapModeProcess);
    }

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

}
}
