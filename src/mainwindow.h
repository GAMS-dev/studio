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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include "filerepository.h"
#include "codeeditor.h"

namespace Ui {
class MainWindow;
}

namespace gams {
namespace studio {

struct RecentData {
    int editFileId = -1;
    QPlainTextEdit* editor = nullptr;
    QString path = ".";
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void createEdit(QTabWidget* tabWidget, QString codecName = QString());
    void createEdit(QTabWidget* tabWidget, int id = -1, QString codecName = QString());
    void ensureCodecMenue(QString codecName);

signals:
    void processOutput(QString text);

private slots:
    void clearProc(int exitCode);
    void addLine(QProcess::ProcessChannel channel, QString text);
    void readyStdOut();
    void readyStdErr();
    void codecChanged(QAction *action);
    void activeTabChanged(int index);
    void fileChanged(int fileId);
    void fileChangedExtern(int fileId);
    void fileDeletedExtern(int fileId);
    void fileClosed(int fileId);
    void appendOutput(QString text);


private slots:
    // File
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();
    void on_actionSave_All_triggered();
    void on_actionClose_triggered();
    void on_actionClose_All_triggered();
    void on_actionExit_Application_triggered();
    // Edit

    // GAMS
    void on_actionRun_triggered();
    // About
    void on_actionOnline_Help_triggered();
    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();
    // View
    void on_actionProject_Explorer_triggered(bool checked);
    void on_actionBottom_Panel_triggered(bool checked);
    void on_actionShow_Welcome_Page_triggered();
    void on_actionGAMS_Library_triggered();
    // AOB
    void on_mainTab_tabCloseRequested(int index);
    void on_treeView_doubleClicked(const QModelIndex &index);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void initTabs();
    void openOrShow(FileContext *fileContext);
    void openOrShow(QString filePath, FileGroupContext *parent);

private:
    Ui::MainWindow *ui;
    QProcess *mProc = nullptr;
    QHash<QTextStream, QColor> mStreams;
    FileRepository mFileRepo;
    QMutex mOutputMutex;
    QActionGroup *mCodecGroup;
    RecentData mRecent;

};

}
}

#endif // MAINWINDOW_H
