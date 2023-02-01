/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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

class GoToDialog;
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

struct HistoryData {
    QStringList &files() { return mLastOpenedFiles; }
    const QStringList &files() const { return mLastOpenedFiles; }
private:
    QStringList mLastOpenedFiles;
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
    void openFiles(QStringList files, OpenGroupOption opt = ogNone);
    void jumpToTab(FileMeta* fm);
    void jumpToLine(int line);
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
    QWidget *currentEdit();

    QWidgetList openEditors();
    QList<QWidget *> openLogs();
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
    void setSearchWidgetPos(const QPoint& searchWidgetPos);
    void execute(QString commandLineStr,
                 std::unique_ptr<AbstractProcess> process = nullptr);

    void resetHistory();
    void clearHistory(FileMeta *file);
    void historyChanged();
    int linesInEditor(QWidget *editor = nullptr);

    void showGamsUpdateWidget(const QString &text);

#ifdef QWEBENGINE
    help::HelpWidget *helpWidget() const;
#endif

    search::ResultsView *resultsView() const;
    void invalidateResultsView();

public slots:
    gams::studio::PExFileNode* openFilePath(QString filePath, gams::studio::PExProjectNode* knownProject = nullptr,
                                            gams::studio::OpenGroupOption opt = ogNone, bool focus = false, bool forcedAsTextEditor = false,
                                            gams::studio::NewTabStrategy tabStrategy = tabAfterCurrent);
    void openFolder(QString path, gams::studio::PExProjectNode* project = nullptr);
    void openFile(gams::studio::FileMeta *fileMeta, bool focus = true,
                  gams::studio::PExProjectNode *project = nullptr, int codecMib = -1,
                  bool forcedTextEditor = false, gams::studio::NewTabStrategy tabStrategy = tabAfterCurrent);
    void receiveAction(const QString &action);
    void receiveModLibLoad(QString gmsFile, bool forceOverwrite = false);
    void receiveOpenDoc(QString doc, QString anchor);
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
    void newFileDialog(QVector<gams::studio::PExProjectNode*> projects = QVector<gams::studio::PExProjectNode *>(),
                       const QString& solverName=QString(),
                       bool projectOnly = false);
    void updateCursorHistoryAvailability();
    void closeProject(gams::studio::PExProjectNode *project);
    void closeFileEditors(const gams::studio::FileId fileId, bool willReopen = false);
    void updateResults(search::SearchResultModel* results);
    void closeResultsView();
    void openPinView(int tabIndex, Qt::Orientation orientation);
    void setGroupFontSize(gams::studio::FontGroup fontGroup, qreal fontSize, QString fontFamily = QString());
    void scrollSynchronize(QWidget *sendingEdit, int dx, int dy);
    void extraSelectionsUpdated();

private slots:
    void initDelayedElements();
    void openDelayedFiles();
    void updateRecentEdit(QWidget *old, QWidget *now);
    void openFileNode(gams::studio::PExAbstractNode *node, bool focus = true,
                      int codecMib = -1, bool forcedAsTextEditor = false,
                      gams::studio::NewTabStrategy tabStrategy = tabAfterCurrent);
    void codecChanged(QAction *action);
    void codecReload(QAction *action);
    void activeTabChanged(int index);
    void tabBarClicked(int index);
    void fileChanged(const gams::studio::FileId fileId);
    void fileClosed(const gams::studio::FileId fileId);
    void fileEvent(const gams::studio::FileEvent &e);
    void logTabRenamed(QWidget *wid, const QString &newName);
    void processFileEvents();
    void postGamsRun(gams::studio::NodeId origin, int exitCode);
    void postGamsLibRun();
    void neosProgress(gams::studio::AbstractProcess *proc, gams::studio::ProcState progress);
    void remoteProgress(gams::studio::AbstractProcess *proc, gams::studio::ProcState progress);
    void closeNodeConditionally(gams::studio::PExFileNode *node);
    void addToGroup(gams::studio::PExProjectNode *project, const QString &filepath);
    void sendSourcePath(QString &source);
    void changeToLog(gams::studio::PExAbstractNode* node, bool openOutput, bool createMissing);
    void storeTree();
    void cloneBookmarkMenu(QMenu *menu);
    void editableFileSizeCheck(const QFile &file, bool &canOpen);
    void newProcessCall(const QString &text, const QString &call);
    void printDocument();
    void updateTabIcon(gams::studio::PExAbstractNode *node, int tabIndex);
    void updateTabIcons();


