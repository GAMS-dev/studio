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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "editors/codeedit.h"
#include "file.h"
#include "file/recentdata.h"
#include "modeldialog/libraryitem.h"
#include "option/lineeditcompleteevent.h"
#include "search/resultsview.h"
#include "option/parametereditor.h"
#include "commandlineparser.h"
#include "statuswidgets.h"
#include "maintabcontextmenu.h"
#include "logtabcontextmenu.h"
#include "gdxdiffdialog/gdxdiffdialog.h"
#include "miro/mirocommon.h"
#include "editors/navigationhistory.h"

#ifdef QWEBENGINE
#include "help/helpwidget.h"
#endif

namespace Ui {
class MainWindow;
}

namespace gams {
namespace studio {

class AbstractProcess;
class GamsProcess;
class GamsLibProcess;
class WelcomePage;
class Settings;
class SearchResultList;
class AutosaveHandler;
class SystemLogEdit;
namespace search {
class SearchDialog;
}
namespace option {
class ParameterEditor;
}
namespace gdxdiffdialog {
class GdxDiffDialog;
}
namespace miro {
class MiroDeployDialog;
class MiroModelAssemblyDialog;
}

struct HistoryData {
    QStringList &files() { return mLastOpenedFiles; }
    const QStringList &files() const { return mLastOpenedFiles; }
private:
    QStringList mLastOpenedFiles;
};

class MainWindow : public QMainWindow
{
    friend MainTabContextMenu;
    friend LogTabContextMenu;

    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setInitialFiles(QStringList files);
    void updateMenuToCodec(int mib);
    void openFiles(QStringList files, bool forceNew = false);
    void watchProjectTree();

    bool outputViewVisibility();
    bool projectViewVisibility();
    bool optionEditorVisibility();
    bool helpViewVisibility();
    QStringList encodingNames();
    QString encodingMIBsString();
    QList<int> encodingMIBs();
    void setEncodingMIBs(QString mibList, int active = -1);
    void setEncodingMIBs(QList<int> mibs, int active = -1);
    void setActiveMIB(int active = -1);
    const HistoryData &history();
    void setOutputViewVisibility(bool visibility);
    void setProjectViewVisibility(bool visibility);
    void setOptionEditorVisibility(bool visibility);
    void setHelpViewVisibility(bool visibility);
    void setToolbarVisibility(bool visibility);
    FileMetaRepo* fileRepo();
    ProjectRepo* projectRepo();
    TextMarkRepo* textMarkRepo();

    QWidgetList openEditors();
    QList<QWidget *> openLogs();
    search::SearchDialog* searchDialog() const;
    void showResults(search::SearchResultList* results);
    void closeResultsPage();
    RecentData *recent();
    void openModelFromLib(const QString &glbFile, modeldialog::LibraryItem *model);
    bool readTabs(const QVariantMap &tabData);
    void writeTabs(QVariantMap &tabData) const;
    void resetViews();
    void resizeOptionEditor(const QSize &size);
    void updateRunState();
    void setForeground();
    void setForegroundOSCheck();
    void convertLowerUpper(bool toUpper);
    void ensureInScreen();
    void setExtendedEditorVisibility(bool visible);
    void resetLoadAmount();
    void openSearchDialog();
    void setSearchWidgetPos(const QPoint& searchWidgetPos);
    void execute(QString commandLineStr,
                 std::unique_ptr<AbstractProcess> process = nullptr,
                 ProjectFileNode *gmsFileNode = nullptr);
    void resetHistory();

#ifdef QWEBENGINE
    help::HelpWidget *helpWidget() const;
#endif
    option::ParameterEditor *gamsParameterEditor() const;

signals:
    void saved();
    void savedAs();

public slots:
    void openFilePath(const QString &filePath, bool focus = true, int codecMib = -1, bool forcedAsTextEditor = false);
    void receiveAction(const QString &action);
    void receiveModLibLoad(QString gmsFile, bool forceOverwrite = false);
    void receiveOpenDoc(QString doc, QString anchor);
    void updateEditorPos();
    void updateEditorMode();
    void updateEditorBlockCount();
    void updateEditorItemCount();
    void updateLoadAmount();
    void setMainGms(ProjectFileNode *node);
    void currentDocumentChanged(int from, int charsRemoved, int charsAdded);
    void getAdvancedActions(QList<QAction *> *actions);
    void appendSystemLog(const QString &text);
    void showErrorMessage(QString text);
    void parameterRunChanged();
    void newFileDialog(QVector<ProjectGroupNode *> groups = QVector<ProjectGroupNode *>(), const QString& solverName="");
    void updateCursorHistoryAvailability();
    bool eventFilter(QObject*sender, QEvent* event);

private slots:
    void openInitialFiles();
    void openFile(FileMeta *fileMeta, bool focus = true, ProjectRunGroupNode *runGroup = nullptr, int codecMib = -1, bool forcedTextEditor = false);
    void openFileNode(ProjectFileNode *node, bool focus = true, int codecMib = -1, bool forcedAsTextEditor = false);
    void reOpenFileNode(ProjectFileNode *node, bool focus = true, int codecMib = -1, bool forcedAsTextEditor = false);
    void codecChanged(QAction *action);
    void codecReload(QAction *action);
    void activeTabChanged(int index);
    void fileChanged(const FileId fileId);
    void fileClosed(const FileId fileId);
    void fileEvent(const FileEvent &e);
    void processFileEvents();
    void postGamsRun(NodeId origin, int exitCode);
    void postGamsLibRun();
    void closeGroup(ProjectGroupNode* group);
    void closeNodeConditionally(ProjectFileNode *node);
    void closeFileEditors(const FileId fileId);
    void addToGroup(ProjectGroupNode *group, const QString &filepath);
    void sendSourcePath(QString &source);
    void changeToLog(ProjectAbstractNode* node, bool openOutput, bool createMissing);
    void storeTree();
    void cloneBookmarkMenu(QMenu *menu);
    void editableFileSizeCheck(const QFile &file, bool &canOpen);
    void newProcessCall(const QString &text, const QString &call);

