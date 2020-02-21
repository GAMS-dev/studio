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
#ifndef STUDIOSETTINGS_H
#define STUDIOSETTINGS_H

#include <QString>
#include <QColor>
#include <QHash>

class QSettings;
class QFile;

namespace gams {
namespace studio {

class MainWindow;

class StudioSettings
{

public:
    StudioSettings(bool ignoreSettings, bool resetSettings, bool resetViewSettings);
    ~StudioSettings();

    void loadSettings(MainWindow *main);
    void saveSettings(MainWindow *main);
    bool writeScheme();
    void readScheme();

    QString defaultWorkspace() const;
    void setDefaultWorkspace(const QString &value);

    bool skipWelcomePage() const;
    void setSkipWelcomePage(bool value);

    bool restoreTabs() const;
    void setRestoreTabs(bool value);

    bool autosaveOnRun() const;
    void setAutosaveOnRun(bool value);

    bool openLst() const;
    void setOpenLst(bool value);


    bool jumpToError() const;
    void setJumpToError(bool value);


    bool foregroundOnDemand() const;
    void setForegroundOnDemand(bool value);


    int fontSize() const;
    void setFontSize(int value);

    bool showLineNr() const;
    void setShowLineNr(bool value);

    bool replaceTabsWithSpaces() const;
    void setReplaceTabsWithSpaces(bool value);

    int tabSize() const;
    void setTabSize(int value);

    bool lineWrapEditor() const;
    void setLineWrapEditor(bool value);

    bool lineWrapProcess() const;
    void setLineWrapProcess(bool value);

    QString fontFamily() const;
    void setFontFamily(const QString &value);

    bool clearLog() const;
    void setClearLog(bool value);

    bool searchUseRegex() const;
    void setSearchUseRegex(bool searchUseRegex);

    bool searchCaseSens() const;
    void setSearchCaseSens(bool searchCaseSens);

    bool searchWholeWords() const;
    void setSearchWholeWords(bool searchWholeWords);

    int selectedScopeIndex() const;
    void setSelectedScopeIndex(int selectedScopeIndex);

    bool wordUnderCursor() const;
    void setWordUnderCursor(bool wordUnderCursor);
    QString userModelLibraryDir() const;

    bool highlightCurrentLine() const;
    void setHighlightCurrentLine(bool highlightCurrentLine);

    bool autoIndent() const;
    void setAutoIndent(bool autoIndent);

    void exportSettings(const QString &path);
    void importSettings(const QString &path, MainWindow *main);

    void loadUserSettings();
    void updateSettingsFiles();

    QString miroInstallationLocation() const;
    void setMiroInstallationLocation(const QString &location);

    int historySize() const;
    void setHistorySize(int historySize);

    bool overridExistingOption() const;
    void setOverrideExistingOption(bool value);

    bool addCommentDescriptionAboveOption() const;
    void setAddCommentDescriptionAboveOption(bool value);

    bool addEOLCommentDescriptionOption() const;
    void setAddEOLCommentDescriptionOption(bool value);

    bool deleteAllCommentsAboveOption() const;
    void setDeleteAllCommentsAboveOption(bool value);

    void resetSettings();
    bool resetSettingsSwitch();
    void resetViewSettings();

    bool restoreTabsAndProjects(MainWindow *main);
    void restoreLastFilesUsed(MainWindow *main);

    bool writeLog() const;
    void setWriteLog(bool writeLog);

    int nrLogBackups() const;
    void setNrLogBackups(int nrLogBackups);

    bool autoCloseBraces() const;
    void setAutoCloseBraces(bool autoCloseBraces);

    int editableMaxSizeMB() const;
    void setEditableMaxSizeMB(int editableMaxSizeMB);

    int colorSchemeIndex() const;
    void setColorSchemeIndex(int colorSchemeIndex);

private:
    QSettings *mAppSettings = nullptr;
    QSettings *mUserSettings = nullptr;
    QFile *mColorSettings = nullptr;
    bool mIgnoreSettings = false;
    bool mResetSettings = false;

    // general settings page
    int mColorSchemeIndex;
    QString mDefaultWorkspace;
    bool mSkipWelcomePage;
    bool mRestoreTabs;
    bool mAutosaveOnRun;
    bool mOpenLst;
    bool mForegroundOnDemand;
    bool mJumpToError;

    // editor settings page
    QString mFontFamily;
    int mFontSize;
    bool mShowLineNr;
    int mTabSize;
    bool mLineWrapEditor;
    bool mLineWrapProcess;
    bool mClearLog;
    bool mWordUnderCursor;
    bool mHighlightCurrentLine;
    bool mAutoIndent;
    bool mWriteLog;
    int mNrLogBackups;
    bool mAutoCloseBraces;
    int mEditableMaxSizeMB;

    // MIRO settings page
    QString mMiroInstallationLocation;

    // misc settings page
    int mHistorySize;

    // solver option editor settings
    bool mOverrideExistingOption = true;
    bool mAddCommentAboveOption = false;
    bool mAddEOLCommentOption = false;
    bool mDeleteCommentsAboveOption = false;

    // search widget
    bool mSearchUseRegex;
    bool mSearchCaseSens;
    bool mSearchWholeWords;
    int mSelectedScopeIndex;

    // user model library directory
    QString mUserModelLibraryDir;
    void checkAndUpdateSettings();
    void initSettingsFiles();
    void loadViewStates(MainWindow *main);
    bool isValidVersion(QString currentVersion);
    int compareVersion(QString currentVersion, QString otherVersion);
};

}
}

#endif // STUDIOSETTINGS_H
