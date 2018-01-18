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
#ifndef STUDIOSETTINGS_H
#define STUDIOSETTINGS_H

#include <QSettings>
#include <QDebug>
#include "mainwindow.h"

namespace gams {
namespace studio {

class StudioSettings
{

public:
    StudioSettings(MainWindow *main);
    ~StudioSettings();

    void loadSettings();
    void saveSettings();

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

    void updateEditorFont(QString fontFamily, int fontSize);
    void redrawEditors();

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

private:
    MainWindow *mMain = nullptr;
    QSettings *mAppSettings = nullptr;
    QSettings *mUserSettings = nullptr;

    // general
    QString mDefaultWorkspace;
    bool mSkipWelcomePage;
    bool mRestoreTabs;
    bool mAutosaveOnRun;
    bool mOpenLst;
    bool mJumpToError;

    // editor
    QString mFontFamily;
    int mFontSize;
    bool mShowLineNr;
    bool mReplaceTabsWithSpaces;
    int mTabSize;
    bool mLineWrapEditor;
    bool mLineWrapProcess;
    bool mClearLog;

    // search widget
    bool mSearchUseRegex;
    bool mSearchCaseSens;
    bool mSearchWholeWords;
    int mSelectedScopeIndex;
};

}
}

#endif // STUDIOSETTINGS_H
