/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "mainwindow.h"

namespace gams {
namespace studio {
namespace search {

namespace Ui {
class SearchDialog;
}

class FileWorker;
class AbstractSearchFileHandler;

class SearchDialog : public QDialog
{
    friend class Search;
    Q_OBJECT

public:
    explicit SearchDialog(AbstractSearchFileHandler* fileHandler, MainWindow *parent = nullptr);
    ~SearchDialog();

    void editorChanged(QWidget *editor);

    void clearResultsView();

    void autofillSearchDialog();

    Search* search();

    ///
    /// \brief jumpToResult jumps to a search result identified by an index.
    ///        Does not jump if the search cache is outdated.
    /// \param index index of a search result in a valid cache
    ///
    void jumpToResult(int index);

    ///
    /// \brief jumpToResult jumpts to a search result identified by a Result object.
    ///        Jump position can be wrong if the document changed since the result was generated.
    /// \param r SearchResult to jump to
    ///
    void jumpToResult(Result r);

    QPoint lastPosition();
    bool hasLastPosition();
    void show(const QPoint &pos);
    void updateSettings();
    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void on_searchNext();
    void on_searchPrev();
    void on_documentContentChanged(int from, int charsRemoved, int charsAdded);
    void finalUpdate();
    void intermediateUpdate(int hits);
    void updateMatchLabel(int current = 0, int max = -1);
    void on_btn_clear_clicked();
    void filesChanged();
    void relaySearchResults(bool showResults, QList<gams::studio::search::Result>* results);
    void updateDialogState();

private slots:
    void findAll();
    void replace();
    void replaceAll();
    void changeScope(int scope);
    void browse();
    void searchParameterChanged();

signals:
    void closeResults();
    void openHelpDocument(QString doc, QString anchor);
    void selectResult(int matchNr);
    void invalidateResultsView();
    void extraSelectionsUpdated();
    void toggle();
    void updateResults(gams::studio::search::SearchResultModel* resultModel);

protected:
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

private:
    void setupConnections();
    void restoreSettings();
    QRegularExpression createRegex();
    void clearSearch();
    Scope selectedScope() const;
    void setSearchStatus(Search::Status status, int hits = 0);
    QList<SearchFile> getFilesByScope(const Parameters &parameters);
    int updateLabelByCursorPos(int lineNr = -1, int colNr = -1);
    void insertHistory();
    void updateEditHighlighting();
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
    void updateComponentAvailability();
    void updateClearButton();
    Parameters createSearchParameters(bool showResults,
                                      bool ignoreReadonly = false,
                                      bool searchBackwards = false);

private:
    Ui::SearchDialog *ui;
    FileWorker* mFileWorker;
    QWidget* mCurrentEditor = nullptr;
    MainWindow* mMain = nullptr;
    AbstractSearchFileHandler* mFileHandler = nullptr;
    QPoint mLastPosition;

    Search mSearch;
    int mFilesInScope;

    SearchResultModel* mSearchResultModel = nullptr;

    TextView *mSplitSearchView = nullptr;
    QTextDocument::FindFlags mSplitSearchFlags;
    bool mSuppressParameterChangedEvent = false;
    int mSearchAnimation = 0;
    Search::Status mSearchStatus = Search::Status::Clear;
    PExProjectNode* mCurrentSearchGroup = nullptr;
};

}
}
}
#endif // SEARCHDIALOG_H
