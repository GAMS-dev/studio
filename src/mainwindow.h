/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>

#include "file/fileevent.h"
#include "file/filemetarepo.h"
#include "file/projectcontextmenu.h"
#include "file/projectrepo.h"
#include "file/recentdata.h"
#include "debugger/server.h"
#include "debugger/pincontrol.h"
#include "modeldialog/libraryitem.h"
#include "search/resultsview.h"
#include "option/parametereditor.h"
#include "statuswidgets.h"
#include "maintabcontextmenu.h"
#include "logtabcontextmenu.h"
#include "gdxdiffdialog/gdxdiffdialog.h"
#include "miro/mirocommon.h"
#include "editors/navigationhistory.h"
#include "neos/neosprocess.h"
#include "reference/symbolreferenceitem.h"
#include "engine/engineprocess.h"
#include "common.h"

#ifdef QWEBENGINE
#include "help/helpwidget.h"
#endif

namespace Ui {
class MainWindow;
}

namespace gams {
namespace studio {

class SettingsDialog;
class AbstractProcess;
class FileEventHandler;
class GamsProcess;
class GamsLibProcess;
class WelcomePage;
class NavigatorDialog;
class Settings;
class SearchResultList;
class AutosaveHandler;
class SystemLogEdit;
class NavigatorLineEdit;

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
}
class TabBarStyle;
namespace pin {
class PinViewWidget;
}
namespace support {
class CheckForUpdate;
}

struct HistoryData {
    QStringList &files() { return mLastOpenedFiles; }
    const QStringList &files() const { return mLastOpenedFiles; }
    QStringList &projects() { return mLastOpenedProjects; }
    const QStringList &projects() const { return mLastOpenedProjects; }
private:
    QStringList mLastOpenedFiles;
    QStringList mLastOpenedProjects;
};

class MainWindow : public QMainWindow
{
    friend class MainTabContextMenu;
    friend class LogTabContextMenu;
    friend class NavigatorDialog;

    Q_OBJECT

public:
    enum OpenPermission { opNone, opNoGsp, opAll };

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void updateMenuToCodec(int mib);
    void openFiles(const QStringList &files, OpenGroupOption opt = ogNone);
    void switchToLogTab(FileMeta* fm);
    void jumpToLine(int line);
    void watchProjectTree();

    bool outputViewVisibility();
    bool projectViewVisibility();
    bool optionEditorVisibility();
    bool helpViewVisibility();
    QStringList encodingNames();
    QString encodingMIBsString();
    QList<int> encodingMIBs();
    void setEncodingMIBs(const QString &mibList, int active = -1);
    void setEncodingMIBs(const QList<int> &mibs, int active = -1);
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
    QWidget *currentEdit();

    const QWidgetList constOpenedEditors();
    QWidgetList openedEditors();
    const QList<QWidget *> constOpenedLogs();
    QList<QWidget *> openedLogs();
    const QStringList openedFiles();
    search::SearchDialog* searchDialog() const;
    RecentData *recent();
    void openModelFromLib(const QString &glbFile, modeldialog::LibraryItem *model);
    bool readTabs(const QVariantMap &tabData);
    void writeTabs(QVariantMap &tabData) const;
    void resetViews();
    void resizeOptionEditor(const QSize &size);
    void setForeground();
    void setForegroundOSCheck();
    void convertLowerUpper(bool toUpper);
    void ensureInScreen();
    void setExtendedEditorVisibility(bool visible);
    void resetLoadAmount();
    void toggleDistractionFreeMode();
    void toggleSearchDialog();
    void toggleFullscreen();
    void execute(const QString &commandLineStr, AbstractProcess* process = nullptr,
                 gams::studio::debugger::DebugStartMode debug = gams::studio::debugger::NoDebug);

    void resetHistory();
    void removeFromHistory(const QString &file);
    void historyChanged();
    int linesInEditor(QWidget *editor = nullptr);

    void showGamsUpdateWidget(const QString &text, bool remindLater = true);

#ifdef QWEBENGINE
    help::HelpWidget *helpWidget() const;
#endif

