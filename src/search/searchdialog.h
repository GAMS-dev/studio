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

    int selectedScope();
    void setSelectedScope(int index);

    void findNext(SearchDialog::SearchDirection direction);
    void clearResults();
    void updateReplaceActionAvailability();

    void autofillSearchField();

    void clearSearch();
    void invalidateCache();

    SearchResultList* getCachedResults();
    void setActiveEditWidget(AbstractEdit *edit);

public slots:
    void on_searchNext();
    void on_searchPrev();
    void on_documentContentChanged(int from, int charsRemoved, int charsAdded);

protected slots:
    void returnPressed();

private slots:
    void on_btn_FindAll_clicked();
    void on_btn_Replace_clicked();
    void on_btn_ReplaceAll_clicked();
    void on_combo_scope_currentIndexChanged(int index);
    void on_btn_back_clicked();
    void on_btn_forward_clicked();
    void on_btn_clear_clicked();
    void on_combo_search_currentTextChanged(const QString &arg1);
    void on_cb_caseSens_stateChanged(int state);
    void on_cb_wholeWords_stateChanged(int arg1);
    void on_cb_regex_stateChanged(int arg1);

protected:
    void showEvent(QShowEvent *event);
    void keyPressEvent(QKeyEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    QFlags<QTextDocument::FindFlag> getFlags();
    void simpleReplaceAll();
    QList<Result> findInFile(FileMeta* fm, bool skipFilters = false);
    QList<Result> findInFiles(QList<FileMeta *> fml, bool skipFilters = false);
    QList<Result> findInGroup();
    QList<Result> findInOpenFiles();
    QList<Result> findInAllFiles();
    void updateMatchAmount(int hits, int current = 0);
    void selectNextMatch(SearchDirection direction);
    void insertHistory();
    void searchParameterChanged();
    void findOnDisk(QRegularExpression searchRegex, FileMeta *fm, SearchResultList *matches);
    void findInDoc(QRegularExpression searchRegex, FileMeta *fm, SearchResultList *matches);

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

    void setSearchStatus(SearchStatus status);

private:
    Ui::SearchDialog *ui;
    MainWindow *mMain;
    QTextCursor mSelection;       // selected with find
    QTextCursor mLastSelection;   // last selection, as starting point for find next
    bool mHasChanged = false;
    SearchResultList mCachedResults;
    bool mFirstReturn = false;
    AbstractEdit *mActiveEdit = nullptr;
};

}
}
#endif // SEARCHDIALOG_H