    // View
    void invalidateTheme();
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
    void on_actionClose_triggered();
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
    void actionGDX_Diff_triggered(QString workingDirectory, QString input1="", QString input2="");
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
    void showEngineStartDialog();
    void engineSubmit(bool start);
    engine::EngineProcess *createEngineProcess();
    void prepareEngineProcess();
    void sslValidation(QString errorMessage);
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
    int logTabCount();
    int currentLogTab();
    QTabWidget* mainTabs();
    void initGamsStandardPaths();
    QWidget *otherEdit();
    void initEdit(FileMeta *fileMeta, QWidget *edit);

private slots:
    void updateAndSaveSettings();
    void updateFonts(qreal fontSize = 0, QString fontFamily = QString());
    void updateEditorLineWrapping();
    void updateTabSize(int size);
    void openProject(const QString gspFile);
    void moveProjectDialog(gams::studio::PExProjectNode *project, bool fullCopy);
    void moveProjectCollideDialog(MultiCopyCheck mcs,
                                  const QStringList &srcFiles, const QStringList &dstFiles,
                                  QStringList &missFiles, QStringList &collideFiles);
    void copyFiles(const QStringList &srcFiles, const QStringList &dstFiles);
    void closePinView();
    void on_actionPin_Right_triggered();
    void on_actionPin_Below_triggered();
    void on_actionNavigator_triggered();

    void on_actionOpen_Project_triggered();

private:
    void zoomWidget(QWidget * widget, int range);
    void initWelcomePage();
    void initIcons();
    void initEnvironment();
    void initNavigator();
    void adjustFonts();
    QVector<PExAbstractNode*> selectedNodes(QModelIndex index = QModelIndex());
    bool handleFileChanges(FileMeta *fc, bool willReopen);
    PExFileNode* addNode(const QString &path, const QString &fileName, PExProjectNode *project = nullptr);
    FileProcessKind fileChangedExtern(FileId fileId);
    FileProcessKind fileDeletedExtern(FileId fileId);
    void openModelFromLib(const QString &glbFile, const QString &modelName, const QString &inputFile, bool forceOverwrite = false);
    void addToOpenedFiles(QString filePath);
    bool terminateProcessesConditionally(QVector<PExProjectNode *> projects);
    void restoreFromSettings();
    QString currentPath();
    neos::NeosProcess *createNeosProcess();
    bool executePrepare(PExProjectNode *project, QString commandLineStr, std::unique_ptr<AbstractProcess> process = nullptr);
    void execution(PExProjectNode *project);
    void openFilesDialog(OpenGroupOption opt);
    void openFilesProcess(const QStringList &files, OpenGroupOption opt);
    PExProjectNode *currentProject();
    int pinViewTabIndex();

    void triggerGamsLibFileCreation(modeldialog::LibraryItem *item);
    void showWelcomePage();
    bool requestCloseChanged(QVector<FileMeta*> changedFiles);
    bool isActiveProjectRunnable();
    bool isRecentGroupRunning();
    void loadCommandLines(PExFileNode* oldfn, PExFileNode* fn);
    void dockWidgetShow(QDockWidget* dw, bool show);
    int showSaveChangesMsgBox(const QString &text);
    void raiseEdit(QWidget *widget);
    void openFileEventMessageBox(QString filePath, bool deleted, bool modified, int count);
    void initToolBar();
    void updateToolbar(QWidget* current);
    void deleteScratchDirs(const QString& path);
    QFont getEditorFont(FontGroup fGroup, QString fontFamily = QString(), qreal pointSize = 0);
    bool isMiroAvailable(bool printError = true);
    bool validMiroPrerequisites();
    void restoreCursorPosition(CursorHistoryItem item);
    bool enabledPrintAction();
    void checkGamsLicense();
    void checkSslLibrary();
    void goToLine(int result);
    QString readGucValue(QString key);
    void initCompleterActions();

private:
    Ui::MainWindow *ui;
    GoToDialog *mGotoDialog;
    FileMetaRepo mFileMetaRepo;
    ProjectRepo mProjectRepo;
    TextMarkRepo mTextMarkRepo;
    QStringList mDelayedFiles;
    NavigationHistory* mNavigationHistory;
    SettingsDialog *mSettingsDialog = nullptr;
    OpenPermission mOpenPermission = opNone;
    pin::PinViewWidget *mPinView = nullptr;
    QHash<FontGroup, qreal> mGroupFontSize;
    NodeId mFocussedLog;

    WelcomePage *mWp;
    search::SearchDialog *mSearchDialog = nullptr;
    search::ResultsView *mResultsView = nullptr;
    NavigatorDialog *mNavigatorDialog = nullptr;
    QPoint mSearchWidgetPos;
#ifdef QWEBENGINE
    help::HelpWidget *mHelpWidget = nullptr;
#endif
    option::ParameterEditor *mGamsParameterEditor = nullptr;
    SystemLogEdit *mSyslog = nullptr;
    StatusWidgets* mStatusWidgets;
    QTimer mWinStateTimer;
    QTimer mSaveSettingsTimer;
    QPrinter mPrinter;
    QPrintDialog *mPrintDialog;

    GamsLibProcess *mLibProcess = nullptr;
    QActionGroup *mCodecGroupSwitch;
    QActionGroup *mCodecGroupReload;
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

    bool mDebugMode = false;
    bool mStartedUp = false;
    bool mShutDown = false;
    QStringList mClosedTabs;
    bool mOverwriteMode = false;
    int mTimerID;
    QStringList mOpenTabsList;
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
    bool mEngineNoDialog = false;
};

}
}

#endif // MAINWINDOW_H