    search::ResultsView *resultsView() const;
    void invalidateResultsView();

signals:
    void cleanupWorkspace(const QStringList&);

public slots:
    gams::studio::PExFileNode* openFilePath(const QString &filePath, gams::studio::PExProjectNode* knownProject = nullptr,
                                            gams::studio::OpenGroupOption opt = ogNone, bool focus = false, bool forcedAsTextEditor = false,
                                            gams::studio::NewTabStrategy tabStrategy = tabAfterCurrent);
    void openFolder(const QString &path, gams::studio::PExProjectNode* project = nullptr);
    void openFile(gams::studio::FileMeta *fileMeta, bool focus = true,
                  gams::studio::PExProjectNode *project = nullptr, int codecMib = -1,
                  bool forcedTextEditor = false, gams::studio::NewTabStrategy tabStrategy = tabAfterCurrent);
    void receiveAction(const QString &action);
    void receiveModLibLoad(const QString &gmsFile, bool forceOverwrite = false);
    void receiveOpenDoc(const QString &doc, const QString &anchor);
    void updateRunState();
    void updateStatusFile();
    void updateStatusPos();
    void updateStatusMode();
    void updateStatusLineCount();
    void updateStatusLoadAmount();
    void openRecentFile();
    void setMainFile(gams::studio::PExFileNode *node);
    void currentDocumentChanged(int from, int charsRemoved, int charsAdded);
    void getAdvancedActions(QList<QAction *> *actions);
    void appendSystemLogInfo(const QString &text) const;
    void appendSystemLogError(const QString &text) const;
    void appendSystemLogWarning(const QString &text) const;
    void parameterRunChanged();
    void newFileDialogPrepare(const QVector<gams::studio::PExProjectNode*> &projects, const QString &inPath = QString(),
                              const QString& solverName = QString(), gams::studio::FileKind fileKind = gams::studio::FileKind::None);
    void newFileDialog(const QVector<gams::studio::PExProjectNode*> &projects, const QString &inPath,
                       const QString& solverName = QString(), gams::studio::FileKind fileKind = gams::studio::FileKind::None);
    void updateCursorHistoryAvailability();
    void closeProject(gams::studio::PExProjectNode *project);
    void closeFileEditors(const FileId &fileId, bool willReopen = false);
    void updateResults(search::SearchResultModel* results);
    void closeResultsView();
    void openPinView(int tabIndex, Qt::Orientation orientation);
    void openInPinView(PExProjectNode *project, QWidget *editInMainTabs);
    void switchToMainTab(FileMeta *fileMeta);
    void setGroupFontSize(gams::studio::FontGroup fontGroup, qreal fontSize, const QString &fontFamily = QString());
    void scrollSynchronize(QWidget *sendingEdit, int dx, int dy);
    void extraSelectionsUpdated();

private slots:
    void initDelayedElements();
    void openDelayedFiles();
    void openFileNode(gams::studio::PExAbstractNode *node, bool focus = true,
                      int codecMib = -1, bool forcedAsTextEditor = false,
                      gams::studio::NewTabStrategy tabStrategy = tabAfterCurrent);
    void focusProject(PExProjectNode *project);
    void codecChanged(QAction *action);
    void codecReload(QAction *action);
    void activeTabChanged(int index);
    void tabBarClicked(int index);
    void fileChanged(const FileId &fileId);
    void fileModifiedChanged(const FileId &fileId, bool modified);
    void fileClosed(const FileId &fileId);
    void fileEvent(const gams::studio::FileEvent &e);
    void logTabRenamed(QWidget *wid, const QString &newName);
    void processFileEvents();
    void postGamsRun(const NodeId &origin, int exitCode);
    void stopDebugServer(PExProjectNode *project, bool stateChecked = false);
    void postGamsLibRun();
    void internalCloseProject(gams::studio::PExProjectNode *project, const QVector<FileMeta *> &openFiles);
    void neosProgress(gams::studio::AbstractProcess *proc, gams::studio::ProcState progress);
    void remoteProgress(gams::studio::AbstractProcess *proc, gams::studio::ProcState progress);
    void closeNodeConditionally(gams::studio::PExFileNode *node);
    void closeAndDeleteFiles(QList<PExFileNode *> fileNodes);
    void addToGroup(gams::studio::PExGroupNode *group, const QString &filepath);
    void sendSourcePath(QString &source);
    void changeToLog(gams::studio::PExAbstractNode* node, bool openOutput, bool createMissing);
    void storeTree();
    void cloneBookmarkMenu(QMenu *menu);
    void editableFileSizeCheck(const QFile &file, bool &canOpen);
    void newProcessCall(const QString &text, const QString &call);
    void printDocument();
    void updateTabIcon(gams::studio::PExAbstractNode *node, int tabIndex);
    void runnableChanged();


