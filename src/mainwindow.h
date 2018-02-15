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

#include <memory>
#include <QtWidgets>

#include "codeeditor.h"
#include "filerepository.h"
#include "modeldialog/libraryitem.h"
#include "option/commandlinehistory.h"
#include "option/commandlineoption.h"
#include "option/lineeditcompleteevent.h"
#include "option/optionconfigurator.h"
#include "option/optioneditor.h"
#include "projectcontextmenu.h"
#include "resultsview.h"
#include "commandlineparser.h"

namespace Ui {
class MainWindow;
}

namespace gams {
namespace studio {

class AbstractProcess;
class GAMSProcess;
class GAMSLibProcess;
class WelcomePage;
class StudioSettings;
class SearchWidget;
class SearchResultList;
class Result;

struct RecentData {
    FileId editFileId = -1;
    QWidget* editor = nullptr;
    QString path = ".";
    FileGroupContext* group = nullptr;
};

struct HistoryData {
    const int MAX_FILE_HISTORY = 8;
    QStringList lastOpenedFiles;

    // TODO: implement projects & sessions
    // QStringList mLastOpenedProjects;
    // QStringList mLastOpenedSessions;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ///
    /// \brief Constructs the GAMS Stuido main windows based on the given settings.
    /// \param settings The GAMS Studio settings.
    /// \param parent The parent widget.
    /// \remark <c>MainWindow</c> takes control of the <c>StudioSettings</c> pointer.
    ///
    explicit MainWindow(StudioSettings *settings, QWidget *parent = 0);
    ~MainWindow();
    void createEdit(QTabWidget* tabWidget, bool focus, QString codecName = QString());
    void createEdit(QTabWidget* tabWidget, bool focus, int id = -1, QString codecName = QString());
    void ensureCodecMenu(QString codecName);
    QStringList openedFiles();
    void openFile(const QString &filePath);
    void openFiles(QStringList pathList);

    bool outputViewVisibility();
    bool projectViewVisibility();
    bool optionEditorVisibility();
    HistoryData* history();
    void setOutputViewVisibility(bool visibility);
    void setProjectViewVisibility(bool visibility);
    void setOptionEditorVisibility(bool visibility);
    void setCommandLineHistory(CommandLineHistory* opt);
    CommandLineHistory* commandLineHistory();
    FileRepository* fileRepository();
    QWidgetList openEditors();
    QList<QPlainTextEdit*> openLogs();
    SearchWidget* searchWidget() const;
    void showResults(SearchResultList &results);
    RecentData *recent();
    StudioSettings *settings() const;
    void openModelFromLib(QString glbFile, QString model, QString gmsFileName = "");

public slots:
    void receiveAction(QString action);
    void receiveModLibLoad(QString lib);

private slots:
    void openFileContext(FileContext *fileContext, bool focus = true);
    void codecChanged(QAction *action);
    void activeTabChanged(int index);
    void fileChanged(FileId fileId);
    void fileChangedExtern(FileId fileId);
    void fileDeletedExtern(FileId fileId);
    void fileClosed(FileId fileId);
    void appendOutput(QProcess::ProcessChannel channel, QString text);
    void postGamsRun(AbstractProcess* process);
    void postGamsLibRun(AbstractProcess* process);
    void closeGroup(FileGroupContext* group);
    void closeFile(FileContext* file);
    void openFilePath(QString filePath, FileGroupContext *parent, bool focus);

    // View
    void gamsProcessStateChanged(FileGroupContext* group);
    void projectContextMenuRequested(const QPoint &pos);
    void setProjectNodeExpanded(const QModelIndex &mi, bool expanded);
    void toggleOptionDefinition(bool checked);

private slots:
    // File
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();
    void on_actionSave_All_triggered();
    void on_actionClose_triggered();
    void on_actionClose_All_triggered();
    void on_actionClose_All_Except_triggered();
    void on_actionExit_Application_triggered();
    // Edit

    // GAMS
    void on_actionRun_triggered();
    void on_actionRun_with_GDX_Creation_triggered();
    void on_actionCompile_triggered();
    void on_actionCompile_with_GDX_Creation_triggered();
    // About
    void on_actionOnline_Help_triggered();
    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();
    // View
    void on_actionOutput_View_triggered(bool checked);
    void on_actionOption_View_triggered(bool checked);
    void on_actionShow_Welcome_Page_triggered();
    void on_actionGAMS_Library_triggered();
    // Other
    void on_mainTab_tabCloseRequested(int index);
    void on_logTab_tabCloseRequested(int index);
    void on_projectView_activated(const QModelIndex &index);
    void on_actionProject_View_triggered(bool checked);
    void on_mainTab_currentChanged(int index);
     // Command Line Option
    void on_runWithChangedOptions();
    void on_runWithParamAndChangedOptions(const QList<OptionItem> forcedOptionItems);
    void on_commandLineHelpTriggered();

    void on_actionSettings_triggered();
    void on_actionSearch_triggered();

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void customEvent(QEvent *event);

private:
    void initTabs();
    FileContext* addContext(const QString &path, const QString &fileName);
    void openContext(const QModelIndex& index);
    void addToOpenedFiles(QString filePath);
    void renameToBackup(QFile *file);
    void triggerGamsLibFileCreation(gams::studio::LibraryItem *item, QString gmsFileName);
    void execute(QString commandLineStr);
    void updateRunState();
    void createWelcomePage();
    void createRunAndCommandLineWidgets();
    bool requestCloseChanged(QList<FileContext*> changedFiles);
    void connectCommandLineWidgets();
    void setRunActionsEnabled(bool enable);
    QString getCommandLineStrFrom(const QList<OptionItem> optionItems,
                                  const QList<OptionItem> forcedOptionItems = QList<OptionItem>());
    void updateEditorFont(const QString &fontFamily, int fontSize);
    void updateEditorLineWrapping();

private:
    Ui::MainWindow *ui;
    SearchWidget *mSearchWidget = nullptr;

    Option* gamsOption;
    OptionEditor* mOptionEditor;
    QDockWidget* mDockOptionView;
    CommandLineHistory* mCommandLineHistory;
    CommandLineOption* mCommandLineOption;
    CommandLineTokenizer* mCommandLineTokenizer;
    QSplitter* mOptionSplitter;

    GAMSProcess *mProcess = nullptr;
    GAMSLibProcess *mLibProcess = nullptr;
    QActionGroup *mCodecGroup;
    RecentData mRecent;
    HistoryData *mHistory;
    std::unique_ptr<StudioSettings> mSettings;
    WelcomePage *mWp = nullptr;
    ResultsView *mResultsView = nullptr;
    bool mBeforeErrorExtraction = true;
    FileRepository mFileRepo;
    ProjectContextMenu mProjectContextMenu;
    void changeToLog(FileContext* fileContext);
};

}
}

#endif // MAINWINDOW_H
