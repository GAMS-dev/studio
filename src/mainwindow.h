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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QMainWindow>

#include "editors/codeeditor.h"
#include "filerepository.h"
#include "modeldialog/libraryitem.h"
#include "option/commandlinehistory.h"
#include "option/commandlineoption.h"
#include "option/lineeditcompleteevent.h"
#include "option/optioneditor.h"
#include "projectcontextmenu.h"
#include "helpview.h"
#include "resultsview.h"
#include "commandlineparser.h"
#include "statuswidgets.h"

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
class GoToWidget;
class AutosaveHandler;

struct RecentData {

    QWidget *editor() const;
    void setEditor(QWidget *editor, MainWindow* window);

    FileId editFileId = -1;
    QString path = ".";
    FileGroupContext* group = nullptr;

private:
    QWidget* mEditor = nullptr;
};

struct HistoryData {
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
    explicit MainWindow(StudioSettings *settings, QWidget *parent = nullptr);
    ~MainWindow();
    void createEdit(QTabWidget* tabWidget, bool focus, int id = -1, int codecMip = -1);
    void updateMenuToCodec(int mib);
    QStringList openedFiles();
    void openFile(const QString &filePath);
    void openFileSkipSettings(const QString &filePath);
    void openFiles(QStringList pathList);


    bool outputViewVisibility();
    bool projectViewVisibility();
    bool optionEditorVisibility();
    bool helpViewVisibility();
    QString encodingMIBsString();
    QList<int> encodingMIBs();
    void setEncodingMIBs(QString mibList, int active = -1);
    void setEncodingMIBs(QList<int> mibs, int active = -1);
    void setActiveMIB(int active = -1);
    HistoryData* history();
    void setOutputViewVisibility(bool visibility);
    void setProjectViewVisibility(bool visibility);
    void setOptionEditorVisibility(bool visibility);
    void setHelpViewVisibility(bool visibility);
    void setCommandLineHistory(CommandLineHistory* opt);
    void checkOptionDefinition(bool checked);
    bool isOptionDefinitionChecked();
    CommandLineHistory* commandLineHistory();
    FileRepository* fileRepository();
    QWidgetList openEditors();
    QList<AbstractEditor *> openLogs();
    SearchWidget* searchWidget() const;
    void showResults(SearchResultList &results);
    RecentData *recent();
    StudioSettings *settings() const;
    void openModelFromLib(QString glbFile, QString model, QString gmsFileName = "");
    HelpView *getDockHelpView() const;
    QStringList readTabs(const QJsonObject &json);
    void writeTabs(QJsonObject &json) const;
    QWidget *welcomePage() const;
    void delayedFileRestoration();

public slots:
    void receiveAction(QString action);
    void receiveModLibLoad(QString model);
    void receiveOpenDoc(QString doc, QString anchor);
    void updateEditorPos();
    void updateEditorMode();
    void updateEditorBlockCount();
    void on_runGmsFile(FileContext *fc);
    void on_changeMainGms(FileContext *fc);

private slots:
    void openFileContext(FileContext *fileContext, bool focus = true, int codecMib = -1);
    void codecChanged(QAction *action);
    void codecReload(QAction *action);
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
    void addToGroup(FileGroupContext *group, const QString &filepath);
    void sendSourcePath(QString &source);
    void openFilePath(QString filePath, FileGroupContext *parent, bool focus, int codecMip = -1);

    // View
    void gamsProcessStateChanged(FileGroupContext* group);
    void projectContextMenuRequested(const QPoint &pos);
    void setProjectNodeExpanded(const QModelIndex &mi, bool expanded);
    void toggleOptionDefinition(bool checked);
    void closeHelpView();
    void optionViewVisibiltyChanged(bool visibility);
    void helpViewVisibilityChanged(bool visibility);

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
    void on_actionHelp_triggered();
    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionUpdate_triggered();
    // View
    void on_actionOutput_View_toggled(bool checked);
    void on_actionOption_View_toggled(bool checked);
    void on_actionHelp_View_triggered(bool checked);
    void on_actionShow_Welcome_Page_triggered();
    void on_actionGAMS_Library_triggered();
    // Other
    void on_mainTab_tabCloseRequested(int index);
    void on_logTab_tabCloseRequested(int index);
    void on_projectView_activated(const QModelIndex &index);
    void on_actionProject_View_toggled(bool checked);
    void on_mainTab_currentChanged(int index);
     // Command Line Option
    void on_runWithChangedOptions();
    void on_runWithParamAndChangedOptions(const QList<OptionItem> forcedOptionItems);
    void on_commandLineHelpTriggered();