    // View
    void invalidateTheme(bool refreshSyntax);
    void rehighlightOpenFiles();
    void gamsProcessStateChanged(gams::studio::PExGroupNode* group);
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
    void on_menuFile_aboutToShow();
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionOpenAlternative_triggered();
    void on_actionOpen_Folder_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();
    void on_actionSave_All_triggered();
    void on_actionNew_Project_triggered();
    void on_actionMove_Project_triggered();
    void on_actionCopy_Project_triggered();
    void on_actionClose_Tab_triggered();
    void on_actionClose_All_triggered();
    void on_actionClose_All_Except_triggered();
    void on_actionExit_Application_triggered();
    void on_actionPrint_triggered();
    // Edit

    // GAMS
    void on_actionRun_triggered();
    void on_actionRun_with_GDX_Creation_triggered();
    void on_actionCompile_triggered();
    void on_actionCompile_with_GDX_Creation_triggered();
    void on_actionInterrupt_triggered();
    void on_actionStop_triggered();
    void on_actionGAMS_Library_triggered();
    void getParameterValue(QString param, QString &value);

    // MIRO
    void on_actionBase_mode_triggered();
    void on_actionConfiguration_mode_triggered();
    void on_actionStop_MIRO_triggered();
    void on_actionDeploy_triggered();
    void on_menuMIRO_aboutToShow();
    void writeNewAssemblyFileData();
    void miroDeploy(bool testDeploy, miro::MiroDeployMode mode);
    void setMiroRunning(bool running);
    void updateMiroEnabled(bool printError = true);

    // Tools
    void on_actionGDX_Diff_triggered();
    void actionGDX_Diff_triggered(const QString &workingDirectory, const QString &input1="", const QString &input2="");
    void actionResetGdxStates(const QStringList &files);
    void on_actionTerminal_triggered();
    void actionTerminalTriggered(const QString &workingDir);

    // Help
    void on_actionGamsHelp_triggered();
    void on_actionStudioHelp_triggered();
    void on_actionAbout_Studio_triggered();
    void on_gamsLicensing_triggered();
    void on_actionAbout_Qt_triggered();

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
    void on_referenceJumpTo(const reference::ReferenceItem &item);

    void focusCmdLine();
    void focusProjectExplorer();
    void focusCentralWidget();
    void focusProcessLogs();
    void openGdxDiffFile();

    void on_actionToggleBookmark_triggered();
    void on_actionNextBookmark_triggered();
    void on_actionPreviousBookmark_triggered();
    void on_actionRemoveBookmarks_triggered();
    void on_actionDeleteScratchDirs_triggered();
    void on_actionChangelog_triggered();
    void on_actionGoBack_triggered();
    void on_actionGoForward_triggered();

    void on_actionRunNeos_triggered();
    void on_actionRunEngine_triggered();
    void on_actionFoldAllTextBlocks_triggered();
    void on_actionUnfoldAllTextBlocks_triggered();
    void on_actionFoldDcoTextBlocks_triggered();
    void on_actionMove_Line_Up_triggered();
    void on_actionMove_Line_Down_triggered();