    // View
    void invalidateScheme();
    void gamsProcessStateChanged(ProjectGroupNode* group);
    void projectContextMenuRequested(const QPoint &pos);
    void mainTabContextMenuRequested(const QPoint& pos);
    void logTabContextMenuRequested(const QPoint& pos);
    void setProjectNodeExpanded(const QModelIndex &mi, bool expanded);
    void isProjectNodeExpanded(const QModelIndex &mi, bool &expanded) const;
    void closeHelpView();
    void outputViewVisibiltyChanged(bool visibility);
    void projectViewVisibiltyChanged(bool visibility);
    void helpViewVisibilityChanged(bool visibility);
    void toolbarVisibilityChanged(bool visibility);
    void showMainTabsMenu();
    void showLogTabsMenu();
    void showTabsMenu();
    void pushDockSizes();
    void popDockSizes();

    // File
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionOpenNew_triggered();
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
    void on_actionInterrupt_triggered();
    void on_actionStop_triggered();
    void on_actionGAMS_Library_triggered();

    // MIRO
    void on_actionBase_mode_triggered();
    void on_actionHypercube_mode_triggered();
    void on_actionConfiguration_mode_triggered();
    void on_actionStop_MIRO_triggered();
    void on_actionCreate_model_assembly_triggered();
    void on_actionDeploy_triggered();
    void on_menuMIRO_aboutToShow();
    void miroAssemblyDialogFinish(int result);
    void miroDeployAssemblyFileUpdate();
    void miroDeploy(bool testDeploy, miro::MiroDeployMode mode);
    void setMiroRunning(bool running);

    // Tools
    void on_actionGDX_Diff_triggered();
    void actionGDX_Diff_triggered(QString workingDirectory, QString input1="", QString input2="");
    void on_actionTerminal_triggered();
    void actionTerminalTriggered(const QString &workingDir);

    // About
    void on_actionHelp_triggered();
    void on_actionAbout_Studio_triggered();
    void on_actionAbout_GAMS_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionUpdate_triggered();

    // View
    void on_actionProcess_Log_triggered(bool checked);
    void on_actionProject_View_triggered(bool checked);
    void on_actionToggle_Extended_Parameter_Editor_toggled(bool checked);
    void on_actionHelp_View_triggered(bool checked);
    void on_actionShow_System_Log_triggered();
    void on_actionShow_Welcome_Page_triggered();
    void on_actionFull_Screen_triggered();
    void on_actionShowToolbar_triggered(bool checked);
    void on_actionDistraction_Free_Mode_toggled(bool checked);

    // Other
    void on_mainTabs_tabCloseRequested(int index);
    void on_logTabs_tabCloseRequested(int index);
    void on_projectView_activated(const QModelIndex &index);

    void on_actionSettings_triggered();
    void on_actionSearch_triggered();
    void on_actionGo_To_triggered();
    void on_actionRedo_triggered();
    void on_actionUndo_triggered();
    void on_actionPaste_triggered();
    void on_actionCopy_triggered();
    void on_actionSelect_All_triggered();
    void on_collapseAll();
    void on_expandAll();
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
    void toggleDebugMode();
    void on_actionRestore_Recently_Closed_Tab_triggered();
    void on_actionReset_Views_triggered();
    void initAutoSave();
    void on_actionEditDefaultConfig_triggered();