    void on_actionSettings_triggered();
    void on_actionSearch_triggered();
    void on_actionGo_To_triggered();
    void on_actionRedo_triggered();
    void on_actionUndo_triggered();
    void on_actionPaste_triggered();
    void on_actionCopy_triggered();
    void on_actionSelect_All_triggered();
    void on_actionCut_triggered();
    void on_actionSet_to_Uppercase_triggered();
    void on_actionSet_to_Lowercase_triggered();
    void on_actionReset_Zoom_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionZoom_In_triggered();
    void on_actionOverwrite_Mode_toggled(bool overwriteMode);
    void on_actionIndent_triggered();
    void on_actionOutdent_triggered();
    void on_actionDuplicate_Line_triggered();
    void on_actionRemove_Line_triggered();
    void on_actionComment_triggered();
    void on_actionSelect_encodings_triggered();
    void interruptTriggered();
    void stopTriggered();
    void toggleLogDebug();
    void on_actionRestore_Recently_Closed_Tab_triggered();

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void customEvent(QEvent *event);
    void timerEvent(QTimerEvent *event);

private:
    void initTabs();
    FileContext* addContext(const QString &path, const QString &fileName);
    void openContext(const QModelIndex& index);
    void addToOpenedFiles(QString filePath);
    void renameToBackup(QFile *file);
    void triggerGamsLibFileCreation(gams::studio::LibraryItem *item, QString gmsFileName);
    void execute(QString commandLineStr, FileContext *gmsFileContext = nullptr);
    void updateRunState();
    void createWelcomePage();
    void createRunAndCommandLineWidgets();
    bool requestCloseChanged(QList<FileContext*> changedFiles);
    void connectCommandLineWidgets();
    void setRunActionsEnabled(bool enable);
    bool isActiveTabEditable();
    QString getCommandLineStrFrom(const QList<OptionItem> optionItems,
                                  const QList<OptionItem> forcedOptionItems = QList<OptionItem>());
    void updateFixedFonts(const QString &fontFamily, int fontSize);
    void updateEditorLineWrapping();
    void parseFilesFromCommandLine(FileGroupContext *fgc);
    void dockWidgetShow(QDockWidget* dw, bool show);

private:
    Ui::MainWindow *ui;
    SearchWidget *mSearchWidget = nullptr;

    Option* mGamsOption;
    OptionEditor* mOptionEditor;
    QDockWidget* mDockOptionView;
    CommandLineHistory* mCommandLineHistory;
    CommandLineOption* mCommandLineOption;
    CommandLineTokenizer* mCommandLineTokenizer;
    QWidget* mCommandWidget;
    QCheckBox* mShowOptionDefintionCheckBox;

    HelpView* mDockHelpView = nullptr;

    GAMSProcess *mProcess = nullptr;
    GAMSLibProcess *mLibProcess = nullptr;
    QActionGroup *mCodecGroupSwitch;
    QActionGroup *mCodecGroupReload;
    RecentData mRecent;
    HistoryData *mHistory;
    std::unique_ptr<StudioSettings> mSettings;
    std::unique_ptr<AutosaveHandler> mAutosaveHandler;
    WelcomePage *mWp = nullptr;
    ResultsView *mResultsView = nullptr;
    bool mBeforeErrorExtraction = true;
    FileRepository mFileRepo;
    ProjectContextMenu mProjectContextMenu;
    void changeToLog(FileContext* fileContext);

    QToolButton* interruptToolButton = nullptr;
    QToolButton* mRunToolButton = nullptr;
    GoToWidget *mGoto;
    bool mLogDebugLines = false;
    QStringList mClosedTabs;
    bool mOverwriteMode = false;
    QTime mPerformanceTime;
    StatusWidgets* mStatusWidgets;
    int TimerID;
    FileMetrics mMetrics;
    bool mCheckOnce = true;
};

}
}

#endif // MAINWINDOW_H