    void showNeosStartDialog();
    void prepareNeosProcess();
    void initEngineStartDialog(bool resume = false);
    void engineSubmit(bool start);
    engine::EngineProcess *createEngineProcess();
    void prepareEngineProcess();
    void sslValidation(const QString &errorMessage);
    void resumeEngine(PExProjectNode *project, engine::EngineProcess *engineProcess);
    void sslUserDecision(QAbstractButton *button);

protected:
    bool eventFilter(QObject*sender, QEvent* event) override;
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void customEvent(QEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    bool event(QEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void updateRecentEdit(QWidget *old, QWidget *now);
    int logTabCount();
    int currentLogTab();
    QTabWidget* mainTabs();
    void initGamsStandardPaths();
    QWidget *otherEdit();
    void initEdit(FileMeta *fileMeta, QWidget *edit);
    void checkForEngingJob();
    void ensureWorkspace();
    void continueAsyncCall();

private slots:
    void updateAndSaveSettings();
    void updateFonts(qreal fontSize = 0, const QString &fontFamily = QString());
    void updateEditorLineWrapping();
    void updateTabSize(int size);
    void openProject(const QString &gspFile);
    void moveProjectDialog(gams::studio::PExProjectNode *project, bool fullCopy);
    void moveProjectCollideDialog(gams::studio::MultiCopyCheck mcs, PExProjectNode *project,
                                  const QStringList &srcFiles, const QStringList &dstFiles,
                                  QStringList &missFiles, QStringList &collideFiles);
    void copyFiles(const QStringList &srcFiles, const QStringList &dstFiles, PExProjectNode *project = nullptr);
    void closePinView();
    void updateProjectList();
    void updateAllowedMenus();
    void on_cbFocusProject_currentIndexChanged(int index);

    void on_actionPin_Right_triggered();
    void on_actionPin_Below_triggered();
    void on_actionNavigator_triggered();

    void on_actionOpen_Project_triggered();
    void on_actionRunDebugger_triggered();
    void on_actionStepDebugger_triggered();

    void updateSystemLogTab(bool focus);


    void on_tbProjectSettings_clicked();

private:
    void zoomWidget(QWidget * widget, int range);
    void initWelcomePage();
    void initIcons();
    void initEnvironment();
    void initNavigator();
    void adjustFonts();
    QVector<PExAbstractNode*> selectedNodes(QModelIndex index = QModelIndex());
    bool handleFileChanges(FileMeta *fc, bool closeAndWillReopen);
    PExFileNode* addNode(const QString &path, const QString &fileName, PExProjectNode *project = nullptr);
    FileProcessKind fileChangedExtern(const FileId &fileId);
    FileProcessKind fileDeletedExtern(const FileId &fileId);
    void openModelFromLibPrepare(const QString &glbFile, const QString &modelName, const QString &inputFile, bool forceOverwrite = false);
    void openModelFromLib(const QString &glbFile, const QString &modelName, const QString &inputFile, bool forceOverwrite = false);
    void addToHistory(const QString &filePath);
    bool terminateProcessesConditionally(const QVector<PExProjectNode *> &projects);
    void restoreFromSettings();
    QString currentPath();
    neos::NeosProcess *createNeosProcess();
    bool executePrepare(PExProjectNode *project, const QString &commandLineStr, AbstractProcess* process = nullptr);
    void execution(PExProjectNode *project);
    void openFilesDialog(OpenGroupOption opt);
    void openFilesProcess(const QStringList &files, OpenGroupOption opt);
    PExProjectNode *currentProject();
    PExProjectNode *openProjectIfExists(const QString &projectFileName);
    int pinViewTabIndex();

    void triggerGamsLibFileCreation(modeldialog::LibraryItem *item);
    void showWelcomePage();
    bool requestCloseChanged(QVector<FileMeta*> changedFiles);
    bool isActiveProjectRunnable();
    bool isRecentGroupRunning();
    void loadCommandLines(PExProjectNode *oldProj, PExProjectNode *proj);
    void dockWidgetShow(QDockWidget* dw, bool show);
    int showSaveChangesMsgBox(const QString &text);
    void raiseEdit(QWidget *widget);
    void openFileEventMessageBox(QString filePath, bool deleted, bool modified, int count);
    void initToolBar();
    void updateCanSave(QWidget* current);
    void deleteScratchDirs(const QString& path);
    QFont getEditorFont(FontGroup fGroup, QString fontFamily = QString(), qreal pointSize = 0);
    bool isMiroAvailable(bool printError = true);
    bool validMiroPrerequisites();
    void restoreCursorPosition(CursorHistoryItem item);
    bool enabledPrintAction();
    void checkGamsLicense();
    void checkSslLibrary();
    QString readGucValue(const QString &key);
    void initCompleterActions();

    void checkForUpdates(const QString &text);
    QDate nextUpdateCheck();

private:
    Ui::MainWindow *ui;
    FileMetaRepo mFileMetaRepo;
    ProjectRepo mProjectRepo;
    TextMarkRepo mTextMarkRepo;
    QStringList mDelayedFiles;
    NavigationHistory* mNavigationHistory;
    SettingsDialog *mSettingsDialog = nullptr;
    OpenPermission mOpenPermission = opNone;
    pin::PinViewWidget *mPinView = nullptr;
    debugger::PinControl mPinControl;
    QHash<FontGroup, qreal> mGroupFontSize;
    NodeId mFocussedLog;

    WelcomePage *mWp;
    search::SearchDialog *mSearchDialog = nullptr;
    search::ResultsView *mResultsView = nullptr;
    NavigatorDialog *mNavigatorDialog = nullptr;
#ifdef QWEBENGINE
    help::HelpWidget *mHelpWidget = nullptr;
#endif
    option::ParameterEditor *mGamsParameterEditor = nullptr;
    SystemLogEdit *mSyslog = nullptr;
    StatusWidgets* mStatusWidgets;
    QTimer mWinStateTimer;
    QTimer mSaveSettingsTimer;
    QPrinter mPrinter;
    QPrintDialog *mPrintDialog = nullptr;

    GamsLibProcess *mLibProcess = nullptr;
    QActionGroup *mCodecGroupSwitch = nullptr;
    QActionGroup *mCodecGroupReload = nullptr;
    QActionGroup *mActFocusProject = nullptr;
    RecentData mRecent;
    HistoryData mHistory;
    QScopedPointer<AutosaveHandler> mAutosaveHandler;
    ProjectContextMenu mProjectContextMenu;
    MainTabContextMenu mMainTabContextMenu;
    LogTabContextMenu mLogTabContextMenu;
    NavigatorLineEdit* mNavigatorInput = nullptr;

    QMutex mFileMutex;
    QVector<FileEventData> mFileEvents;
    QTimer mFileTimer;
    QSharedPointer<FileEventHandler> mFileEventHandler;
    TabBarStyle *mTabStyle = nullptr;
    QString mExportProjectFilePath;
    QVariantMap mAsyncCallOptions;

    bool mDebugMode = false;
    bool mStartedUp = false;
    bool mShutDown = false;
    QStringList mClosedTabs;
    bool mOverwriteMode = false;
    int mTimerID;
    QVector<int> mClosedTabsIndexes;
    bool mMaximizedBeforeFullScreen;
    bool mIgnoreSslErrors = false;
    bool mNeosNoDialog = false;
    int mCurrentMainTab = -1;
    QString mNeosMail;
    qreal mInitialTableFontSize = -1.0;
    qreal mTableFontSizeDif = 0.0;

    bool mWidgetStates[4];
    QScopedPointer<gdxdiffdialog::GdxDiffDialog> mGdxDiffDialog;

    QScopedPointer<miro::MiroDeployDialog> mMiroDeployDialog;
    bool mMiroRunning = false;
    QString mEngineAuthToken;
    QString mEngineJobTag;
    bool mEngineNoDialog = false;

    QScopedPointer<support::CheckForUpdate> mC4U;
};

}
}

#endif // MAINWINDOW_H