    void on_actionNextTab_triggered();
    void on_actionPreviousTab_triggered();
    void on_referenceJumpTo(reference::ReferenceItem item);

    void focusCmdLine();
    void focusProjectExplorer();
    void focusCentralWidget();
    void focusProcessLogs();

    void on_actionToggleBookmark_triggered();
    void on_actionNextBookmark_triggered();
    void on_actionPreviousBookmark_triggered();
    void on_actionRemoveBookmarks_triggered();
    void on_actionDeleteScratchDirs_triggered();
    void on_actionChangelog_triggered();
    void openGdxDiffFile();
    void on_actionGoBack_triggered();
    void on_actionGoForward_triggered();
    void on_actionPrint_triggered();

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *e);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void customEvent(QEvent *event);
    void timerEvent(QTimerEvent *event);
    bool event(QEvent *event);
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);
    int logTabCount();
    int currentLogTab();
    QTabWidget* mainTabs();

private:
    void initTabs();
    void initIcons();
    ProjectFileNode* addNode(const QString &path, const QString &fileName, ProjectGroupNode *group = nullptr);
    int fileChangedExtern(FileId fileId, bool ask, int count = 1);
    int fileDeletedExtern(FileId fileId, bool ask, int count = 1);
    void openModelFromLib(const QString &glbFile, const QString &modelName, const QString &inputFile, bool forceOverwrite = false);
    void addToOpenedFiles(QString filePath);
    void historyChanged();
    bool terminateProcessesConditionally(QVector<ProjectRunGroupNode *> runGroups);
    void updateAndSaveSettings();
    void restoreFromSettings();
    QString currentPath();

    void triggerGamsLibFileCreation(modeldialog::LibraryItem *item);
    void showWelcomePage();
    bool requestCloseChanged(QVector<FileMeta*> changedFiles);
    bool isActiveTabRunnable();
    bool isRecentGroupRunning();
    void loadCommandLines(ProjectFileNode* oldfn, ProjectFileNode* fn);
    void updateFixedFonts(const QString &fontFamily, int fontSize);
    void updateEditorLineWrapping();
    void analyzeCommandLine(GamsProcess *process, const QString &commandLineStr, ProjectGroupNode *fgc);
    void dockWidgetShow(QDockWidget* dw, bool show);
    int showSaveChangesMsgBox(const QString &text);
    void raiseEdit(QWidget *widget);
    int externChangedMessageBox(QString filePath, bool deleted, bool modified, int count);
    void initToolBar();
    void updateToolbar(QWidget* current);
    void deleteScratchDirs(const QString& path);
    QFont createEditorFont(const QString &fontFamily, int pointSize);
    bool isMiroAvailable();
    bool validMiroPrerequisites();
    void restoreCursorPosition(CursorHistoryItem item);

private:
    Ui::MainWindow *ui;
    FileMetaRepo mFileMetaRepo;
    ProjectRepo mProjectRepo;
    TextMarkRepo mTextMarkRepo;
    QStringList mInitialFiles;
    NavigationHistory* mNavigationHistory;

    WelcomePage *mWp;
    search::SearchDialog *mSearchDialog = nullptr;
    QPoint mSearchWidgetPos;
#ifdef QWEBENGINE
    help::HelpWidget *mHelpWidget = nullptr;
#endif
    option::ParameterEditor *mGamsParameterEditor = nullptr;
    SystemLogEdit *mSyslog = nullptr;
    StatusWidgets* mStatusWidgets;
    QTimer mWinStateTimer;

    GamsLibProcess *mLibProcess = nullptr;
    QActionGroup *mCodecGroupSwitch;
    QActionGroup *mCodecGroupReload;
    RecentData mRecent;
    HistoryData mHistory;
    QScopedPointer<AutosaveHandler> mAutosaveHandler;
    ProjectContextMenu mProjectContextMenu;
    MainTabContextMenu mMainTabContextMenu;
    LogTabContextMenu mLogTabContextMenu;

    QVector<FileEventData> mFileEvents;
    QTimer mFileTimer;
    int mExternFileEventChoice = -1;

    bool mDebugMode = false;
    bool mStartedUp = false;
    QStringList mClosedTabs;
    bool mOverwriteMode = false;
    int mTimerID;
    QStringList mOpenTabsList;
    QVector<int> mClosedTabsIndexes;
    bool mMaximizedBeforeFullScreen;

    bool mWidgetStates[4];
    QScopedPointer<gdxdiffdialog::GdxDiffDialog> mGdxDiffDialog;

    QScopedPointer<miro::MiroDeployDialog> mMiroDeployDialog;
    QScopedPointer<miro::MiroModelAssemblyDialog> mMiroAssemblyDialog;
    bool mMiroRunning = false;
};

}
}

#endif // MAINWINDOW_H
