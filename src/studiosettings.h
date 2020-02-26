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
#include <QSettings>

class QSettings;
class QFile;

namespace gams {
namespace studio {

class MainWindow;

struct SettingsData {
    bool mIgnoreSettings = false;
    bool mResetSettings = false;

    // general settings page
    QString defaultWorkspace;
    bool skipWelcomePage;
    bool restoreTabs;
    bool autosaveOnRun;
    bool openLst;
    bool foregroundOnDemand;
    bool jumpToError;

    // editor settings page
    QString fontFamily;
    int fontSize;
    bool showLineNr;
    int tabSize;
    bool lineWrapEditor;
    bool lineWrapProcess;
    bool clearLog;
    bool wordUnderCursor;
    bool highlightCurrentLine;
    bool autoIndent;
    bool writeLog;
    int nrLogBackups;
    bool autoCloseBraces;
    int editableMaxSizeMB;

    // MIRO settings page
    QString miroInstallationLocation;

    // misc settings page
    int historySize;

    // solver option editor settings
    bool overrideExistingOption = true;
    bool addCommentAboveOption = false;
    bool addEOLCommentOption = false;
    bool deleteCommentsAboveOption = false;

    // search widget
    bool searchUseRegex;
    bool searchCaseSens;
    bool searchWholeWords;
    int selectedScopeIndex;

    // user model library directory
    QString userModelLibraryDir;
};

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

private:
    QSettings *mAppSettings = nullptr;
    QSettings *mUserSettings = nullptr;
    QFile *mColorSettings = nullptr;
    SettingsData mData;
    int mJsonFormat;

    bool mIgnoreSettings = false;
    bool mResetSettings = false;
    QStringList mVersions;

private:
    void init();
    void initFromPrevious();

    void checkAndUpdateSettings();
    void initSettingsFiles();
    void loadViewStates(MainWindow *main);
    bool isValidVersion(QString currentVersion);
    int compareVersion(QString currentVersion, QString otherVersion);
};

}
}

#endif // STUDIOSETTINGS_H
