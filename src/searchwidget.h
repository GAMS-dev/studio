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
#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "mainwindow.h"
#include <QDialog>

namespace Ui {
class SearchWidget;
}

namespace gams {
namespace studio {

class SearchWidget : public QDialog
{
    Q_OBJECT

public:
    explicit SearchWidget(MainWindow *parent = 0);
    ~SearchWidget();

    bool regex();
    bool caseSens();
    bool wholeWords();
    QString searchTerm();

    int selectedScope();
    void setSelectedScope(int index);

    QList<Result> findInFile(FileSystemContext *fsc);

    enum SearchDirection {
        Forward = 0,
        Backward = 1
    };
    void findNext(SearchWidget::SearchDirection direction);
    void clearResults();

private slots:
    void on_btn_FindAll_clicked();
    void on_btn_Replace_clicked();
    void on_btn_ReplaceAll_clicked();
    void on_combo_scope_currentIndexChanged(int index);
    void on_btn_back_clicked();
    void on_btn_forward_clicked();
    void on_btn_clear_clicked();
    void on_combo_search_currentTextChanged(const QString &arg1);

private:
    Ui::SearchWidget *ui;
    MainWindow *mMain;
    QTextCursor mSelection;       // selected with find
    QTextCursor mLastSelection;   // last selection, as starting point for find next
    QList<TextMark*> mAllTextMarks;
    bool hasChanged = false;
    QList<Result> cachedResults;


    void showEvent(QShowEvent *event);
    void keyPressEvent(QKeyEvent *e);
    QFlags<QTextDocument::FindFlag> getFlags();
    void closeEvent(QCloseEvent *event);
    void simpleReplaceAll();
    QList<Result> findInGroup(FileSystemContext *fsc = nullptr);
    QList<Result> findInOpenFiles();
    QList<Result> findInAllFiles();
    void updateMatchAmount(int hits, int current = 0, bool clear = false);
    void selectNextMatch(SearchDirection direction, QList<Result> matches);
    void insertHistory();

    enum SearchScope {
        ThisFile = 0,
        ThisGroup= 1,
        OpenTabs = 2,
        AllFiles = 3
    };
};

}
}
#endif // SEARCHWIDGET_H
