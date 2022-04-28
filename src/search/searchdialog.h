/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include <QComboBox>

#include "search.h"
#include "abstractsearchfilehandler.h"

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
    explicit SearchDialog(AbstractSearchFileHandler* fileHandler, QWidget *parent = nullptr);
    ~SearchDialog();

    void editorChanged(QWidget *editor);

    QRegularExpression createRegex();
    bool regex();
    bool caseSens();
    bool wholeWords();

    Search::Scope selectedScope();
    void setSelectedScope(int index);

    void clearResultsView();
    void clearSearch();

    void autofillSearchField();

    QList<Result> filteredResultList(QString file) const;
    Search* search();

    void updateClearButton();
    QSet<FileMeta*> filterFiles(QSet<FileMeta *> files, bool ignoreReadOnly);

    void setSearchStatus(Search::Status status, int hits = 0);
    void jumpToResult(int matchNr);

public slots:
    void on_searchNext();
    void on_searchPrev();
    void on_documentContentChanged(int from, int charsRemoved, int charsAdded);
    void finalUpdate();
    void intermediateUpdate(int hits);
    void updateMatchLabel(int current = 0);
    void updateComponentAvailability();
    void on_btn_clear_clicked();

private slots:
    void on_btn_FindAll_clicked();
    void on_btn_Replace_clicked();
    void on_btn_ReplaceAll_clicked();
    void on_combo_scope_currentIndexChanged(int scope);
    void on_btn_back_clicked();
    void on_btn_forward_clicked();
    void on_combo_search_currentTextChanged(const QString);
    void on_cb_caseSens_stateChanged(int);
    void on_cb_wholeWords_stateChanged(int);
    void on_cb_regex_stateChanged(int);
    void on_btn_browse_clicked();
    void on_cb_subdirs_stateChanged(int);
    void on_combo_path_currentTextChanged(const QString);
    void on_combo_fileExcludePattern_currentTextChanged(const QString);

signals:
    void showResults(gams::studio::search::SearchResultModel* results);
    void closeResults();
    void setWidgetPosition(const QPoint& searchWidgetPos);
    void openHelpDocument(QString doc, QString anchor);
    void selectResult(int matchNr);
    void invalidateResultsView();
    void extraSelectionsUpdated();

protected:
    void showEvent(QShowEvent *event);
    void keyPressEvent(QKeyEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    void restoreSettings();
    QSet<FileMeta*> getFilesByScope(bool ignoreReadOnly = false);
    int updateLabelByCursorPos(int lineNr = -1, int colNr = -1);
    void insertHistory();
    void searchParameterChanged();
    void updateEditHighlighting();
    void updateDialogState();
    void clearSelection();
    void setSearchSelectionActive(bool active);
    AbstractSearchFileHandler* fileHandler();
    QWidget* currentEditor();
    void findNextPrev(bool backwards);
    void addEntryToComboBox(QComboBox* box);
    void adjustHeight();
    void setSearchedFiles(int files);
    bool checkSearchTerm();
    void checkRegex();

private:
    Ui::SearchDialog *ui;
    QWidget* mCurrentEditor = nullptr;
    AbstractSearchFileHandler* mFileHandler = nullptr;

    Search mSearch;
    int mFilesInScope;

    SearchResultModel* mSearchResultModel = nullptr;

    TextView *mSplitSearchView = nullptr;
    QTextDocument::FindFlags mSplitSearchFlags;
    bool mShowResults = true;
    bool mSuppressParameterChangedEvent = false;
    int mSearchAnimation = 0;
    Search::Status mSearchStatus = Search::Status::Clear;
    PExProjectNode* mCurrentSearchGroup = nullptr;
};

}
}
}
#endif // SEARCHDIALOG_H
