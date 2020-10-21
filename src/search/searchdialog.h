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
#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include "mainwindow.h"

namespace gams {
namespace studio {
namespace search {

namespace Ui {
class SearchDialog;
}

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

    void findNext(SearchDialog::SearchDirection direction, bool ignoreReadOnly = false);
    void clearResults();
    void updateReplaceActionAvailability();

    void autofillSearchField();

    void clearSearch();
    void invalidateCache();

    SearchResultList* results();

    ResultsView *resultsView() const;
    void setResultsView(ResultsView *resultsView);

    void updateSearchCache(bool ignoreReadOnly = false);

public slots:
    void on_searchNext();
    void on_searchPrev();
    void on_documentContentChanged(int from, int charsRemoved, int charsAdded);
    void finalUpdate();
    void intermediateUpdate();
    void updateNrMatches(int current = 0, int max = -1);

protected slots:
    void searchResume();

private slots:
    void on_btn_FindAll_clicked();
    void on_btn_Replace_clicked();
    void on_btn_ReplaceAll_clicked();
    void on_combo_scope_currentIndexChanged(int);
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
        Clear = 2,
        Replacing = 4
    };

    void replaceAll();
    void findInFiles(SearchResultList* collection, QList<FileMeta *> fml);
    QList<FileMeta*> getFilesByScope(bool ignoreReadOnly = false);
    void updateFindNextLabel(int lineNr, int colNr);
    void selectNextMatch(SearchDirection direction);
    void insertHistory();
    void searchParameterChanged();
    void findOnDisk(QRegularExpression searchRegex, FileMeta *fm, SearchResultList* collection);
    void findInDoc(FileMeta *fm, SearchResultList* collection);
    void updateEditHighlighting();
    void setSearchOngoing(bool searching);
    void setSearchStatus(SearchStatus status);
    int replaceOpened(FileMeta* fm, QRegularExpression regex, QString replaceTerm, QFlags<QTextDocument::FindFlag> flags);
    int replaceUnopened(FileMeta* fm, QRegularExpression regex, QString replaceTerm);

private:
    Ui::SearchDialog *ui;
    MainWindow *mMain;
    ResultsView *mResultsView = nullptr;
    SearchResultList *mCachedResults = nullptr;
    bool mHasChanged = true;
    TextView *mSplitSearchView = nullptr;
    QTextDocument::FindFlags mSplitSearchFlags;
    bool mSplitSearchContinue = false;
    bool mShowResults = true;
    bool mIsReplacing = false;
    bool mSuppressChangeEvent = false;
    QFlags<QTextDocument::FindFlag> setFlags(SearchDirection direction);
    QThread mThread;
    bool mSearching = false;
    QMutex mMutex;
    bool mOutsideOfList = false;
};

}
}
}
#endif // SEARCHDIALOG_H
