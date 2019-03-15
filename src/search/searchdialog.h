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
#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H
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
#include <QDialog>
#include "../mainwindow.h"

namespace Ui {
class SearchDialog;
}

namespace gams {
namespace studio {

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    enum SearchDirection {
        Forward = 0,
        Backward = 1
    };
    explicit SearchDialog(MainWindow *parent = nullptr);
    ~SearchDialog();

    bool regex();
    bool caseSens();
    bool wholeWords();
    QString searchTerm();
    QRegularExpression createRegex();

    int selectedScope();
    void setSelectedScope(int index);

    void findNext(SearchDialog::SearchDirection direction);
    void clearResults();
    void updateReplaceActionAvailability();

    void autofillSearchField();

    void clearSearch();
    void invalidateCache();

    SearchResultList* cachedResults();
    void setActiveEditWidget(QWidget *edit);

    ResultsView *resultsView() const;
    void setResultsView(ResultsView *resultsView);

    void updateSearchCache();

public slots:
    void on_searchNext();
    void on_searchPrev();
    void on_documentContentChanged(int from, int charsRemoved, int charsAdded);
    void finalUpdate();
    void intermediateUpdate();

protected slots:
    void returnPressed();
    void searchResume();

private slots:
    void on_btn_FindAll_clicked();
    void on_btn_Replace_clicked();
    void on_btn_ReplaceAll_clicked();
    void on_combo_scope_currentIndexChanged(int index);
    void on_btn_back_clicked();
    void on_btn_forward_clicked();
    void on_btn_clear_clicked();
    void on_combo_search_currentTextChanged(const QString);
    void on_cb_caseSens_stateChanged(int);
    void on_cb_wholeWords_stateChanged(int arg1);
    void on_cb_regex_stateChanged(int arg1);

signals:
    void startSearch();

protected:
    void showEvent(QShowEvent *event);
    void keyPressEvent(QKeyEvent *e);

private:
    enum SearchScope {
        ThisFile = 0,
        ThisGroup= 1,
        OpenTabs = 2,
        AllFiles = 3
    };

    enum SearchStatus {
        Searching = 0,
        NoResults = 1,
        Clear = 2
    };
    void simpleReplaceAll();
    void findInFiles(QMutex& mutex, QList<FileMeta *> fml, bool skipFilters = false);
    void findInGroup(QMutex& mutex);
    void findInOpenFiles(QMutex& mutex);
    void findInAllFiles(QMutex& mutex);
    void updateMatchAmount(int current = 0);
    void selectNextMatch(SearchDirection direction, bool second = false);
    void insertHistory();
    void searchParameterChanged();
    void findOnDisk(QRegularExpression searchRegex, FileMeta *fm, SearchResultList *matches);
    void findInDoc(QRegularExpression searchRegex, FileMeta *fm);
    void updateEditHighlighting();
    void setSearchOngoing(bool searching);
    void setSearchStatus(SearchStatus status);

private:
    Ui::SearchDialog *ui;
    MainWindow *mMain;
    QTextCursor mSelection;       // selected with find
    QTextCursor mLastSelection;   // last selection, as starting point for find next
    ResultsView *mResultsView = nullptr;
    SearchResultList *mCachedResults = nullptr;
    QWidget *mActiveEdit = nullptr;
    bool mHasChanged = true;
    bool mFirstReturn = false;
    TextView *mSplitSearchView = nullptr;
    QRegularExpression mSplitSearchRegEx;
    QTextDocument::FindFlags mSplitSearchFlags;
    bool mSplitSearchContinue = false;
    bool mShowResults = true;
    QFlags<QTextDocument::FindFlag> setFlags(SearchDirection direction);
    QThread mThread;
    bool mSearching = false;
    QMutex mMutex;
    void updateFindNextLabel(QTextCursor matchSelection);
};

}
}
#endif // SEARCHDIALOG_H
