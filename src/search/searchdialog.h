/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
#include "search.h"
#include "abstractsearchfilehandler.h"
#include "searchfilehandler.h"

namespace gams {
namespace studio {
namespace search {

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    friend Search;
    Q_OBJECT

public:
    explicit SearchDialog(MainWindow *parent, SearchFileHandler* fileHandler);
    ~SearchDialog();

    void setCurrentEditor(QWidget *editor);

    QRegularExpression createRegex();
    bool regex();
    bool caseSens();
    bool wholeWords();

    int selectedScope();
    void setSelectedScope(int index);

    void clearResultsView();
    void clearSearch();

    void autofillSearchField();

    QList<Result> filteredResultList(QString file) const;
    Search* search();

    void updateClearButton();

public slots:
    void on_searchNext();
    void on_searchPrev();
    void on_documentContentChanged(int from, int charsRemoved, int charsAdded);
    void finalUpdate();
    void intermediateUpdate(int hits);
    void updateNrMatches(int current = 0);
    void updateComponentAvailability();

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
    void showResults(gams::studio::search::SearchResultModel* results);
    void closeResults();

protected:
    void showEvent(QShowEvent *event);
    void keyPressEvent(QKeyEvent *e);

private:
    void restoreSettings();
    QList<FileMeta*> getFilesByScope(bool ignoreReadOnly = false);
    int updateLabelByCursorPos(int lineNr = -1, int colNr = -1);
    void insertHistory();
    void searchParameterChanged();
    void updateEditHighlighting();
    void updateUi(bool searching);
    void setSearchStatus(Search::Status status, int hits = 0);
    void clearSelection();
    void clearSearchSelection();

private:
    Ui::SearchDialog *ui;
    MainWindow *mMain;
    QWidget* mCurrentEditor;
    SearchFileHandler* mFileHandler = nullptr;

    Search mSearch;

    SearchResultModel* mSearchResultModel = nullptr;

    TextView *mSplitSearchView = nullptr;
    QTextDocument::FindFlags mSplitSearchFlags;
    bool mShowResults = true;
    bool mSuppressParameterChangedEvent = false;
    int mSearchAnimation = 0;
};

}
}
}
#endif // SEARCHDIALOG_H
