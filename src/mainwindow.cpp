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
#include <QtConcurrent>
#include <QSslSocket>
#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "editors/codeedit.h"
#include "editors/abstractedit.h"
#include "editors/systemlogedit.h"
#include "encodingsdialog.h"
#include "welcomepage.h"
#include "modeldialog/modeldialog.h"
#include "exception.h"
#include "commonpaths.h"
#include "process.h"
#include "lxiviewer/lxiviewer.h"
#include "gdxviewer/gdxviewer.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"
#include "logger.h"
#include "editors/navigationhistorylocator.h"
#include "settings.h"
#include "settingsdialog.h"
#include "search/searchdialog.h"
#include "search/searchfilehandler.h"
#include "search/searchlocator.h"
#include "search/searchresultmodel.h"
#include "search/resultsview.h"
#include "gotodialog.h"
#include "support/updatedialog.h"
#include "support/checkforupdatewrapper.h"
#include "autosavehandler.h"
#include "support/distributionvalidator.h"
#include "tabdialog.h"
#include "colors/palettemanager.h"
#include "help/helpdata.h"
#include "support/gamslicensingdialog.h"
#include "viewhelper.h"
#include "miro/miroprocess.h"
#include "miro/mirodeploydialog.h"
#include "miro/mirodeployprocess.h"
#include "confirmdialog.h"
#include "fileeventhandler.h"
#include "file/projectoptions.h"
#include "file/pathrequest.h"
#include "engine/enginestartdialog.h"
#include "neos/neosstartdialog.h"
#include "option/gamsuserconfig.h"
#include "keys.h"
#include "tabbarstyle.h"
#include "support/gamslicenseinfo.h"
#include "headerviewproxy.h"
#include "pinviewwidget.h"

#ifdef __APPLE__
#include "../platform/macos/macoscocoabridge.h"
#endif

namespace gams {
namespace studio {

static const QStringList COpenAltText {"&Open in New Project...",
                                       "&Open in Current Project...",
                                       "Open the file(s) in a new project",
                                       "Open the file(s) in the current project"};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      mGotoDialog(new GoToDialog(this)),
      mFileMetaRepo(this),
      mProjectRepo(this),
      mTextMarkRepo(this),
      mAutosaveHandler(new AutosaveHandler(this)),
      mMainTabContextMenu(this),
      mLogTabContextMenu(this),
      mFileEventHandler(new FileEventHandler(this)),
      mGdxDiffDialog(new gdxdiffdialog::GdxDiffDialog(this)),
      mMiroDeployDialog(new miro::MiroDeployDialog(this))
{
    mTextMarkRepo.init(&mFileMetaRepo, &mProjectRepo);
    initEnvironment();

    mPrintDialog = new QPrintDialog(&mPrinter, this);

    ui->setupUi(this);

    // Timers
    mFileTimer.setSingleShot(true);
    mFileTimer.setInterval(100);
    connect(&mFileTimer, &QTimer::timeout, this, &MainWindow::processFileEvents);
    mWinStateTimer.setSingleShot(true);
    mWinStateTimer.setInterval(10);
    connect(&mWinStateTimer, &QTimer::timeout, this, &MainWindow::pushDockSizes);
    mTimerID = startTimer(60000);

    setAcceptDrops(true);

    // Shortcuts
    ui->actionRedo->setShortcuts(ui->actionRedo->shortcuts() << QKeySequence("Ctrl+Shift+Z"));

#ifdef __APPLE__
    ui->actionNextTab->setShortcut(QKeySequence("Ctrl+}"));
    ui->actionPreviousTab->setShortcut(QKeySequence("Ctrl+{"));
    MacOSCocoaBridge::disableDictationMenuItem(true);
    MacOSCocoaBridge::disableCharacterPaletteMenuItem(true);
    MacOSCocoaBridge::setAllowsAutomaticWindowTabbing(false);
    MacOSCocoaBridge::setFullScreenMenuItemEverywhere(false);
    ui->actionFull_Screen->setShortcut(QKeySequence::FullScreen);
    ui->actionToggleBookmark->setShortcut(QKeySequence("Meta+M"));
    ui->actionPreviousBookmark->setShortcut(QKeySequence("Meta+,"));
    ui->actionNextBookmark->setShortcut(QKeySequence("Meta+."));
    ui->actionToggle_Extended_Parameter_Editor->setShortcut(QKeySequence("Meta+Alt+3"));
#else
    ui->actionFull_Screen->setShortcuts({QKeySequence("Alt+Enter"), QKeySequence("Alt+Return")});
#endif
    ui->actionDistraction_Free_Mode->setShortcuts({QKeySequence("Ctrl+Alt+Enter"), QKeySequence("Ctrl+Alt+Return")});
    ui->actionToggle_Extended_Parameter_Editor->setToolTip("<html><head/><body><p>Show Extended Parameter Editor (<span style=\"font-weight:600;\">"+ui->actionToggle_Extended_Parameter_Editor->shortcut().toString()+"</span>)</p></body></html>");

    new QShortcut(QKeySequence("Ctrl+="), this, SLOT(on_actionZoom_In_triggered()));
    ui->actionGoForward->setShortcut(QKeySequence(QKeySequence::Forward));
    ui->actionGoBack->setShortcut(QKeySequence(QKeySequence::Back));
    ui->actionGoForward->shortcuts().append(QKeySequence(Qt::ForwardButton));
    ui->actionGoBack->shortcuts().append(QKeySequence(Qt::BackButton));

    // PinView
    ui->splitter->setCollapsible(0, false);
    connect(&mMainTabContextMenu, &MainTabContextMenu::openPinView, this, &MainWindow::openPinView);
    connect(ui->mainTabs, &TabWidget::openPinView, this, &MainWindow::openPinView);
    mPinView = new pin::PinViewWidget(ui->splitter);
    connect(mPinView, &pin::PinViewWidget::hidden, this, &MainWindow::closePinView);

    // Status Bar
    mStatusWidgets = new StatusWidgets(this);

    // Project View Setup
    int iconSize = fontMetrics().lineSpacing() + 4;
    ui->projectView->setModel(mProjectRepo.treeModel());
    ui->projectView->setRootIndex(mProjectRepo.treeModel()->rootModelIndex());
    ui->projectView->setHeaderHidden(true);
    ui->projectView->setItemDelegate(new TreeItemDelegate(ui->projectView));
    ui->projectView->setIconSize(QSize(iconSize, iconSize));
    ui->projectView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->projectView->selectionModel(), &QItemSelectionModel::selectionChanged, &mProjectRepo, &ProjectRepo::selectionChanged);
    connect(ui->projectView, &ProjectTreeView::dropFiles, &mProjectRepo, &ProjectRepo::dropFiles);
    connect(ui->projectView, &ProjectTreeView::openProjectOptions, this, [this](QModelIndex idx) {
        PExProjectNode * project = mProjectRepo.node(idx)->toProject();
        if (project) openProjectOptions(project);
    });

    mProjectRepo.init(ui->projectView, &mFileMetaRepo, &mTextMarkRepo);
    mFileMetaRepo.init(&mTextMarkRepo, &mProjectRepo);

#ifdef QWEBENGINE
    mHelpWidget = new help::HelpWidget(this);
    ui->dockHelpView->setWidget(mHelpWidget);
    ui->dockHelpView->hide();
#endif

    mRecent.init(this);
    initToolBar();
    HeaderViewProxy *hProxy = HeaderViewProxy::instance();
    hProxy->setSepColor(palette().midlight().color());

    mCodecGroupReload = new QActionGroup(this);
    connect(mCodecGroupReload, &QActionGroup::triggered, this, &MainWindow::codecReload);
    mCodecGroupSwitch = new QActionGroup(this);
    connect(mCodecGroupSwitch, &QActionGroup::triggered, this, &MainWindow::codecChanged);
    connect(ui->mainTabs, &QTabWidget::currentChanged, this, &MainWindow::activeTabChanged);
    connect(ui->mainTabs, &QTabWidget::currentChanged, this, &MainWindow::on_menuFile_aboutToShow);
    connect(ui->logTabs, &QTabWidget::tabBarClicked, this, &MainWindow::tabBarClicked);
    connect(ui->logTabs, &TabWidget::closeTab, this, &MainWindow::on_logTabs_tabCloseRequested);
    connect(ui->mainTabs, &QTabWidget::tabBarClicked, this, &MainWindow::tabBarClicked);
    connect(ui->mainTabs, &TabWidget::closeTab, this, &MainWindow::on_mainTabs_tabCloseRequested);

    connect(&mFileMetaRepo, &FileMetaRepo::fileEvent, this, &MainWindow::fileEvent);
    connect(&mFileMetaRepo, &FileMetaRepo::editableFileSizeCheck, this, &MainWindow::editableFileSizeCheck);
    connect(&mFileMetaRepo, &FileMetaRepo::setGroupFontSize, this, &MainWindow::setGroupFontSize);
    connect(&mFileMetaRepo, &FileMetaRepo::scrollSynchronize, this, &MainWindow::scrollSynchronize);
    connect(&mProjectRepo, &ProjectRepo::addWarning, this, &MainWindow::appendSystemLogWarning);
    connect(&mProjectRepo, &ProjectRepo::openFile, this, &MainWindow::openFile);
    connect(&mProjectRepo, &ProjectRepo::openFolder, this, &MainWindow::openFolder);
    connect(&mProjectRepo, &ProjectRepo::openProject, this, &MainWindow::openProject);
    connect(&mProjectRepo, &ProjectRepo::setNodeExpanded, this, &MainWindow::setProjectNodeExpanded);
    connect(&mProjectRepo, &ProjectRepo::isNodeExpanded, this, &MainWindow::isProjectNodeExpanded);
    connect(&mProjectRepo, &ProjectRepo::gamsProcessStateChanged, this, &MainWindow::gamsProcessStateChanged);
    connect(&mProjectRepo, &ProjectRepo::getParameterValue, this, &MainWindow::getParameterValue);
    connect(&mProjectRepo, &ProjectRepo::closeFileEditors, this, &MainWindow::closeFileEditors);
    connect(&mProjectRepo, &ProjectRepo::openRecentFile, this, &MainWindow::openRecentFile);
    connect(&mProjectRepo, &ProjectRepo::logTabRenamed, this, &MainWindow::logTabRenamed);

    connect(ui->projectView, &QTreeView::customContextMenuRequested, this, &MainWindow::projectContextMenuRequested);
    connect(&mProjectContextMenu, &ProjectContextMenu::closeProject, this, &MainWindow::closeProject);
    connect(&mProjectContextMenu, &ProjectContextMenu::closeFile, this, &MainWindow::closeNodeConditionally);
    connect(&mProjectContextMenu, &ProjectContextMenu::addExistingFile, this, &MainWindow::addToGroup);
    connect(&mProjectContextMenu, &ProjectContextMenu::getSourcePath, this, &MainWindow::sendSourcePath);
    connect(&mProjectContextMenu, &ProjectContextMenu::newFileDialog, this, &MainWindow::newFileDialog);
    connect(&mProjectContextMenu, &ProjectContextMenu::setMainFile, this, &MainWindow::setMainGms);
    connect(&mProjectContextMenu, &ProjectContextMenu::openLogFor, this, &MainWindow::changeToLog);
    connect(&mProjectContextMenu, &ProjectContextMenu::openFilePath, this, &MainWindow::openFileWithOption);
    connect(&mProjectContextMenu, &ProjectContextMenu::selectAll, this, &MainWindow::on_actionSelect_All_triggered);
    connect(&mProjectContextMenu, &ProjectContextMenu::expandAll, this, &MainWindow::on_expandAll);
    connect(&mProjectContextMenu, &ProjectContextMenu::collapseAll, this, &MainWindow::on_collapseAll);
    connect(&mProjectContextMenu, &ProjectContextMenu::openTerminal, this, &MainWindow::actionTerminalTriggered);
    connect(&mProjectContextMenu, &ProjectContextMenu::openGdxDiffDialog, this, &MainWindow::actionGDX_Diff_triggered);
    connect(mGotoDialog, &QDialog::finished, this, &MainWindow::goToLine);

    ui->mainTabs->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->mainTabs->tabBar(), &QTabBar::customContextMenuRequested, this, &MainWindow::mainTabContextMenuRequested);
    ui->logTabs->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->logTabs->tabBar(), &QTabBar::customContextMenuRequested, this, &MainWindow::logTabContextMenuRequested);

    connect(&mProjectContextMenu, &ProjectContextMenu::openFile, this, &MainWindow::openFileNode);
    connect(&mProjectContextMenu, &ProjectContextMenu::reOpenFile, this, &MainWindow::reOpenFileNode);
    connect(&mProjectContextMenu, &ProjectContextMenu::exportProject, this, &MainWindow::exportProjectDialog);
    connect(&mProjectContextMenu, &ProjectContextMenu::importProject, this, &MainWindow::importProjectDialog);
    connect(&mProjectContextMenu, &ProjectContextMenu::newProject, this, &MainWindow::newProjectDialog);

    connect(ui->dockProjectView, &QDockWidget::visibilityChanged, this, &MainWindow::projectViewVisibiltyChanged);
    connect(ui->dockProcessLog, &QDockWidget::visibilityChanged, this, &MainWindow::outputViewVisibiltyChanged);
    connect(ui->dockHelpView, &QDockWidget::visibilityChanged, this, &MainWindow::helpViewVisibilityChanged);
    connect(ui->toolBar, &QToolBar::visibilityChanged, this, &MainWindow::toolbarVisibilityChanged);

    ui->dockProjectView->installEventFilter(this);
    ui->dockProcessLog->installEventFilter(this);
    ui->dockHelpView->installEventFilter(this);
    installEventFilter(this);

    connect(this, &MainWindow::saved, this, &MainWindow::on_actionSave_triggered);
    connect(this, &MainWindow::savedAs, this, &MainWindow::on_actionSave_As_triggered);
    connect(qApp, &QApplication::focusChanged, this, &MainWindow::updateRecentEdit);

    connect(mGdxDiffDialog.get(), &QDialog::accepted, this, &MainWindow::openGdxDiffFile);
    connect(mMiroDeployDialog.get(), &miro::MiroDeployDialog::accepted,
            this, [this](){ miroDeploy(false, miro::MiroDeployMode::None); });
    connect(mMiroDeployDialog.get(), &miro::MiroDeployDialog::deploy,
            this, &MainWindow::miroDeploy);
    connect(mMiroDeployDialog.get(), &miro::MiroDeployDialog::newAssemblyFileData,
            this, &MainWindow::writeNewAssemblyFileData);

    setEncodingMIBs(encodingMIBs());
    ui->menuEncoding->setEnabled(false);

    engine::EngineProcess::startupInit();
    mEngineAuthToken = Settings::settings()->toString(SettingsKey::skEngineUserToken);
    mTabStyle = new TabBarStyle(ui->mainTabs, ui->logTabs, QApplication::style()->objectName());
    initIcons();
    restoreFromSettings();

    search::SearchFileHandler* sfh = new search::SearchFileHandler(this);
    mSearchDialog = new search::SearchDialog(sfh, this);
    connect(&mProjectContextMenu, &ProjectContextMenu::closeFile, mSearchDialog,
            &search::SearchDialog::updateDialogState);

    connect(mSearchDialog, &search::SearchDialog::updateResults, this, &MainWindow::updateResults);
    connect(mSearchDialog, &search::SearchDialog::closeResults, this, &MainWindow::closeResultsView);
    connect(mSearchDialog, &search::SearchDialog::setWidgetPosition, this, &MainWindow::setSearchWidgetPos);
    connect(mSearchDialog, &search::SearchDialog::openHelpDocument, this, &MainWindow::receiveOpenDoc);
    connect(mSearchDialog, &search::SearchDialog::invalidateResultsView, this, &MainWindow::invalidateResultsView);
    connect(mSearchDialog, &search::SearchDialog::extraSelectionsUpdated, this, &MainWindow::extraSelectionsUpdated);
    connect(mSearchDialog, &search::SearchDialog::toggle, this, &MainWindow::toggleSearchDialog);
    connect(&mProjectRepo, &ProjectRepo::childrenChanged, mSearchDialog, &search::SearchDialog::filesChanged);

    mFileMetaRepo.completer()->setCasing(CodeCompleterCasing(Settings::settings()->toInt(skEdCompleterCasing)));

    // stack help under output
    if (tabifiedDockWidgets(ui->dockHelpView).contains(ui->dockProcessLog)) {
        ui->dockHelpView->setFloating(true);
        tabifyDockWidget(ui->dockProcessLog, ui->dockHelpView);
        ui->dockHelpView->setFloating(false);
    }

    mSyslog = new SystemLogEdit(this);
    ViewHelper::initEditorType(mSyslog, EditorType::syslog);
    mSyslog->setFont(getEditorFont(fgLog));
    connect(mSyslog, &AbstractEdit::zoomRequest, this, [this](int delta) {
        zoomWidget(mSyslog, delta);
    });
    on_actionShow_System_Log_triggered();

    mNavigationHistory = new NavigationHistory(this);
    NavigationHistoryLocator::provide(mNavigationHistory);
    connect(mNavigationHistory, &NavigationHistory::historyChanged, this, &MainWindow::updateCursorHistoryAvailability);

    initWelcomePage();

    QPalette pal = ui->projectView->palette();
    pal.setColor(QPalette::Highlight, Qt::transparent);
    ui->projectView->setPalette(pal);

    mNavigationHistory->startRecord();
    QPushButton *tabMenu = new QPushButton(Theme::icon(":/%1/menu"), "", ui->mainTabs);
    connect(tabMenu, &QPushButton::pressed, this, &MainWindow::showMainTabsMenu);
    tabMenu->setMaximumWidth(40);
    ui->mainTabs->setCornerWidget(tabMenu);
    tabMenu = new QPushButton(Theme::icon(":/%1/menu"), "", ui->logTabs);
    connect(tabMenu, &QPushButton::pressed, this, &MainWindow::showLogTabsMenu);
    tabMenu->setMaximumWidth(40);
    ui->logTabs->setCornerWidget(tabMenu);

    // set up services
    search::SearchLocator::provide(mSearchDialog->search());
    SysLogLocator::provide(mSyslog);
    QTimer::singleShot(0, this, &MainWindow::initDelayedElements);

    if (Settings::settings()->toString(skMiroInstallPath).isEmpty()) {
        auto path = QDir::toNativeSeparators(miro::MiroCommon::path(""));
        Settings::settings()->setString(skMiroInstallPath, path);
    }
    ui->menuMIRO->setEnabled(isMiroAvailable());

    // Themes
    ViewHelper::changeAppearance();
    connect(Theme::instance(), &Theme::changed, this, &MainWindow::invalidateTheme);
    invalidateTheme();
    ViewHelper::updateBaseTheme();
    initGamsStandardPaths();
    updateRunState();
    initCompleterActions();

    checkGamsLicense();
    checkSslLibrary();
}

void MainWindow::watchProjectTree()
{
    connect(&mProjectRepo, &ProjectRepo::changed, this, &MainWindow::storeTree);
    connect(&mProjectRepo, &ProjectRepo::childrenChanged, this, [this]() {
        // to update the project if changed
        mRecent.setEditor(mRecent.editor());
        updateRunState();
    });
    connect(&mProjectRepo, &ProjectRepo::parentAssigned, this, [this](const PExAbstractNode *node) {
        if (PExFileNode *fn = mProjectRepo.asFileNode(node->id())) {
            if (fn->file()->editors().contains(ui->mainTabs->currentWidget()))
                loadCommandLines(fn, fn);
        }
    });
    mStartedUp = true;
    updateRecentEdit(nullptr, ui->mainTabs->currentWidget());
}

MainWindow::~MainWindow()
{
    if (mSettingsDialog) mSettingsDialog->deleteLater();
    killTimer(mTimerID);
    delete mWp;
    delete ui;
    delete mNavigationHistory;
    delete mResultsView;
    delete mSearchDialog;
    delete mPrintDialog;
    FileType::clear();
    HeaderViewProxy::deleteInstance();
}

void MainWindow::initWelcomePage()
{
    mWp = new WelcomePage(this);
    connect(mWp, &WelcomePage::openFilePath, this, &MainWindow::openFilePath);
    if (Settings::settings()->toBool(skSkipWelcomePage))
        mWp->hide();
    else
        showWelcomePage();
}

void MainWindow::initEnvironment()
{
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    QString gamsDir = QDir::toNativeSeparators(CommonPaths::systemDir());
    QByteArray gamsArr = (gamsDir + QDir::listSeparator() + gamsDir + QDir::separator() + "gbin").toLatin1();

    QByteArray curPath = qgetenv("PATH");
    qputenv("PATH", gamsArr + (curPath.isEmpty()? QByteArray() : QDir::listSeparator().toLatin1() + curPath));

#ifndef _WIN32
    curPath = qgetenv("LD_LIBRARY_PATH");
    qputenv("LD_LIBRARY_PATH", gamsArr + (curPath.isEmpty()? QByteArray() : QDir::listSeparator().toLatin1() + curPath));
#endif
#ifdef __APPLE__
    curPath = qgetenv("DYLD_LIBRARY_PATH");
    qputenv("DYLD_LIBRARY_PATH", gamsArr + (curPath.isEmpty()? QByteArray() : QDir::listSeparator().toLatin1() + curPath));
#endif
}

void MainWindow::adjustFonts()
{
    const qreal fontFactor = 0.95;
    const qreal fontFactorStatusbar = 0.85;
    QFont f(ui->menuBar->font());

    f.setPointSizeF(ui->menuBar->font().pointSizeF() * fontFactor);
    mTableFontSizeDif = f.pointSizeF() - Settings::settings()->toInt(skEdFontSize);
    ui->centralWidget->setFont(f);
    ui->splitter->setFont(f);
    ui->dockProjectView->setFont(f);
    ui->dockProcessLog->setFont(f);
    ui->dockHelpView->setFont(f);
    mGamsParameterEditor->dockChild()->setFont(f);

    f.setPointSizeF(f.pointSizeF() * fontFactorStatusbar);
    ui->statusBar->setFont(f);
}

void MainWindow::initIcons()
{
    setWindowIcon(Theme::icon(":/img/gams-w"));
    ui->actionCompile->setIcon(Theme::icon(":/%1/code"));
    ui->actionCompile_with_GDX_Creation->setIcon(Theme::icon(":/%1/code-gdx"));
    ui->actionCopy->setIcon(Theme::icon(":/%1/copy"));
    ui->actionCut->setIcon(Theme::icon(":/%1/cut"));
    ui->actionClose->setIcon(Theme::icon(":/%1/remove"));
    ui->actionExit_Application->setIcon(Theme::icon(":/%1/door-open"));
    ui->actionGAMS_Library->setIcon(Theme::icon(":/%1/books"));
    ui->actionGDX_Diff->setIcon(Theme::icon(":/%1/gdxdiff"));
    ui->actionHelp_View->setIcon(Theme::icon(":/%1/question"));
    ui->actionInterrupt->setIcon(Theme::icon(":/%1/stop"));
    ui->actionNew->setIcon(Theme::icon(":/%1/file"));
    ui->actionNextBookmark->setIcon(Theme::icon(":/%1/forward"));
    ui->actionOpen->setIcon(Theme::icon(":/%1/folder-open-bw"));
    ui->actionPaste->setIcon(Theme::icon(":/%1/paste"));
    ui->actionPreviousBookmark->setIcon(Theme::icon(":/%1/backward"));
    ui->actionProcess_Log->setIcon(Theme::icon(":/%1/output"));
    ui->actionProject_View->setIcon(Theme::icon(":/%1/project"));
    ui->actionRedo->setIcon(Theme::icon(":/%1/redo"));
    ui->actionReset_Zoom->setIcon(Theme::icon(":/%1/search-off"));
    ui->actionRun->setIcon(Theme::icon(":/%1/play"));
    ui->actionRun_with_GDX_Creation->setIcon(Theme::icon(":/%1/run-gdx"));
    ui->actionRunNeos->setIcon(Theme::icon(":/img/neos", false, ":/img/neos-g"));
    ui->actionRunEngine->setIcon(Theme::icon(":/img/engine", false, ":/img/engine-g"));
    ui->actionSave->setIcon(Theme::icon(":/%1/save"));
    ui->actionSearch->setIcon(Theme::icon(":/%1/search"));
    ui->actionSettings->setIcon(Theme::icon(":/%1/cog"));
    ui->actionStop->setIcon(Theme::icon(":/%1/kill"));
    ui->actionTerminal->setIcon(Theme::icon(":/%1/terminal"));
    ui->actionToggleBookmark->setIcon(Theme::icon(":/%1/bookmark"));
    ui->actionToggle_Extended_Parameter_Editor->setIcon(Theme::icon(":/%1/show"));
    ui->actionUndo->setIcon(Theme::icon(":/%1/undo"));
    ui->actionUpdate->setIcon(Theme::icon(":/%1/update"));
    ui->actionZoom_In->setIcon(Theme::icon(":/%1/search-plus"));
    ui->actionZoom_Out->setIcon(Theme::icon(":/%1/search-minus"));
    ui->actionShowToolbar->setIcon(Theme::icon(":/%1/hammer"));
    ui->actionGamsHelp->setIcon(Theme::icon(":/%1/book"));
    ui->actionStudioHelp->setIcon(Theme::icon(":/%1/book"));
    ui->actionChangelog->setIcon(Theme::icon(":/%1/new"));
    ui->actionGoForward->setIcon(Theme::icon(":/%1/forward"));
    ui->actionGoBack->setIcon(Theme::icon(":/%1/backward"));
}

void MainWindow::initToolBar()
{
    mGamsParameterEditor = new option::ParameterEditor(
                ui->actionRun, ui->actionRun_with_GDX_Creation, ui->actionCompile, ui->actionCompile_with_GDX_Creation,
                ui->actionRunNeos, ui->actionRunEngine, ui->actionInterrupt, ui->actionStop, this);

    // this needs to be done here because the widget cannot be inserted between separators from ui file
    ui->toolBar->insertSeparator(ui->actionToggle_Extended_Parameter_Editor);
    ui->toolBar->insertWidget(ui->actionToggle_Extended_Parameter_Editor, mGamsParameterEditor);
    ui->toolBar->insertSeparator(ui->actionProject_View);
    connect(mGamsParameterEditor->dockChild(), &AbstractView::zoomRequest, this, [this](int delta) {
        zoomWidget(mGamsParameterEditor->dockChild(), delta);
    });
}

void MainWindow::updateToolbar(QWidget* current)
{
    // deactivate save for welcome page
    bool activateSave = (current != mWp);

    for (auto a : ui->toolBar->actions()) {
        if (a->text() == "&Save") a->setEnabled(activateSave);
    }
}

void MainWindow::initAutoSave()
{
    mAutosaveHandler->recoverAutosaveFiles(mAutosaveHandler->checkForAutosaveFiles(mOpenTabsList));
}

void MainWindow::on_actionEditDefaultConfig_triggered()
{
    QString filePath = CommonPaths::defaultGamsUserConfigFile();

    QFile file(filePath);
    if (file.exists()) {
        FileMeta *fm = mFileMetaRepo.fileMeta(filePath);
        if (fm) {
           openFile(fm);
           return;
        }
    } else {
        file.open(QIODevice::WriteOnly); // create empty file
        file.close();
    }

    QFileInfo fi(filePath);

    PExProjectNode *project = mProjectRepo.createProject(fi.completeBaseName(), fi.absolutePath(), "");
    PExFileNode *node = addNode("", filePath, project);
    openFileNode(node);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    mAutosaveHandler->saveChangedFiles();
    updateAndSaveSettings();
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        ui->actionFull_Screen->setChecked(windowState().testFlag(Qt::WindowFullScreen));
        popDockSizes();
    } else if (event->type() == QEvent::WindowActivate) {
        processFileEvents();
    } else if (event->type() == QEvent::ApplicationPaletteChange) {
        if (!mSettingsDialog || !mSettingsDialog->preventThemeChanging())
            ViewHelper::updateBaseTheme();
        else {
            mSettingsDialog->delayBaseThemeChange(true);
        }
    }
    return QMainWindow::event(event);
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event)
    QScreen *screen = window()->screen();
    QSize scrDiff = screen->availableSize() - frameSize();
    if (!isMaximized() && !isFullScreen() && (scrDiff.width()>0 || scrDiff.height()>0) && screen->size() != size()) {
        Settings::settings()->setPoint(skWinPos, pos());
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    // JM: In the closing procedure the WindowFlag "Maximized" is deleted without resizing the window. That leads to a
    // resizeEvent with the wrong (maximized) size. Thus the metrics need to be taken into account here to skip that.

    QScreen *screen = window()->screen();
    QSize scrDiff = screen->availableSize() - frameSize();
    if (!isMaximized() && !isFullScreen() && (scrDiff.width()>0 || scrDiff.height()>0) && screen->size() != size()) {
        Settings::settings()->setSize(skWinSize, size());
    }
}

int MainWindow::logTabCount()
{
    return ui->logTabs->count();
}

int MainWindow::currentLogTab()
{
    return ui->logTabs->currentIndex();
}

QTabWidget* MainWindow::mainTabs()
{
    return ui->mainTabs;
}

void MainWindow::initGamsStandardPaths()
{
    support::GamsLicenseInfo licenseInfo;
    CommonPaths::setGamsStandardPaths(licenseInfo.gamsConfigLocations(), CommonPaths::StandardConfigPath);
    CommonPaths::setGamsStandardPaths(licenseInfo.gamsDataLocations(), CommonPaths::StandardDataPath);
}

QWidget *MainWindow::currentEdit()
{
    if (mPinView->widget() && mRecent.editor() == mPinView->widget())
        return mRecent.editor();
    if (ui->mainTabs->currentWidget() != mWp)
        return ui->mainTabs->currentWidget();
    return nullptr;
}

QWidget *MainWindow::otherEdit()
{
    // Only if edit has focus
    if (currentEdit() != mPinView->widget())
        return mPinView->widget();
    QWidget *wid = ui->mainTabs->currentWidget();
    if (wid == mWp) return nullptr;
    return wid;
}

void MainWindow::getParameterValue(QString param, QString &value)
{
    bool joker = param.endsWith('*');
    if (joker) param.resize(param.size()-1);
    QString params = mGamsParameterEditor->getCurrentCommandLineData();
    params = mGamsParameterEditor->getOptionTokenizer()->normalize(params);
    const QList<option::OptionItem> parList = mGamsParameterEditor->getOptionTokenizer()->tokenize(params);
    for (const option::OptionItem &item : parList) {
        if (joker) {
            if (item.key.startsWith(param, Qt::CaseInsensitive)) {
                value = item.value.trimmed();
                if (value.startsWith('"') && value.endsWith('"'))
                    value = value.mid(1, value.size()-2);
                return;
            }
        } else if (item.key.compare(param, Qt::CaseInsensitive) == 0) {
            value = item.value.trimmed();
            if (value.startsWith('"') && value.endsWith('"'))
                value = value.mid(1, value.size()-2);
            return;
        }
    }
}

void MainWindow::addToGroup(PExProjectNode* project, const QString& filepath)
{
    openFileNode(mProjectRepo.findOrCreateFileNode(filepath, project), true);
}

void MainWindow::sendSourcePath(QString &source)
{
    source = mRecent.path();
}

void MainWindow::updateMenuToCodec(int mib)
{
    ui->menuEncoding->setEnabled(mib != -1);
    if (mib == -1) return;
    QList<int> enc = encodingMIBs();
    if (!enc.contains(mib)) {
        enc << mib;
        std::sort(enc.begin(), enc.end());
        if (enc.contains(0)) enc.move(enc.indexOf(0), 0);
        if (enc.contains(106)) enc.move(enc.indexOf(106), 0);
        setEncodingMIBs(enc, mib);
    } else {
        setActiveMIB(mib);
    }
}

void MainWindow::setOutputViewVisibility(bool visibility)
{
    ui->actionProcess_Log->setChecked(visibility);
    ui->dockProcessLog->setVisible(visibility);
}

void MainWindow::setProjectViewVisibility(bool visibility)
{
    ui->actionProject_View->setChecked(visibility);
    ui->dockProjectView->setVisible(visibility);
}

void MainWindow::setOptionEditorVisibility(bool visibility)
{
    mGamsParameterEditor->setEditorExtended(visibility);
}

void MainWindow::setToolbarVisibility(bool visibility)
{
    ui->toolBar->setVisible(visibility);
}

void MainWindow::setHelpViewVisibility(bool visibility)
{
#ifdef QWEBENGINE
    if (!visibility)
        mHelpWidget->clearStatusBar();
    else
        mHelpWidget->setFocus();
    ui->actionHelp_View->setChecked(visibility);
    ui->dockHelpView->setVisible(visibility);
#endif
}

bool MainWindow::outputViewVisibility()
{
    return ui->actionProcess_Log->isChecked();
}

bool MainWindow::projectViewVisibility()
{
    return ui->actionProject_View->isChecked();
}

bool MainWindow::optionEditorVisibility()
{
    return mGamsParameterEditor->isEditorExtended();
}

bool MainWindow::helpViewVisibility()
{
    return ui->actionHelp_View->isChecked();
}

void MainWindow::on_actionProcess_Log_triggered(bool checked)
{
    dockWidgetShow(ui->dockProcessLog, checked);
}

void MainWindow::on_actionProject_View_triggered(bool checked)
{
    dockWidgetShow(ui->dockProjectView, checked);
}

void MainWindow::on_actionHelp_View_triggered(bool checked)
{
    dockWidgetShow(ui->dockHelpView, checked);
}

FileMetaRepo *MainWindow::fileRepo()
{
    return &mFileMetaRepo;
}

ProjectRepo *MainWindow::projectRepo()
{
    return &mProjectRepo;
}

TextMarkRepo *MainWindow::textMarkRepo()
{
    return  &mTextMarkRepo;
}

QWidgetList MainWindow::openEditors()
{
    QWidgetList res;
    for (int i = 0; i < ui->mainTabs->count(); ++i) {
        res << ui->mainTabs->widget(i);
    }
    return res;
}

QList<QWidget*> MainWindow::openLogs()
{
    QList<QWidget*> resList;
    for (int i = 0; i < ui->logTabs->count(); i++) {
        if (AbstractEdit* ed = ViewHelper::toAbstractEdit(ui->logTabs->widget(i)))
            resList << ed;
        if (TextView* tv = ViewHelper::toTextView(ui->logTabs->widget(i)))
            resList << tv;
    }
    return resList;
}

void MainWindow::receiveAction(const QString &action)
{
    if (action == "createNewFile")
        on_actionNew_triggered();
    else if(action == "browseModLib")
        on_actionGAMS_Library_triggered();
    else if(action == "openWhatsNew")
        on_actionChangelog_triggered();
}

void MainWindow::openModelFromLib(const QString &glbFile, modeldialog::LibraryItem* model)
{
    QFileInfo file(model->files().constFirst());
    QString inputFile = file.completeBaseName() + ".gms";

    openModelFromLib(glbFile, model->nameWithSuffix(), inputFile);
}

void MainWindow::openModelFromLib(const QString &glbFile, const QString &modelName, const QString &inputFile, bool forceOverwrite)
{
    bool openInCurrent = Settings::settings()->toBool(skOpenInCurrent) && mRecent.project();
    QString destDir = openInCurrent ? mRecent.project()->workDir() : Settings::settings()->toString(skDefaultWorkspace);
    QString gmsFilePath = destDir + "/" + inputFile;
    QFile gmsFile(gmsFilePath);

    mFileMetaRepo.unwatch(gmsFilePath);
    if (gmsFile.exists()) {
        FileMeta* fm = mFileMetaRepo.findOrCreateFileMeta(gmsFilePath);

        if (!forceOverwrite) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("File already existing");
            msgBox.setText("The file " + gmsFilePath + " already exists in your working directory.");
            msgBox.setInformativeText("What do you want to do with the existing file?");
            msgBox.setStandardButtons(QMessageBox::Abort);
            msgBox.addButton("Open", QMessageBox::ActionRole);
            msgBox.addButton("Replace", QMessageBox::ActionRole);
            int answer = msgBox.exec();

            switch(answer) {
            case 0: {// open
                PExProjectNode* project = openInCurrent ? mRecent.project() : nullptr;
                openFileNode(addNode("", gmsFilePath, project));
                return;
            }
            case 1: // replace
                fm->renameToBackup();
                // and continue;
                break;
            case QMessageBox::Abort:
                return;
            }
        } else {
            // continuing will replace old file
            fm->renameToBackup();
        }
    }

    QDir gamsSysDir(CommonPaths::systemDir());
    mLibProcess = new GamsLibProcess(this);
    mLibProcess->setInputFile(inputFile);
    mLibProcess->setWorkingDirectory(destDir);

    QStringList args {
        "-lib",
        QDir::toNativeSeparators(gamsSysDir.filePath(glbFile)),
        (modelName.isEmpty() ? QString::number(-1) : modelName),
        QDir::toNativeSeparators(destDir)
    };
    mLibProcess->setParameters(args);

    // This log is passed to the system-wide log
    connect(mLibProcess, &AbstractProcess::newProcessCall, this, &MainWindow::newProcessCall);
    connect(mLibProcess, &GamsProcess::newStdChannelData, this, &MainWindow::appendSystemLogInfo);
    connect(mLibProcess, &GamsProcess::finished, this, &MainWindow::postGamsLibRun);

    mLibProcess->execute();
}

void MainWindow::receiveModLibLoad(QString gmsFile, bool forceOverwrite)
{
    openModelFromLib("gamslib_ml/gamslib.glb", gmsFile, gmsFile + ".gms", forceOverwrite);
}

void MainWindow::receiveOpenDoc(QString doc, QString anchor)
{
    QString link = CommonPaths::systemDir() + "/" + doc;
    link = QFileInfo(link).canonicalFilePath();
    QUrl result = QUrl::fromLocalFile(link);

    if (!anchor.isEmpty())
        result = QUrl(result.toString() + "#" + anchor);
#ifdef QWEBENGINE
    helpWidget()->on_urlOpened(result);
#endif

    on_actionHelp_View_triggered(true);
}

search::SearchDialog* MainWindow::searchDialog() const
{
    return mSearchDialog;
}

QStringList MainWindow::encodingNames()
{
    QStringList res;
    for (QAction *act: ui->menuconvert_to->actions()) {
        if (!act->data().isNull()) {
            QTextCodec *codec = QTextCodec::codecForMib(act->data().toInt());
            if (!codec) continue;
            res << codec->name();
        }
    }
    return res;
}

QString MainWindow::encodingMIBsString()
{
    QStringList res;
    for (QAction *act: ui->menuconvert_to->actions()) {
        if (!act->data().isNull()) res << act->data().toString();
    }
    return res.join(",");
}

QList<int> MainWindow::encodingMIBs()
{
    QList<int> res;
    for (QAction *act: mCodecGroupReload->actions())
        if (!act->data().isNull()) res << act->data().toInt();
    return res;
}

void MainWindow::setEncodingMIBs(QString mibList, int active)
{
    QList<int> mibs;
    const QStringList strMibs = mibList.split(",");
    for (const QString &mib: strMibs) {
        if (mib.length()) mibs << mib.toInt();
    }
    setEncodingMIBs(mibs, active);
}

void MainWindow::setEncodingMIBs(QList<int> mibs, int active)
{
    while (mCodecGroupSwitch->actions().size()) {
        QAction *act = mCodecGroupSwitch->actions().constLast();
        if (ui->menuconvert_to->actions().contains(act))
            ui->menuconvert_to->removeAction(act);
        mCodecGroupSwitch->removeAction(act);
    }
    while (mCodecGroupReload->actions().size()) {
        QAction *act = mCodecGroupReload->actions().constLast();
        if (ui->menureload_with->actions().contains(act))
            ui->menureload_with->removeAction(act);
        mCodecGroupReload->removeAction(act);
    }
    for (int mib: mibs) {
        if (!QTextCodec::availableMibs().contains(mib)) continue;
        QAction *act;
        act = new QAction(QTextCodec::codecForMib(mib)->name(), mCodecGroupSwitch);
        act->setCheckable(true);
        act->setData(mib);
        act->setChecked(mib == active);

        act = new QAction(QTextCodec::codecForMib(mib)->name(), mCodecGroupReload);
        act->setCheckable(true);
        act->setData(mib);
        act->setChecked(mib == active);
    }
    ui->menuconvert_to->addActions(mCodecGroupSwitch->actions());
    ui->menureload_with->addActions(mCodecGroupReload->actions());
}

void MainWindow::setActiveMIB(int active)
{
    for (QAction *act: ui->menuconvert_to->actions())
        if (!act->data().isNull()) {
            act->setChecked(act->data().toInt() == active);
        }

    for (QAction *act: ui->menureload_with->actions())
        if (!act->data().isNull()) {
            act->setChecked(act->data().toInt() == active);
        }
}

void MainWindow::gamsProcessStateChanged(PExGroupNode* group)
{
    if (mRecent.project() == group) updateRunState();

    PExProjectNode* project = group->toProject();
    PExLogNode* log = project->logNode();

    QTabBar::ButtonPosition closeSide = QTabBar::ButtonPosition(style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, nullptr, this));
    for (int i = 0; i < ui->logTabs->children().size(); i++) {
        if (mFileMetaRepo.fileMeta(ui->logTabs->widget(i)) == log->file()) {

            if (project->gamsProcessState() == QProcess::Running)
                ui->logTabs->tabBar()->tabButton(i, closeSide)->hide();
            else if (project->gamsProcessState() == QProcess::NotRunning)
                ui->logTabs->tabBar()->tabButton(i, closeSide)->show();
        }
    }
}

void MainWindow::projectContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = ui->projectView->indexAt(pos);
    QVector<PExAbstractNode*> nodes;
    if (index.isValid()) {
        QModelIndexList list = ui->projectView->selectionModel()->selectedIndexes();
        if (!list.contains(index)) return;
        for (NodeId id: mProjectRepo.treeModel()->selectedIds()) {
            nodes << mProjectRepo.node(id);
        }
    }
    mProjectContextMenu.setNodes(nodes);
    mProjectContextMenu.setParent(this);
    mProjectContextMenu.exec(ui->projectView->viewport()->mapToGlobal(pos));
}

void MainWindow::mainTabContextMenuRequested(const QPoint& pos)
{
    int tabIndex = ui->mainTabs->tabBar()->tabAt(pos);
    QWidget *edit = ui->mainTabs->widget(tabIndex);
    FileMeta *fm = mFileMetaRepo.fileMeta(edit);
    mMainTabContextMenu.setTabIndex(tabIndex, fm && fm->isPinnable());
    mMainTabContextMenu.exec(ui->mainTabs->tabBar()->mapToGlobal(pos));
}

void MainWindow::logTabContextMenuRequested(const QPoint& pos)
{
    int tabIndex = ui->logTabs->tabBar()->tabAt(pos);
    mLogTabContextMenu.setTabIndex(tabIndex);
    mLogTabContextMenu.exec(ui->logTabs->tabBar()->mapToGlobal(pos));
}

void MainWindow::setProjectNodeExpanded(const QModelIndex& mi, bool expanded)
{
    ui->projectView->setExpanded(mi, expanded);
}

void MainWindow::isProjectNodeExpanded(const QModelIndex &mi, bool &expanded) const
{
    expanded = ui->projectView->isExpanded(mi);
}

void MainWindow::closeHelpView()
{
    if (ui->dockHelpView)
        ui->dockHelpView->close();
}

void MainWindow::outputViewVisibiltyChanged(bool visibility)
{
    ui->actionProcess_Log->setChecked(visibility || tabifiedDockWidgets(ui->dockProcessLog).count());
}

void MainWindow::projectViewVisibiltyChanged(bool visibility)
{
    ui->actionProject_View->setChecked(visibility || tabifiedDockWidgets(ui->dockProjectView).count());
}

void MainWindow::helpViewVisibilityChanged(bool visibility)
{
    ui->actionHelp_View->setChecked(visibility || tabifiedDockWidgets(ui->dockHelpView).count());
}

void MainWindow::toolbarVisibilityChanged(bool visibility)
{
    ui->actionShowToolbar->setChecked(visibility);
}

void MainWindow::showMainTabsMenu()
{
    TabDialog *tabDialog = new TabDialog(ui->mainTabs, this);
    tabDialog->exec();
    tabDialog->deleteLater();
}

void MainWindow::showLogTabsMenu()
{
    TabDialog *tabDialog = new TabDialog(ui->logTabs, this);
    tabDialog->exec();
    tabDialog->deleteLater();
}

void MainWindow::showTabsMenu()
{
    QWidget *wid = focusWidget();

    if (wid && wid->parent()->parent() == ui->logTabs)
        showLogTabsMenu();
    else
        showMainTabsMenu();
}

void MainWindow::pushDockSizes()
{
    bool fs = (windowState().testFlag(Qt::WindowMaximized) || windowState().testFlag(Qt::WindowFullScreen));

    QList<Qt::DockWidgetArea> areas;
    areas << Qt::LeftDockWidgetArea << Qt::RightDockWidgetArea << Qt::TopDockWidgetArea << Qt::BottomDockWidgetArea;
    QList<int> sizes = Settings::settings()->toIntList(fs ? skWinMaxSizes : skWinNormSizes);
    if (sizes.isEmpty()) sizes << -1 << -1 << -1 << -1; // (left, right, top, bottom) - (normal, full-size)

    QList<QDockWidget*> dwList;
    dwList << mGamsParameterEditor->extendedEditor() << ui->dockProjectView << ui->dockProcessLog << ui->dockHelpView;
    for (QDockWidget* dw : qAsConst(dwList)) {
        if (!dw->isVisible() || dw->isFloating()) continue;
        Qt::DockWidgetArea area = dockWidgetArea(dw);
        switch (area) {
        case Qt::LeftDockWidgetArea: sizes[0] = dw->width(); break;
        case Qt::RightDockWidgetArea: sizes[1] = dw->width(); break;
        case Qt::TopDockWidgetArea: sizes[2] = dw->height(); break;
        case Qt::BottomDockWidgetArea: sizes[3] = dw->height(); break;
        default: break;
        }
    }
    Settings::settings()->setIntList(fs ? skWinMaxSizes : skWinNormSizes, sizes);
}

void MainWindow::popDockSizes()
{
    bool fs = (windowState().testFlag(Qt::WindowMaximized) || windowState().testFlag(Qt::WindowFullScreen));
    QList<int> sizes = Settings::settings()->toIntList(fs ? skWinMaxSizes : skWinNormSizes);
    if (sizes.isEmpty()) return;

    QList<QDockWidget*> dwList;
    dwList << mGamsParameterEditor->extendedEditor() << ui->dockProjectView << ui->dockProcessLog << ui->dockHelpView;

    QList<QDockWidget*> dwResizeH;
    QList<int> dwSizesH;
    QList<QDockWidget*> dwResizeV;
    QList<int> dwSizesV;
    for (QDockWidget* dw : qAsConst(dwList)) {
        if (!dw->isVisible() || dw->isFloating()) continue;
        Qt::DockWidgetArea area = dockWidgetArea(dw);
        switch (area) {
        case Qt::LeftDockWidgetArea: if (sizes.at(0) >= 0) {
                dwResizeH << dw;
                dwSizesH << sizes.at(0);
            } break;
        case Qt::RightDockWidgetArea: if (sizes.at(1) >= 0) {
                dwResizeH << dw;
                dwSizesH << sizes.at(1);
            } break;
        case Qt::TopDockWidgetArea: if (sizes.at(2) >= 0) {
                dwResizeV << dw;
                dwSizesV << sizes.at(2);
            } break;
        case Qt::BottomDockWidgetArea: if (sizes.at(3) >= 0) {
                dwResizeV << dw;
                dwSizesV << sizes.at(3);
            } break;
        default: break;
        }
    }
    if (!dwResizeH.isEmpty()) resizeDocks(dwResizeH, dwSizesH, Qt::Horizontal);
    if (!dwResizeV.isEmpty()) resizeDocks(dwResizeV, dwSizesV, Qt::Vertical);
}

void MainWindow::focusCmdLine()
{
    raise();
    activateWindow();
    mGamsParameterEditor->focus();
}

void MainWindow::focusProjectExplorer()
{
    setProjectViewVisibility(true);
    ui->dockProjectView->activateWindow();
    ui->dockProjectView->raise();
    ui->projectView->setFocus();
}

void MainWindow::focusCentralWidget()
{
    if (mRecent.editor()) {
        raise();
        activateWindow();
        mRecent.editor()->setFocus();
    } else if (mWp->isVisible()) {
        raise();
        activateWindow();
        mWp->setFocus();
    }
}

void MainWindow::focusProcessLogs()
{
    setOutputViewVisibility(true);
    ui->dockProcessLog->activateWindow();
    ui->dockProcessLog->raise();
    if (!ui->logTabs->currentWidget())
        on_actionShow_System_Log_triggered();

    ui->logTabs->currentWidget()->setFocus();
}

void MainWindow::updateStatusFile()
{
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editFileId());
    if (fm) {
        mStatusWidgets->setFileName(QDir::toNativeSeparators(fm->location()));
        mStatusWidgets->setEncoding(fm->codecMib());
    }
}

void MainWindow::updateStatusPos()
{
    QPoint pos;
    QPoint anchor;
    if (CodeEdit *ce = ViewHelper::toCodeEdit(mRecent.editor())) {
        ce->getPositionAndAnchor(pos, anchor);
    } else if (AbstractEdit* edit = ViewHelper::toAbstractEdit(mRecent.editor())) {
        QTextCursor cursor = edit->textCursor();
        pos = QPoint(cursor.positionInBlock()+1, cursor.blockNumber()+1);
        if (cursor.hasSelection()) {
            cursor.setPosition(cursor.anchor());
            anchor = QPoint(cursor.positionInBlock()+1, cursor.blockNumber()+1);
        }
    } else if (TextView *tv = ViewHelper::toTextView(mRecent.editor())) {
        pos = tv->position() + QPoint(1,1);
        anchor = tv->anchor() + QPoint(1,1);
    }
    mStatusWidgets->setPosAndAnchor(pos, anchor);
}

void MainWindow::updateStatusMode()
{
    CodeEdit* edit = ViewHelper::toCodeEdit(mRecent.editor());
    if (ViewHelper::toSolverOptionEdit(mRecent.editor())) {
        mStatusWidgets->setEditMode(StatusWidgets::EditMode::Insert);
    } else if (ViewHelper::toGamsConfigEditor(mRecent.editor())) {
        mStatusWidgets->setEditMode(StatusWidgets::EditMode::Insert);
    } else if (!edit || edit->isReadOnly()) {
        mStatusWidgets->setEditMode(StatusWidgets::EditMode::Readonly);
    } else {
        mStatusWidgets->setEditMode(edit->overwriteMode() ? StatusWidgets::EditMode::Overwrite
                                                          : StatusWidgets::EditMode::Insert);
    }
}

void MainWindow::updateStatusLineCount()
{
    if (AbstractEdit* edit = ViewHelper::toAbstractEdit(mRecent.editor()))
        mStatusWidgets->setLineCount(edit->blockCount());
    else if (TextView *tv = ViewHelper::toTextView(mRecent.editor()))
        mStatusWidgets->setLineCount(tv->lineCount());
    else if (option::SolverOptionWidget* edit = ViewHelper::toSolverOptionEdit(mRecent.editor()))
        mStatusWidgets->setLineCount(edit->getItemCount());
    else mStatusWidgets->setLineCount(-1);
}

void MainWindow::updateStatusLoadAmount()
{
    if (TextView *tv = ViewHelper::toTextView(mRecent.editor())) {
        qreal amount = qAbs(qreal(tv->knownLines()) / tv->lineCount());
        mStatusWidgets->setLoadAmount(amount);
    }
}

void MainWindow::openRecentFile()
{
    if (mRecent.editor())
        openFile(mFileMetaRepo.fileMeta(mRecent.editFileId()));
}

void MainWindow::currentDocumentChanged(int from, int charsRemoved, int charsAdded)
{
    if (!searchDialog()->search()->regex().pattern().isEmpty())
        searchDialog()->on_documentContentChanged(from, charsRemoved, charsAdded);
}

void MainWindow::getAdvancedActions(QList<QAction*>* actions)
{
    QList<QAction*> act(ui->menuAdvanced->actions());
    *actions = act;
}

void MainWindow::newFileDialog(QVector<PExProjectNode*> projects, const QString& solverName)
{
    QString path;
    if (!projects.isEmpty()) {
        path = projects.constFirst()->workDir();

    } else if (mRecent.editFileId() >= 0) {
        FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editFileId());
        if (fm) path = QFileInfo(fm->location()).path();
    }
    if (path.isEmpty()) path = currentPath();
    if (path.isEmpty()) path = ".";

    if (solverName.isEmpty()) {
        // find a free file name
        int nr = 1;
        while (QFileInfo(path, QString("new%1.gms").arg(nr)).exists()) ++nr;
        path += QString("/new%1.gms").arg(nr);
    } else {
        int nr = 1;
        QString suffix = "opt";
        QString filename = QString("%1.%2").arg(solverName, suffix);
        while (QFileInfo(path, filename).exists()) {
            ++nr;  // note: "op1" is invalid
            if (nr<10) suffix = "op";
            else if (nr<100) suffix = "o";
            else suffix = "";
            filename = QString("%1.%2%3").arg(solverName, suffix).arg(nr);
        }
        path += QString("/%1").arg(filename);
    }

    QString filePath = solverName.isEmpty()
                             ? QFileDialog::getSaveFileName(this, "Create new file...",
                                                            path,
                                                            ViewHelper::dialogFileFilterUserCreated().join(";;"), nullptr, QFileDialog::DontConfirmOverwrite)
                             : QFileDialog::getSaveFileName(this, QString("Create new %1 option file...").arg(solverName),
                                                            path, ViewHelper::dialogOptFileFilter(solverName),
                                                            nullptr, QFileDialog::DontConfirmOverwrite);
    if (filePath == "") return;
    QFileInfo fi(filePath);
    if (fi.suffix().isEmpty())
        filePath += (solverName.isEmpty() ? ".gms" : ".opt");

    QFile file(filePath);
    FileMeta *destFM = mFileMetaRepo.fileMeta(filePath);

    if (file.exists()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("File already existing");
        msgBox.setText("The file " + filePath + " already exists in your working directory.");
        msgBox.setInformativeText("What do you want to do with the existing file?");
        msgBox.setStandardButtons(QMessageBox::Abort);
        msgBox.addButton("Open", QMessageBox::ActionRole);
        msgBox.addButton("Replace", QMessageBox::ActionRole);
        int answer = msgBox.exec();

        switch(answer) {
        case 0: // open
            // do nothing and continue
            break;
        case 1: // replace
            if (destFM)
               closeFileEditors(destFM->id());
            file.open(QIODevice::WriteOnly); // create empty file
            file.close();
            break;
        case QMessageBox::Abort:
            return;
        }
    } else {
        file.open(QIODevice::WriteOnly); // create empty file
        file.close();
    }

    if (!projects.isEmpty()) { // add file to each selected project
        for (PExProjectNode *project: projects)
            openFileNode(addNode("", filePath, project));
    } else { // create new project
        PExProjectNode *project = mProjectRepo.createProject(fi.completeBaseName(), fi.absolutePath(), "");
        PExFileNode* node = addNode("", filePath, project);
        openFileNode(node);
        setMainGms(node); // does nothing if file is not of type gms
    }
}

void MainWindow::on_menuFile_aboutToShow()
{
    ui->actionPrint->setEnabled(enabledPrintAction());
}

void MainWindow::on_actionNew_triggered()
{
    QVector<PExProjectNode*> project;
    if (Settings::settings()->toBool(skOpenInCurrent) && mRecent.project())
        project << mRecent.project();
    newFileDialog(project);
}

void MainWindow::on_actionOpen_triggered()
{
    openFiles(Settings::settings()->toBool(skOpenInCurrent) ? ogCurrentGroup : ogFindGroup);
}

void MainWindow::on_actionOpenAlternative_triggered()
{
    openFiles(Settings::settings()->toBool(skOpenInCurrent) ? ogNewGroup : ogCurrentGroup);
}

void MainWindow::on_actionOpen_Folder_triggered()
{
    const QString folder = QFileDialog::getExistingDirectory(this, "Open Folder", currentPath(),
                                                                QFileDialog::ShowDirsOnly);
    openFolder(folder);
}

void MainWindow::openFolder(QString path, PExProjectNode *project)
{
    if (path.isEmpty()) return;

    QDir dir(path);
    QDirIterator dirIter(dir, QDirIterator::Subdirectories);

    QSet<QString> allFiles;
    while (dirIter.hasNext()) {
        QFileInfo f(dirIter.next());

        if (f.isFile())
            allFiles.insert(f.absoluteFilePath());
    }

    if (allFiles.count() > 499) {
        QMessageBox msgBox(this);
        msgBox.setText("Warning");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(path + " contains " + QString::number(allFiles.count())
                       + " files. Adding that many files can take a long time to complete.\n"
                       + "Do you want to continue?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if (msgBox.exec() == QMessageBox::No) return; // abort
    }

    if (!project) {
        if (Settings::settings()->toBool(skOpenInCurrent) && mRecent.project())
            project = mRecent.project();
        else
            project = projectRepo()->createProject(dir.dirName(), path, "");
    }

    foreach(QString file, allFiles)
        projectRepo()->findOrCreateFileNode(file, project);
}

void MainWindow::on_actionSave_triggered()
{
    FileMeta* fm = mFileMetaRepo.fileMeta(mRecent.editFileId());
    if (!fm) return;

    if (fm->isModified() && !fm->isReadOnly())
        fm->save();
    else if (fm->isReadOnly())
        on_actionSave_As_triggered();

}

void MainWindow::on_actionSave_As_triggered()
{
    PExFileNode *node = mProjectRepo.findFileNode(mRecent.editor());
    if (!node) return;
    FileMeta *fileMeta = node->file();
    int choice = 0;
    QString filePath = fileMeta->location();
    QFileInfo fi(filePath);
    while (choice < 1) {
        QStringList filters;
        if (fileMeta->kind() == FileKind::Opt) {
            filters << ViewHelper::dialogOptFileFilter(fi.baseName());
            filePath = QFileDialog::getSaveFileName(this, "Save file as...",
                                                    filePath, filters.join(";;"),
                                                    &filters.first(),
                                                    QFileDialog::DontConfirmOverwrite);
        } else {
            filters = ViewHelper::dialogFileFilterAll();

            QString selFilter = filters.first();
            for (const QString &f: qAsConst(filters)) {
                if (f.contains("*."+fi.suffix())) {
                    selFilter = f;
                    break;
                }
            }

            filePath = QFileDialog::getSaveFileName(this, "Save file as...",
                                                    filePath, filters.join(";;"),
                                                    &selFilter,
                                                    QFileDialog::DontConfirmOverwrite);
        }
        if (filePath.isEmpty()) return;

        choice = 1;
        if ( fileMeta->kind() == FileKind::Opt  &&
             QString::compare(fi.baseName(), QFileInfo(filePath).completeBaseName(), Qt::CaseInsensitive)!=0 )
            choice = QMessageBox::question(this, "Different solver name"
                                               , QString("The option file name '%1' is different than source option file name '%2'. Saved file '%3' may not be displayed properly.")
                                                      .arg(QFileInfo(filePath).completeBaseName(), QFileInfo(fileMeta->location()).completeBaseName(), QFileInfo(filePath).fileName())
                                               , "Select other", "Continue", "Abort", 0, 2);
        if (choice == 0)
            continue;
        else if (choice == 2)
                 break;

        choice = 1;
        if (FileType::from(fileMeta->kind()) != FileType::from(QFileInfo(filePath).fileName())) {
            if (fileMeta->kind() == FileKind::Opt) {
                choice = QMessageBox::question(this, "Invalid Option File Name or Suffix"
                                                   , QString("'%1' is not a valid solver option file name or suffix. File Saved as '%2' may not be displayed properly.")
                                                          .arg(QFileInfo(filePath).suffix(), QFileInfo(filePath).fileName())
                                                   , "Select other", "Continue", "Abort", 0, 2);
            } else if (fileMeta->kind() == FileKind::Guc) {
                choice = QMessageBox::question(this, "Invalid Gams Configuration File Name or Suffix"
                                                   , QString("'%1' is not a valid Gams configuration file name or suffix. File saved as '%1' may not be displayed properly.")
                                                          .arg(QFileInfo(filePath).fileName())
                                                   , "Select other", "Continue", "Abort", 0, 2);
            } else {
                choice = QMessageBox::question(this, "Different File Type"
                                                   , QString("Target '%1'' is of different type than the type of source '%2'. File saved as '%1' may not be displayed properly.")
                                                          .arg(QFileInfo(filePath).fileName(), QFileInfo(fileMeta->location()).fileName())
                                                   , "Select other", "Continue", "Abort", 0, 2);
            }
        }
        if (choice == 0)
            continue;
        else if (choice == 2)
                 break;

        // perform file copy when file is either a gdx file or a ref file
        bool exists = QFile::exists(filePath);
        if ((fileMeta->kind() == FileKind::Gdx) || (fileMeta->kind() == FileKind::Ref))  {
            choice = 1;
            if (exists) {
                choice = QMessageBox::question(this, "File exists", filePath+" already exists."
                                               , "Select other", "Overwrite", "Abort", 0, 2);
                if (choice == 1 && fileMeta->location() != filePath)
                    QFile::remove(filePath);
            }
            if (choice == 1)
                QFile::copy(fileMeta->location(), filePath);
        } else {
            FileMeta *destFM = mFileMetaRepo.fileMeta(filePath);
            choice = (destFM && destFM->isModified()) ? -1 : exists ? 0 : 1;
            if (choice < 0)
                choice = QMessageBox::question(this, "Destination file modified"
                                               , QString("Your unsaved changes on %1 will be lost.").arg(filePath)
                                               , "Select other", "Continue", "Abort", 0, 2);
            else if (choice < 1)
                choice = QMessageBox::question(this, "File exists", filePath+" already exists."
                                               , "Select other", "Overwrite", "Abort", 0, 2);

            if (choice == 1) {
                FileKind oldKind = node->file()->kind();

                // when overwriting a node, remove existing to prevent project explorer to contain two identical entries
                if (PExFileNode* pfn = mProjectRepo.findFile(filePath, node->assignedProject()))
                    mProjectRepo.closeNode(pfn);

                mProjectRepo.saveNodeAs(node, filePath);

                if (oldKind == node->file()->kind()) { // if old == new
                    ui->mainTabs->tabBar()->setTabText(ui->mainTabs->currentIndex(), fileMeta->name(NameModifier::raw));
                    ViewHelper::setModified(ui->mainTabs->currentWidget(), false);
                } else { // reopen in new editor
                    int index = ui->mainTabs->currentIndex();
                    openFileNode(node, true);
                    on_mainTabs_tabCloseRequested(index);
                }
                updateStatusFile();
                updateAndSaveSettings();
            }
        }
        if (choice == 1) {
//            mRecent.path = QFileInfo(filePath).path();
            PExFileNode* newNode =
                    mProjectRepo.findOrCreateFileNode(filePath, node->assignedProject());
            openFileNode(newNode, true);
        }
    }
    mProjectRepo.treeModel()->sortChildNodes(node->parentNode());
}

void MainWindow::on_actionSave_All_triggered()
{
    for (FileMeta* fm: mFileMetaRepo.openFiles())
        fm->save();
}

void MainWindow::on_actionClose_triggered()
{
    on_mainTabs_tabCloseRequested(ui->mainTabs->currentIndex());
}

void MainWindow::on_actionClose_All_triggered()
{
    disconnect(ui->mainTabs, &QTabWidget::currentChanged, this, &MainWindow::activeTabChanged);
    if (ui->mainTabs->count() > 1)
        ui->mainTabs->tabBar()->moveTab(ui->mainTabs->currentIndex(), ui->mainTabs->count()-1);

    for(int i = ui->mainTabs->count(); i > 0; i--) {
        on_mainTabs_tabCloseRequested(0);
    }
    connect(ui->mainTabs, &QTabWidget::currentChanged, this, &MainWindow::activeTabChanged);
}

void MainWindow::on_actionClose_All_Except_triggered()
{
    int except = ui->mainTabs->currentIndex();
    for(int i = ui->mainTabs->count(); i >= 0; i--) {
        if(i != except) {
            on_mainTabs_tabCloseRequested(i);
        }
    }
}

void MainWindow::codecChanged(QAction *action)
{
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editFileId());
    if (fm) {
        if (!fm->isReadOnly()) {
            fm->setCodecMib(action->data().toInt());
        }
        updateMenuToCodec(action->data().toInt());
        updateStatusFile();
    }
}

void MainWindow::codecReload(QAction *action)
{
    if (!focusWidget()) return;
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editFileId());
    if (fm && (fm->kind() == FileKind::Log || fm->kind() == FileKind::Guc || fm->kind() == FileKind::PrO))
        return;
    if (fm && fm->codecMib() != action->data().toInt()) {
        bool reload = true;
        if (fm->isModified()) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(QDir::toNativeSeparators(fm->location())+" has been modified.");
            msgBox.setInformativeText("Do you want to discard your changes and reload it with Character Set "
                                      + action->text() + "?");
            msgBox.addButton(tr("Discard and Reload"), QMessageBox::ResetRole);
            msgBox.setStandardButtons(QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            reload = msgBox.exec();
        }
        if (reload) {
            fm->load(action->data().toInt());
            updateMenuToCodec(fm->codecMib());
            updateStatusFile();
        }
    }
}

void MainWindow::loadCommandLines(PExFileNode* oldfn, PExFileNode* fn)
{
    if (oldfn && oldfn != fn) {
        // node changed from valid: store current command-line
        if (PExProjectNode* oldgroup = oldfn->assignedProject())
            oldgroup->addRunParametersHistory( mGamsParameterEditor->getCurrentCommandLineData());
    }

    if (fn) {
        // switched to valid node
        if (PExProjectNode* group = fn->assignedProject())
            mGamsParameterEditor->loadCommandLine( group->getRunParametersHistory() );
    } else {
        // switched to welcome page
        mGamsParameterEditor->loadCommandLine(QStringList());
    }
}

void MainWindow::activeTabChanged(int index)
{
    QWidget *editWidget = (index < 0 ? nullptr : ui->mainTabs->widget(index));
    PExFileNode* oldNode = mProjectRepo.findFileNode(mRecent.editor());
    PExFileNode* node = mProjectRepo.findFileNode(editWidget);
    updateRecentEdit(mRecent.editor(), editWidget);
    if (CodeEdit* ce = ViewHelper::toCodeEdit(editWidget))
        ce->updateExtraSelections();
    else if (TextView* tv = ViewHelper::toTextView(editWidget))
        tv->updateExtraSelections();

    if (node) {
        changeToLog(node, false, false);
        updateStatusFile();

        if (AbstractEdit* edit = ViewHelper::toAbstractEdit(editWidget)) {
            ui->menuEncoding->setEnabled(node && !edit->isReadOnly());
            ui->menuconvert_to->setEnabled(node && !edit->isReadOnly());
        } else if (ViewHelper::toTextView(editWidget)) {
            ui->menuEncoding->setEnabled(true);
            ui->menuconvert_to->setEnabled(false);
        } else if (ViewHelper::toGdxViewer(editWidget)) {
            ui->menuconvert_to->setEnabled(false);
            node->file()->reload();
        } else if (reference::ReferenceViewer* refViewer = ViewHelper::toReferenceViewer(editWidget)) {
            ui->menuEncoding->setEnabled(false);
            ui->menuconvert_to->setEnabled(false);
            if (mProjectRepo.findFileNode(refViewer)) {
                ui->menuconvert_to->setEnabled(false);
                updateMenuToCodec(node->file()->codecMib());
            }
        } else if (option::SolverOptionWidget* solverOptionEditor = ViewHelper::toSolverOptionEdit(editWidget)) {
            ui->menuEncoding->setEnabled(false);
            if (mProjectRepo.findFileNode(solverOptionEditor)) {
                ui->menuEncoding->setEnabled(true);
                ui->menuconvert_to->setEnabled(true);
                node->file()->reload();
                updateMenuToCodec(node->file()->codecMib());
            }
        } else if (option::GamsConfigEditor* gucEditor = ViewHelper::toGamsConfigEditor((editWidget))) {
            ui->menuEncoding->setEnabled(false);
            if (mProjectRepo.findFileNode(gucEditor)) {
                ui->menuEncoding->setEnabled(false);
                ui->menureload_with->setEnabled(false);
                ui->menuconvert_to->setEnabled(false);
                node->file()->reload();
                updateMenuToCodec(node->file()->codecMib());
            }
        }
        updateMenuToCodec(node->file()->codecMib());
    } else {
        ui->menuEncoding->setEnabled(false);
    }

    loadCommandLines(oldNode, node);
    updateRunState();
    searchDialog()->updateDialogState();
    updateToolbar(mainTabs()->currentWidget());

    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly()) ce->setOverwriteMode(mOverwriteMode);
    updateStatusMode();
    updateStatusFile();
    updateStatusLineCount();
    updateStatusLoadAmount();
    updateStatusPos();

    updateCursorHistoryAvailability();
}

void MainWindow::tabBarClicked(int index)
{
    Q_UNUSED(index)
    QTabWidget *tabs = sender() == ui->mainTabs ? ui->mainTabs : sender() == ui->logTabs ? ui->logTabs : nullptr;
    if (tabs && tabs->currentWidget()) tabs->currentWidget()->setFocus();
}

void MainWindow::fileChanged(const FileId fileId)
{
    mProjectRepo.fileChanged(fileId);
    FileMeta *fm = mFileMetaRepo.fileMeta(fileId);
    if (!fm) return;
    for (QWidget *edit: fm->editors()) {
        if (mPinView->widget() == edit) {
            ViewHelper::setModified(edit, fm->isModified());
            mPinView->setFileName(fm->name(NameModifier::editState), QDir::toNativeSeparators(fm->location()));
        } else {
            int index = ui->mainTabs->indexOf(edit);
            if (index >= 0) {
                ViewHelper::setModified(edit, fm->isModified());
                fm->updateTabName(ui->mainTabs, index);
            }
        }
    }
}

void MainWindow::fileClosed(const FileId fileId)
{
    Q_UNUSED(fileId)
}

FileProcessKind MainWindow::fileChangedExtern(FileId fileId)
{
    FileMeta *file = mFileMetaRepo.fileMeta(fileId);
    // file has not been loaded: nothing to do
    if (!file->isOpen()) return FileProcessKind::ignore;
    if (file->kind() == FileKind::Log) return FileProcessKind::ignore;
    if (file->kind() == FileKind::Gdx) {
        QFile f(file->location());
        if (!f.exists()) return FileProcessKind::fileLocked;
        bool changed = file->refreshMetaData();
        for (QWidget *e : file->editors()) {
            if (gdxviewer::GdxViewer *gv = ViewHelper::toGdxViewer(e)) {
                gv->setHasChanged(true);
                int gdxErr = gv->reload(file->codec(), changed, false);
                if (gdxErr) return (gdxErr==-1 ? FileProcessKind::fileBecameInvalid : gdxErr==-2 ? FileProcessKind::fileLocked : FileProcessKind::ignore);
            }
        }
        return FileProcessKind::ignore;
    }
    if (file->kind() == FileKind::Opt) {
        for (QWidget *e : file->editors()) {
            if (option::SolverOptionWidget *sow = ViewHelper::toSolverOptionEdit(e))
               sow->setFileChangedExtern(true);
        }
    }
    if (file->kind() == FileKind::Guc) {
        for (QWidget *e : file->editors()) {
            if (option::GamsConfigEditor *guce = ViewHelper::toGamsConfigEditor(e))
               guce->setFileChangedExtern(true);
        }
    }
    if (!file->isModified() && file->isAutoReload()) {
        file->reload();
        return FileProcessKind::ignore;
    }
    return file->isModified() ? FileProcessKind::changedConflict : FileProcessKind::changedExternOnly;
}

FileProcessKind MainWindow::fileDeletedExtern(FileId fileId)
{
    FileMeta *file = mFileMetaRepo.fileMeta(fileId);
    if (!file) return FileProcessKind::ignore;
    if (file->exists(true)) return FileProcessKind::ignore;
    mTextMarkRepo.removeMarks(fileId, QSet<TextMark::Type>() << TextMark::all);
    if (!file->isOpen()) {
        const QVector<PExFileNode*> nodes = mProjectRepo.fileNodes(file->id());
        for (PExFileNode* node: nodes)
            mProjectRepo.closeNode(node);
        mHistory.files().removeAll(file->location());
        historyChanged();
        return FileProcessKind::ignore;
    }
    return FileProcessKind::removedExtern;
}

void MainWindow::fileEvent(const FileEvent &e)
{
    FileMeta *fm = mFileMetaRepo.fileMeta(e.fileId());
    if (!fm) return;
    if (e.kind() == FileEventKind::changed)
        fileChanged(e.fileId()); // Just update display kind
    else if (e.kind() == FileEventKind::created)
        fileChanged(e.fileId()); // Just update display kind
    else if (e.kind() == FileEventKind::closed)
        fileClosed(e.fileId());
    else {
        fileChanged(e.fileId()); // First update display kind
        fm->invalidate();

        // file handling with user-interaction are delayed
        {
            QMutexLocker locker(&mFileMutex);
            FileEventData data = e.data();
            if (mFileEvents.contains(data))
                mFileEvents.removeAll(data);
            mFileEvents << data;
        }

        // prevent delaying file processing of events from other files
        if (!mFileTimer.isActive())
            mFileTimer.start();
    }
}

void MainWindow::logTabRenamed(QWidget *wid, const QString &newName)
{
    int ind = ui->logTabs->indexOf(wid);
    if (ind < 0) return;
    ui->logTabs->setTabText(ind, newName);
}

void MainWindow::processFileEvents()
{
    static bool active = false;
    if (mFileEvents.isEmpty()) return;
    // Pending events but window is not active: wait and retry
    if (!isActiveWindow() || active) {
        mFileTimer.start();
        return;
    }
    active = true;

    // First process all events that need no user decision. For the others: remember the kind of change
    QSet<FileEventData> scheduledEvents;
    QMap<FileProcessKind, QVector<FileEventData>> remainEvents;
    while (true) {
        FileEventData fileEvent;
        { // lock only while accessing mFileEvents
            QMutexLocker locker(&mFileMutex);
            if (mFileEvents.isEmpty()) break;
            fileEvent = mFileEvents.takeFirst();
        }
        FileMeta *fm = mFileMetaRepo.fileMeta(fileEvent.fileId);
        if (!fm || fm->kind() == FileKind::Log)
            continue;
        int elapsed = fileEvent.time.msecsTo(QTime().currentTime());
        if (elapsed < 100) {
            // always add latest event
            QSet<FileEventData>::const_iterator it = scheduledEvents.constFind(fileEvent);
            if (it != scheduledEvents.constEnd() && it->time < fileEvent.time)
                scheduledEvents -= fileEvent;
            scheduledEvents += fileEvent;
            continue;
        }
        FileProcessKind remainKind = FileProcessKind::ignore;
        switch (fileEvent.kind) {
        case FileEventKind::changedExtern:
            remainKind = fileChangedExtern(fm->id());
            break;
        case FileEventKind::removedExtern:
            remainKind = fileDeletedExtern(fm->id());
            break;
        default: break;
        }
        if (remainKind != FileProcessKind::ignore) {
            if (!remainEvents.contains(remainKind)) remainEvents.insert(remainKind, QVector<FileEventData>());
            if (!remainEvents[remainKind].contains(fileEvent)) remainEvents[remainKind] << fileEvent;
        }
    }

    // Then ask what to do with the files of each remainKind
    for (auto key: remainEvents.keys()) {
        switch (key) {
        case FileProcessKind::changedExternOnly: // changed externally but unmodified internally
        case FileProcessKind::changedConflict: // changed externally and modified internally
            mFileEventHandler->process(FileEventHandler::Change, remainEvents.value(key));
            break;
        case FileProcessKind::removedExtern: // removed externally
            mFileEventHandler->process(FileEventHandler::Deletion, remainEvents.value(key));
            break;
        case FileProcessKind::fileLocked: // file is locked: reschedule event
            for (auto event: remainEvents.value(key))
                scheduledEvents << event;
            break;
        case FileProcessKind::fileBecameInvalid: // file is invalid: close it
            for (const FileEventData &ed : remainEvents.value(key)) {
                closeFileEditors(ed.fileId);
            }
            break;
        default:
            break;
        }
    }

    // add events that have been skipped due too early processing
    if (!scheduledEvents.isEmpty()) {
        QMutexLocker locker(&mFileMutex);
        for (const FileEventData &data : qAsConst(scheduledEvents)) {
            int i = mFileEvents.indexOf(data);
            if (i >= 0) {
                if (mFileEvents.at(i).time > data.time) continue;
                mFileEvents.removeAt(i);
            }
            mFileEvents << data;
        }
        mFileTimer.start();
    }

    active = false;
}

void MainWindow::appendSystemLogInfo(const QString &text) const
{
    mSyslog->append(text, LogMsgType::Info);
}

void MainWindow::appendSystemLogError(const QString &text) const
{
    mSyslog->append(text, LogMsgType::Error);
}

void MainWindow::appendSystemLogWarning(const QString &text) const
{
    mSyslog->append(text, LogMsgType::Warning);
}

void MainWindow::postGamsRun(NodeId origin, int exitCode)
{
    if (origin == -1) {
        appendSystemLogError("No fileId set to process");
        return;
    }
    PExProjectNode* project = mProjectRepo.findProject(origin);
    if (!project) {
        appendSystemLogError("No group attached to process");
        return;
    }

    if (exitCode == ecTooManyScratchDirs) {
        PExProjectNode* project = mProjectRepo.findProject(ViewHelper::groupId(mRecent.editor()));
        QString path = project ? QDir::toNativeSeparators(project->workDir()) : currentPath();

        // TODO fix QDialog::exec() issue
        QMessageBox msgBox;
        msgBox.setWindowTitle("Delete scratch directories");
        msgBox.setText("GAMS was unable to run because there are too many scratch directories "
                       "in the current workspace folder. Clean up your workspace and try again.\n"
                       "The current working directory is " + path);
        msgBox.setInformativeText("Delete scratch directories now?");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        if (msgBox.exec() == QMessageBox::Yes)
            deleteScratchDirs(path);
    }

    // add all created files to project explorer
    project->addNodesForSpecialFiles();

    FileMeta *runMeta = project->runnableGms();
    if (!runMeta) {
        appendSystemLogError("Invalid runable attached to process");
        return;
    }
    if (project && project->hasLogNode()) {
        PExLogNode *logNode = project->logNode();
        logNode->logDone();
        if (logNode->file()->editors().size()) {
            if (TextView* tv = ViewHelper::toTextView(logNode->file()->editors().first())) {
                if (Settings::settings()->toBool(skJumpToError)) {
                    int errLine = tv->firstErrorLine();
                    if (errLine >= 0) tv->jumpTo(errLine, 0);
                }
                MainWindow::disconnect(tv, &TextView::selectionChanged, this, &MainWindow::updateStatusPos);
                MainWindow::disconnect(tv, &TextView::blockCountChanged, this, &MainWindow::updateStatusLineCount);
                MainWindow::disconnect(tv, &TextView::loadAmountChanged, this, &MainWindow::updateStatusLoadAmount);
            }
        }
    }
    if (project && runMeta->exists(true)) {
        QString lstFile = project->parameter("ls2");
        mProjectRepo.findOrCreateFileNode(lstFile, project);
        lstFile = project->parameter("lst");
        bool doFocus = (project == mRecent.project());
        PExFileNode* lstNode = mProjectRepo.findOrCreateFileNode(lstFile, project);

        if (lstNode)
            for (QWidget *edit: lstNode->file()->editors())
                if (TextView* tv = ViewHelper::toTextView(edit)) tv->endRun();

        bool alreadyJumped = false;
        if (Settings::settings()->toBool(skJumpToError))
            alreadyJumped = project->jumpToFirstError(doFocus, lstNode);

        if (lstNode && !alreadyJumped && Settings::settings()->toBool(skOpenLst))
            openFileNode(lstNode);
    }
    updateRunState();
}

void MainWindow::postGamsLibRun()
{
    if(mLibProcess->exitCode() != 0) {
        SysLogLocator::systemLog()->append("Error retrieving model from model library: gamslib returned with exit code " + QString::number(mLibProcess->exitCode()),LogMsgType::Error);
        if (mLibProcess) {
            mLibProcess->deleteLater();
            mLibProcess = nullptr;
        }
        return;
    }
    PExProjectNode* project = (Settings::settings()->toBool(skOpenInCurrent)
                               && mRecent.project()) ? mRecent.project() : nullptr;
    PExFileNode *node = mProjectRepo.findFile(mLibProcess->workingDirectory() + "/" + mLibProcess->inputFile(), project);
    if (!node)
        node = addNode(mLibProcess->workingDirectory(), mLibProcess->inputFile(), project);
    if (node) mFileMetaRepo.watch(node->file());
    if (node && !node->file()->editors().isEmpty()) {
        if (node->file()->kind() != FileKind::Log && node->file()->kind() != FileKind::PrO)
            node->file()->load(node->file()->codecMib());
    }
    openFileNode(node);
    if (mLibProcess) {
        mLibProcess->deleteLater();
        mLibProcess = nullptr;
    }
}

void MainWindow::on_actionExit_Application_triggered()
{
    close();
}

void MainWindow::on_actionGamsHelp_triggered()
{
#ifdef QWEBENGINE
    QWidget* widget = focusWidget();
    if (mGamsParameterEditor->isAParameterEditorFocused(widget)) {
        QString optionName = mGamsParameterEditor->getSelectedParameterName(widget);
        if (optionName.isEmpty())
            mHelpWidget->on_helpContentRequested( help::DocumentType::StudioMain, "",
                                                  help::HelpData::getStudioSectionName(help::StudioSection::OptionEditor));
        else
            mHelpWidget->on_helpContentRequested( help::DocumentType::GamsCall, optionName);
    } else if (ui->projectView->hasFocus()) {
        auto section = help::HelpData::getStudioSectionName(help::StudioSection::ProjectExplorer);
        mHelpWidget->on_helpContentRequested(help::DocumentType::StudioMain, "", section);
    } else {
        QWidget *wid = focusWidget();
        while (wid && wid != ui->logTabs) wid = wid->parentWidget();

        if (wid) {
            auto section = help::HelpData::getStudioSectionName(help::StudioSection::ProcessLog);
            mHelpWidget->on_helpContentRequested(help::DocumentType::StudioMain, "", section);
        } else {
            QWidget* editWidget = mRecent.editor();
            if (editWidget) {
                FileMeta* fm = mFileMetaRepo.fileMeta(editWidget);
                if (!fm) {
                    mHelpWidget->on_helpContentRequested( help::DocumentType::StudioMain, "",
                                                          help::HelpData::getStudioSectionName(help::StudioSection::WelcomePage));
                } else {
                    if (mRecent.editor() != nullptr) {
                        if (widget == mRecent.editor()) {
                           CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
                           if (ce) {
                               QString word;
                               int iKind = 0;
                               ce->wordInfo(ce->textCursor(), word, iKind);

                               if (iKind == static_cast<int>(syntax::SyntaxKind::Title)) {
                                   mHelpWidget->on_helpContentRequested(help::DocumentType::DollarControl, "title");
                               } else if (iKind == static_cast<int>(syntax::SyntaxKind::Dco)) {
                                   mHelpWidget->on_helpContentRequested(help::DocumentType::DollarControl, word);
                               } else {
                                   mHelpWidget->on_helpContentRequested(help::DocumentType::Index, word);
                               }
                            }
                        } else {
                            option::SolverOptionWidget* optionEdit =  ViewHelper::toSolverOptionEdit(mRecent.editor());
                            if (optionEdit) {
                                QString optionName = optionEdit->getSelectedOptionName(widget);
                                if (optionName.isEmpty())
                                    mHelpWidget->on_helpContentRequested( help::DocumentType::StudioMain, "",
                                                                          help::HelpData::getStudioSectionName(help::StudioSection::SolverOptionEditor));
                                else
                                    mHelpWidget->on_helpContentRequested( help::DocumentType::Solvers, optionName,
                                                                          optionEdit->getSolverName());
                            } else if (ViewHelper::toGdxViewer(mRecent.editor())) {
                                       mHelpWidget->on_helpContentRequested( help::DocumentType::StudioMain, "",
                                                                             help::HelpData::getStudioSectionName(help::StudioSection::GDXViewer));
                            } else if (ViewHelper::toReferenceViewer(mRecent.editor())) {
                                       mHelpWidget->on_helpContentRequested( help::DocumentType::StudioMain, "",
                                                                             help::HelpData::getStudioSectionName(help::StudioSection::ReferenceFileViewer));
                            } else if (ViewHelper::toLxiViewer(mRecent.editor())) {
                                       mHelpWidget->on_helpContentRequested( help::DocumentType::StudioMain, "",
                                                                             help::HelpData::getStudioSectionName(help::StudioSection::ListingViewer));
                            } else if (option::GamsConfigEditor* editor = ViewHelper::toGamsConfigEditor(mRecent.editor())) {
                                      QString optionName = editor->getSelectedParameterName(widget);
                                     if (optionName.isEmpty())
                                         mHelpWidget->on_helpContentRequested( help::DocumentType::StudioMain, "",
                                                                               help::HelpData::getStudioSectionName(help::StudioSection::GamsUserConfigEditor));
                                     else
                                         mHelpWidget->on_helpContentRequested( help::DocumentType::GamsCall, optionName);
                            } else {
                                mHelpWidget->on_helpContentRequested( help::DocumentType::Main, "");
                            }
                        }
                    } else {
                        mHelpWidget->on_helpContentRequested( help::DocumentType::Main, "");
                    }
                }
            } else {
                mHelpWidget->on_helpContentRequested( help::DocumentType::Main, "");
            }
        }
    }

    if (ui->dockHelpView->isHidden())
        ui->dockHelpView->show();
    if (tabifiedDockWidgets(ui->dockHelpView).count())
        ui->dockHelpView->raise();
#endif
}

void MainWindow::on_actionStudioHelp_triggered()
{
#ifdef QWEBENGINE
    mHelpWidget->on_helpContentRequested( help::DocumentType::StudioMain,
                                          QString(),
                                          QString());

    if (ui->dockHelpView->isHidden())
        ui->dockHelpView->show();
    if (tabifiedDockWidgets(ui->dockHelpView).count())
        ui->dockHelpView->raise();
#endif
}

void MainWindow::on_actionAbout_Studio_triggered()
{
    QMessageBox about(this);
    about.setWindowTitle(ui->actionAbout_Studio->text());
    about.setTextFormat(Qt::RichText);
    about.setText(support::GamsLicensingDialog::header());
    about.setInformativeText(support::GamsLicensingDialog::aboutStudio());
    about.setIconPixmap(Theme::icon(":/img/gams-w24").pixmap(QSize(64, 64)));
    about.addButton(QMessageBox::Ok);
    about.exec();
}

void MainWindow::on_gamsLicensing_triggered()
{
    support::GamsLicensingDialog dialog(ui->gamsLicensing->text(), this);
    dialog.exec();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::on_actionChangelog_triggered()
{
    QString filePath = CommonPaths::changelog();
    QFile changelog(filePath);
    if (!changelog.exists()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setText("Changelog file was not found. You can find all the information on https://www.gams.com/latest/docs/");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    FileMeta* fm = mFileMetaRepo.findOrCreateFileMeta(filePath);
    fm->setKind(FileKind::TxtRO);
    openFile(fm, true);
}

void MainWindow::on_actionUpdate_triggered()
{
    support::UpdateDialog updateDialog(this);
    updateDialog.checkForUpdate();
    updateDialog.exec();
}

void MainWindow::on_actionTerminal_triggered()
{
    auto workingDir = currentPath();
    actionTerminalTriggered(workingDir);
}

void MainWindow::actionTerminalTriggered(const QString &workingDir)
{
    auto environment = QProcessEnvironment::systemEnvironment();

    QProcess process;
#if defined(__APPLE__)
    QString statement = "tell application \"Terminal\"\n"
                        "do script \"export PATH=$PATH:\\\"%1\\\" && cd %2\"\n"
                        "activate \"Terminal\"\n"
                        "end tell\n";
    process.setProgram("osascript");
    process.setArguments({"-e", statement.arg(CommonPaths::systemDir()).arg(workingDir)});
#elif defined(__unix__)
    QStringList terms = {"gnome-terminal", "konsole", "xfce-terminal", "xterm"};
    for (auto term: terms) {
        auto appPath = QStandardPaths::findExecutable(term);
        if (appPath.isEmpty()) {
            continue;
        } else {
            process.setProgram(appPath);
            break;
        }
    }
    process.setWorkingDirectory(workingDir);
    environment.insert("PATH", QDir::toNativeSeparators(CommonPaths::systemDir()) + ":" + environment.value("PATH"));
    environment.insert("LD_PRELOAD", "");
    environment.insert("LD_LIBRARY_PATH", "");
    process.setProcessEnvironment(environment);
    SysLogLocator::systemLog()->append("On some Linux distributions GAMS Studio may not be able to start a terminal.", LogMsgType::Info);
#else
    process.setProgram("cmd.exe");
    process.setArguments({"/k", "title", "GAMS Terminal"});
    process.setWorkingDirectory(workingDir);
    auto gamsDir = QDir::toNativeSeparators(CommonPaths::systemDir());
    process.setCreateProcessArgumentsModifier([] (QProcess::CreateProcessArguments *args)
    {
        args->flags |= CREATE_NEW_CONSOLE;
        args->startupInfo->dwFlags &= ulong(~STARTF_USESTDHANDLES);
    });
#endif
    if (!process.startDetached())
        appendSystemLogError("Error opening terminal: " + process.errorString());
}

void MainWindow::on_mainTabs_tabCloseRequested(int index)
{
    QWidget* widget = ui->mainTabs->widget(index);
    FileMeta* fc = mFileMetaRepo.fileMeta(widget);
    if (!fc) {
        // assuming we are closing a welcome page here
        ui->mainTabs->removeTab(index);
        mClosedTabs << "WELCOME_PAGE";
        mClosedTabsIndexes << index;
        return;
    }

    int ret = QMessageBox::Discard;
    if (fc->isModified()) {
        // only ask, if this is the last editor of this file
        ret = showSaveChangesMsgBox(ui->mainTabs->tabText(index)+" has been modified.");
    }

    if (ret == QMessageBox::Save) {
        mAutosaveHandler->clearAutosaveFiles(mOpenTabsList);
        fc->save();
        closeFileEditors(fc->id());
    } else if (ret == QMessageBox::Discard) {
        mAutosaveHandler->clearAutosaveFiles(mOpenTabsList);
        fc->setModified(false);
        closeFileEditors(fc->id());
    } else if (ret == QMessageBox::Cancel) {
        // do nothing
    }
    searchDialog()->updateDialogState();
}

int MainWindow::showSaveChangesMsgBox(const QString &text)
{
    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    return msgBox.exec();
}

void MainWindow::on_logTabs_tabCloseRequested(int index)
{
    bool isResults = ui->logTabs->widget(index) == resultsView();
    if (isResults) {
        mSearchDialog->clearResultsView();
        return;
    }

    QWidget* edit = ui->logTabs->widget(index);
    if (!edit) return;

    ui->logTabs->removeTab(index);

    // dont remove syslog and dont delete resultsView
    if (!(edit == mSyslog || isResults)) {
        FileMeta* log = mFileMetaRepo.fileMeta(edit);
        if (log) log->removeEditor(edit);
        edit->deleteLater();
    }
}

void MainWindow::showWelcomePage()
{
    ui->mainTabs->insertTab(0, mWp, QString("Welcome")); // always first position
    ui->mainTabs->setCurrentIndex(0); // go to welcome page

}

bool MainWindow::isActiveProjectRunnable()
{
    QWidget* editWidget = mRecent.editor();
    if (editWidget) {
       FileMeta* fm = mFileMetaRepo.fileMeta(editWidget);
       if (!fm) { // assuming a welcome page here
           return false;
       } else {
           if (!mRecent.project()) return false;
           PExProjectNode *project = mRecent.project()->assignedProject();
           return project && project->runnableGms();
       }
    }
    return false;
}

bool MainWindow::isRecentGroupRunning()
{
    if (!mRecent.project()) return false;
    PExProjectNode *project = mRecent.project()->assignedProject();
    if (!project) return false;
    return (project->gamsProcessState() != QProcess::NotRunning);
}

void MainWindow::on_actionShow_System_Log_triggered()
{
    int index = ui->logTabs->indexOf(mSyslog);
    if (index < 0) {
        ui->logTabs->addTab(mSyslog, "System");
        ui->logTabs->setCurrentWidget(mSyslog);
    } else {
        ui->logTabs->setCurrentIndex(index);
    }
    mSyslog->raise();
    dockWidgetShow(ui->dockProcessLog, true);
}

void MainWindow::on_actionShow_Welcome_Page_triggered()
{
    showWelcomePage();
}

void MainWindow::triggerGamsLibFileCreation(modeldialog::LibraryItem *item)
{
    if (item)
        openModelFromLib(item->library()->glbFile(), item);
}

const HistoryData &MainWindow::history()
{
    return mHistory;
}

void MainWindow::resetHistory()
{
    mHistory = HistoryData();
    historyChanged();
}

void MainWindow::addToOpenedFiles(QString filePath)
{
    if (!QFileInfo::exists(filePath)) return;

    if (filePath.startsWith("[")) return; // invalid

    while (mHistory.files().size() > Settings::settings()->toInt(skHistorySize) && !mHistory.files().isEmpty())
        mHistory.files().removeLast();

    if (Settings::settings()->toInt(skHistorySize) == 0) {
        historyChanged();
        return;
    }

    if (!mHistory.files().contains(filePath)) {
        mHistory.files().insert(0, filePath);
    } else {
        mHistory.files().move(mHistory.files().indexOf(filePath), 0);
    }
    historyChanged();
}

void MainWindow::clearHistory(FileMeta *file)
{
    mHistory.files().removeAll(file->location());
}

void MainWindow::historyChanged()
{
    if (mWp) mWp->historyChanged();
    QVariantList joHistory;
    for (const QString &file : mHistory.files()) {
        if (file.isEmpty()) break;
        QVariantMap joOpenFile;
        joOpenFile["file"] = file;
        joHistory << joOpenFile;
    }
    Settings::settings()->setList(skHistory, joHistory);
}

bool MainWindow::terminateProcessesConditionally(QVector<PExProjectNode *> projects)
{
    if (projects.isEmpty()) return true;
    QVector<PExProjectNode *> runningGroups;
    QStringList runningNames;
    int remoteCount = 0;
    int ignoredCount = 0;
    for (PExProjectNode* project: projects) {
        if (project->process() && project->process()->state() != QProcess::NotRunning) {
            if (project->process()->terminateOption() == AbstractProcess::termRemote) {
                ++remoteCount;
                runningGroups.prepend(project);
                runningNames.prepend(project->name());
            } else {
                if (project->process()->terminateOption() == AbstractProcess::termIgnored)
                    ++ignoredCount;
                runningGroups << project;
                runningNames << project->name();
            }
        }
    }
    if (runningNames.isEmpty()) return true;
    int localCount = runningNames.size() - remoteCount;
    for (int i = 0; i < runningNames.size(); ++i) {
        if (runningGroups.at(i)->process()->terminateOption() == AbstractProcess::termRemote)
            runningNames.replace(i, runningNames.at(i) + " (remote)");
        else if (runningGroups.at(i)->process()->terminateOption() == AbstractProcess::termIgnored)
            runningNames.replace(i, runningNames.at(i) + " (won't stop)");
    }

    QString title = runningNames.size() > 1 ? QString::number(runningNames.size())+" processes are running"
                                            : runningNames.first()+" is running";
    if (!localCount) title += " remotely";
    else if (remoteCount) title += ", "+QString::number(remoteCount)+" remotely";

    bool ignoreOnly = localCount == ignoredCount;
    QString message = (ignoreOnly && !remoteCount ? QString("It's not possible to stop the ")
                                                  : QString("Do you want to stop the ") )
                    + (runningNames.size() > 1 ? "processes?\n" : "process?\n");

    while (runningNames.size() > 10) runningNames.removeLast();
    message += runningNames.join("\n");
    if (runningNames.size() < runningGroups.size()) runningNames << " ...";

    int choice = remoteCount ? localCount ? QMessageBox::question(this, title, message, "Stop All", "Stop Local", "Cancel")
                                          : QMessageBox::question(this, title, message, "Stop", "Keep", "Cancel")
                             : ignoreOnly ? QMessageBox::question(this, title, message, "Exit anyway", "Cancel") + 1
                                          : QMessageBox::question(this, title, message, "Stop", "Cancel") + 1;
    if (choice == 2) return false;
    for (PExProjectNode* project: qAsConst(runningGroups)) {
        if (project->process()->terminateOption() && choice == 1)
            project->process()->terminateLocal();
        else
            project->process()->terminate();
    }
    return true;
}

void MainWindow::updateAndSaveSettings()
{
    Settings *settings = Settings::settings();

    QScreen *screen = window()->screen();
    QSize scrDiff = screen->availableSize() - frameSize();
    if (!isMaximized() && !isFullScreen() && (scrDiff.width()>0 || scrDiff.height()>0) && screen->size() != size()) {
        settings->setSize(skWinSize, size());
        settings->setPoint(skWinPos, pos());
    }
    settings->setByteArray(skWinState, saveState());
    settings->setBool(skWinMaximized, isMaximized() || (mMaximizedBeforeFullScreen && isFullScreen()));
    settings->setBool(skWinFullScreen, isFullScreen());

    settings->setBool(skViewProject, projectViewVisibility());
    settings->setBool(skViewOutput, outputViewVisibility());
    settings->setBool(skViewHelp, helpViewVisibility());
    settings->setBool(skViewOption, optionEditorVisibility());

    settings->setString(skEncodingMib, encodingMIBsString());

    settings->setBool(skSearchUseRegex, searchDialog()->regex());
    settings->setBool(skSearchCaseSens, searchDialog()->caseSens());
    settings->setBool(skSearchWholeWords, searchDialog()->wholeWords());

#ifdef QWEBENGINE
    QVariantList joBookmarks;
    QMap<QString, QString> bookmarkMap(helpWidget()->getBookmarkMap());
    QMap<QString, QString>::const_iterator it = bookmarkMap.constBegin();
    while (it != bookmarkMap.constEnd()) {
        QVariantMap joBookmark;
        joBookmark.insert("location", it.key());
        joBookmark.insert("name", it.value());
        joBookmarks << joBookmark;
        ++it;
    }
    settings->setList(skHelpBookmarks, joBookmarks);
    settings->setDouble(skHelpZoomFactor, helpWidget()->getZoomFactor());
#endif

    QVariantList projects;
    projectRepo()->write(projects);
    settings->setList(skProjects, projects);

    QVariantMap tabData;
    writeTabs(tabData);
    settings->setMap(skTabs, tabData);
    settings->setInt(skPinViewTabIndex, pinViewTabIndex());

    historyChanged();

    settings->setList(SettingsKey::skUserThemes, Theme::instance()->writeUserThemes());

    // at last, save the settings
    settings->save();
}

void MainWindow::restoreFromSettings()
{
    Settings *settings = Settings::settings();

    // main window
    resize(settings->toSize(skWinSize));
    move(settings->toPoint(skWinPos));
    ensureInScreen();
    QTimer::singleShot(0, this, &MainWindow::ensureInScreen);

    mMaximizedBeforeFullScreen = settings->toBool(skWinMaximized);
    if (settings->toBool(skWinFullScreen)) {
        setWindowState(windowState() ^ Qt::WindowFullScreen);
    } else if (mMaximizedBeforeFullScreen) {
        setWindowState(windowState() ^ Qt::WindowMaximized);
    }
    ui->actionFull_Screen->setChecked(settings->toBool(skWinFullScreen));
    restoreState(settings->toByteArray(skWinState));
    ui->actionOpenAlternative->setText(COpenAltText.at(Settings::settings()->toBool(skOpenInCurrent) ? 0 : 1));
    ui->actionOpenAlternative->setToolTip(COpenAltText.at(Settings::settings()->toBool(skOpenInCurrent) ? 2 : 3));
    // tool-/menubar
    setProjectViewVisibility(settings->toBool(skViewProject));
    setOutputViewVisibility(settings->toBool(skViewOutput));
    setExtendedEditorVisibility(settings->toBool(skViewOption));
    setHelpViewVisibility(settings->toBool(skViewHelp));
    setEncodingMIBs(settings->toString(skEncodingMib));

    QStringList invalidSuffix;
    QStringList suffixes = FileType::validateSuffixList(settings->toString(skUserGamsTypes), &invalidSuffix);
    mFileMetaRepo.setUserGamsTypes(suffixes);
    if (!invalidSuffix.isEmpty()) {
        settings->setString(skUserGamsTypes, suffixes.join(","));
    }

    // help
#ifdef QWEBENGINE
    const QVariantList joHelp = settings->toList(skHelpBookmarks);
    QMap<QString, QString> bookmarkMap;
    for (const QVariant &joVal: joHelp) {
        if (!joVal.canConvert(QVariant::Map)) continue;
        QVariantMap entry = joVal.toMap();
        bookmarkMap.insert(entry.value("location").toString(), entry.value("name").toString());
    }
    helpWidget()->setBookmarkMap(bookmarkMap);
    double hZoom = settings->toDouble(skHelpZoomFactor);
    helpWidget()->setZoomFactor(hZoom > 0.0 ? hZoom : 1.0);
#endif

    Theme::instance()->readUserThemes(settings->toList(SettingsKey::skUserThemes));

}

void MainWindow::importProjectDialog()
{
    QString path = mRecent.project() ? mRecent.project()->location() : CommonPaths::defaultWorkingDir();
    QFileDialog *dialog = new QFileDialog(this, QString("Import Project"), path);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setNameFilters(ViewHelper::dialogProjectFilter());
    connect(dialog, &QFileDialog::fileSelected, this, [this](const QString &fileName) { openProject(fileName); });
    connect(dialog, &QFileDialog::finished, this, [dialog]() { dialog->deleteLater(); });
    dialog->setModal(true);
    dialog->open();
}

void MainWindow::openProject(const QString gspFile)
{
    if (mOpenPermission == opNoGsp) {
        if (!mDelayedFiles.contains(gspFile, FileType::fsCaseSense()))
            mDelayedFiles << gspFile;
        return;
    }
    QJsonDocument json;
    QFile file(gspFile);
    if (file.open(QFile::ReadOnly)) {
        QJsonParseError parseResult;
        json = QJsonDocument::fromJson(file.readAll(), &parseResult);
        if (parseResult.error) {
            appendSystemLogError("Couldn't parse project from " + gspFile);
            return;
        }
        file.close();

        QString name = QFileInfo(gspFile).fileName();
        QString path = QFileInfo(file).path();
        QVariantMap map = json.object().toVariantMap();
        QVariantList data = map.value("projects").toList();
        loadProject(data, name, path, false);
    } else {
        appendSystemLogError("Couldn't open project " + gspFile);
    }
}

void MainWindow::loadProject(const QVariantList data, const QString &name, const QString &basePath, bool ignoreMissingFiles)
{
    path::PathRequest *dialog = new path::PathRequest(this);
    dialog->init(&mProjectRepo, name, basePath, data);

    if (ignoreMissingFiles || dialog->checkProject()) {
        dialog->deleteLater();
        mProjectRepo.read(data, basePath);
    } else {
        connect(dialog, &path::PathRequest::finished, this, [this, dialog]() {
            dialog->deleteLater();
            mOpenPermission = opAll;
            QTimer::singleShot(0, this, &MainWindow::openDelayedFiles);
        });
        connect(dialog, &path::PathRequest::accepted, this, [this, data, dialog]() {
            mProjectRepo.read(data, dialog->baseDir());
        });
        dialog->open();
#ifdef __APPLE__
        dialog->show();
        dialog->raise();
#endif
        mOpenPermission = opNoGsp;
    }
}

void MainWindow::exportProjectDialog(PExProjectNode *project)
{
    QFileDialog *dialog = new QFileDialog(this, QString("Export Project %1").arg(project->name()),
                                          project->location()+'/'+project->name()+".gsp");
    dialog->setProperty("warned", false);
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setNameFilters(ViewHelper::dialogProjectFilter());
    dialog->setDefaultSuffix("gsp");
    connect(dialog,&QFileDialog::directoryEntered, this, [dialog, project](const QString &) {
        if (dialog->directory() != QDir(project->location())) {
            QToolTip::showText(QCursor::pos(), "<body><b>Warning!</b><br/>If the project is "
                                               "stored outside of it's base, file locations are lost.</body>");
        } else {
            QToolTip::hideText();
        }
    });
    connect(dialog, &QFileDialog::fileSelected, this, [this, project](const QString &fileName) {
        QFile file(fileName);
        if (file.open(QFile::WriteOnly)) {
            QVariantMap map;
            QVariantList data;
            mProjectRepo.write(project, data, true);
            map.insert("projects", data);
            file.write(QJsonDocument(QJsonObject::fromVariantMap(map)).toJson());
            file.close();
        } else {
            appendSystemLogError("Couldn't write project to " + fileName);
        }

    });
    connect(dialog, &QFileDialog::finished, this, [dialog]() { dialog->deleteLater(); });
    dialog->setModal(true);
    dialog->open();
}

void MainWindow::closePinView()
{
    QWidget *edit = mPinView->widget();
    if (edit) {
        mPinView->removeWidget();
        mRecent.removeEditor(edit);
        FileMeta *fm = mFileMetaRepo.fileMeta(edit);
        if (fm) fm->removeEditor(edit);
        edit->deleteLater();
        updateRecentEdit(edit, ui->mainTabs->currentWidget());
    }
    Settings::settings()->setInt(skPinViewTabIndex, -1);
}

QString MainWindow::currentPath()
{
    if (currentEdit() != nullptr)
        return mRecent.path();
    return Settings::settings()->toString(skDefaultWorkspace);

}

void MainWindow::on_actionGAMS_Library_triggered()
{
    QString path = Settings::settings()->toString(skUserModelLibraryDir).split(',', Qt::SkipEmptyParts).first();
    modeldialog::ModelDialog dialog(path, this);
    if(dialog.exec() == QDialog::Accepted) {
        QMessageBox msgBox;
        modeldialog::LibraryItem *item = dialog.selectedLibraryItem();

        triggerGamsLibFileCreation(item);
    }
}

void MainWindow::on_actionGDX_Diff_triggered()
{
    QString path = currentPath();
    actionGDX_Diff_triggered(path);
}

void MainWindow::actionGDX_Diff_triggered(QString workingDirectory, QString input1, QString input2)
{
    mGdxDiffDialog->setRecentPath(workingDirectory);
    if (!input1.isEmpty()) { // function call was triggered from the context menu
        // when both input1 and input2 are specified, the corresponding files in the dialog need to be set
        if (!input2.isEmpty()) {
            mGdxDiffDialog->setInput1(input1);
            mGdxDiffDialog->setInput2(input2);
        } else {
            if (mGdxDiffDialog->input1().trimmed().isEmpty()) // set input1 line edit if empty
                mGdxDiffDialog->setInput1(input1);
            else if (mGdxDiffDialog->input2().trimmed().isEmpty()) // set input2 if input1 is not empty but input1 is empty
                mGdxDiffDialog->setInput2(input1);
            else // set input1 otherwise
                mGdxDiffDialog->setInput1(input1);
        }
    }
    // for Mac OS
    if (mGdxDiffDialog->isVisible()) {
        mGdxDiffDialog->raise();
        mGdxDiffDialog->activateWindow();
    } else {
        mGdxDiffDialog->show();
    }
}

void MainWindow::on_actionBase_mode_triggered()
{
    if (!validMiroPrerequisites())
        return;

    auto miroProcess = std::make_unique<miro::MiroProcess>(new miro::MiroProcess);
    miroProcess->setSkipModelExecution(ui->actionSkip_model_execution->isChecked());
    miroProcess->setWorkingDirectory(mRecent.project()->workDir());
    miroProcess->setModelName(mRecent.project()->mainModelName());
    miroProcess->setMiroPath(miro::MiroCommon::path(Settings::settings()->toString(skMiroInstallPath)));
    miroProcess->setMiroMode(miro::MiroMode::Base);

    execute(mGamsParameterEditor->getCurrentCommandLineData(), std::move(miroProcess));
}

void MainWindow::on_actionConfiguration_mode_triggered()
{
    if (!validMiroPrerequisites())
        return;

    auto miroProcess = std::make_unique<miro::MiroProcess>(new miro::MiroProcess);
    miroProcess->setSkipModelExecution(ui->actionSkip_model_execution->isChecked());
    miroProcess->setWorkingDirectory(mRecent.project()->workDir());
    miroProcess->setModelName(mRecent.project()->mainModelName());
    miroProcess->setMiroPath(miro::MiroCommon::path(Settings::settings()->toString(skMiroInstallPath)));
    miroProcess->setMiroMode(miro::MiroMode::Configuration);

    execute(mGamsParameterEditor->getCurrentCommandLineData(), std::move(miroProcess));
}

void MainWindow::on_actionStop_MIRO_triggered()
{
    if (!mRecent.project())
        return;
    mRecent.project()->process()->terminate();
}

void MainWindow::on_actionDeploy_triggered()
{
    if (!validMiroPrerequisites())
        return;

    QString assemblyFile = mRecent.project()->workDir() + "/" +
                           miro::MiroCommon::assemblyFileName(mRecent.project()->mainModelName());

    QStringList checkedFiles;
    if (mRecent.project()) {
        checkedFiles = miro::MiroCommon::unifiedAssemblyFileContent(assemblyFile,
                                                                mRecent.project()->mainModelName(false));
    }

    mMiroDeployDialog->setDefaults();
    mMiroDeployDialog->setAssemblyFileName(assemblyFile);
    mMiroDeployDialog->setWorkingDirectory(mRecent.project()->workDir());
    mMiroDeployDialog->setModelName(mRecent.project()->mainModelName());
    mMiroDeployDialog->setSelectedFiles(checkedFiles);
    mMiroDeployDialog->exec();
}

void MainWindow::writeNewAssemblyFileData()
{
    if (!miro::MiroCommon::writeAssemblyFile(mMiroDeployDialog->assemblyFileName(),
                                             mMiroDeployDialog->selectedFiles()))
        SysLogLocator::systemLog()->append(QString("Could not write model assembly file: %1")
                                           .arg(mMiroDeployDialog->assemblyFileName()),
                                           LogMsgType::Error);
    else {
        mMiroDeployDialog->setAssemblyFileName(mMiroDeployDialog->assemblyFileName());
        mProjectRepo.findOrCreateFileNode(mMiroDeployDialog->assemblyFileName(), mRecent.project());
    }
}

void MainWindow::on_menuMIRO_aboutToShow()
{
    updateMiroEnabled();
}

void MainWindow::miroDeploy(bool testDeploy, miro::MiroDeployMode mode)
{
    if (!mRecent.project())
        return;

    auto process = std::make_unique<miro::MiroDeployProcess>(new miro::MiroDeployProcess);
    process->setMiroPath(miro::MiroCommon::path( Settings::settings()->toString(skMiroInstallPath)));
    process->setWorkingDirectory(mRecent.project()->workDir());
    process->setModelName(mRecent.project()->mainModelName());
    process->setTestDeployment(testDeploy);
    process->setTargetEnvironment(mMiroDeployDialog->targetEnvironment());

    if (testDeploy) {
        switch(mode){
        case miro::MiroDeployMode::Base:
            process->setBaseMode(true);
            break;
        default:
            break;
        }
    } else {
        process->setBaseMode(true);
    }

    execute(mGamsParameterEditor->getCurrentCommandLineData(), std::move(process));
}

void MainWindow::setMiroRunning(bool running)
{
    mMiroRunning = running;
    updateMiroEnabled();
}

void MainWindow::updateMiroEnabled(bool printError)
{
    bool available = isMiroAvailable(printError) && isActiveProjectRunnable();
    ui->menuMIRO->setEnabled(available);
    mMiroDeployDialog->setEnabled(available && !mMiroRunning);
    ui->actionBase_mode->setEnabled(available && !mMiroRunning);
    ui->actionHypercube_mode->setEnabled(available && !mMiroRunning);
    ui->actionConfiguration_mode->setEnabled(available && !mMiroRunning);
    ui->actionSkip_model_execution->setEnabled(available && !mMiroRunning);
    ui->actionCreate_model_assembly->setEnabled(available && !mMiroRunning);
    ui->actionDeploy->setEnabled(available && !mMiroRunning);
}

void MainWindow::on_projectView_activated(const QModelIndex &index)
{
    PExAbstractNode* node = mProjectRepo.node(index);
    if (!node) return;
    if ((node->type() == NodeType::group) || (node->type() == NodeType::project)) {
        ui->projectView->isExpanded(index) ? ui->projectView->collapse(index) : ui->projectView->expand(index);
    } else if (node->type() == NodeType::file) {
        PExFileNode *node = mProjectRepo.asFileNode(index);
        if (node) {
            bool pin = QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier;
            bool pinV = pin && (QGuiApplication::queryKeyboardModifiers() & Qt::ShiftModifier);
            openFileNode(node, !pin);
            if (pin && node->file()) {
                int idx = -1;
                for (int i = 0; i < node->file()->editors().size(); ++i) {
                    QWidget *wid = node->file()->editors().at(i);
                    idx = ui->mainTabs->indexOf(wid);
                    if (idx >= 0) break;
                }
                if (idx >= 0) {
                    openPinView(idx, pinV ? Qt::Vertical : Qt::Horizontal);
                    ui->projectView->selectionModel()->select(mProjectRepo.treeModel()->index(node), QItemSelectionModel::ClearAndSelect);
                    mPinView->widget()->setFocus();
                }
            }
        }
    }
}

bool MainWindow::requestCloseChanged(QVector<FileMeta *> changedFiles)
{
    if (changedFiles.size() <= 0) return true;

    int ret = QMessageBox::Discard;
    QMessageBox msgBox;
    QString filesText = changedFiles.size()==1
              ? QDir::toNativeSeparators(changedFiles.first()->location()) + " has been modified."
              : QString::number(changedFiles.size())+" files have been modified";
    ret = showSaveChangesMsgBox(filesText);
    if (ret == QMessageBox::Save) {
        mAutosaveHandler->clearAutosaveFiles(mOpenTabsList);
        for (FileMeta* fm : changedFiles) {
            if (fm->isModified()) {
                fm->save();
            }
        }
    } else if (ret == QMessageBox::Cancel) {
        return false;
    } else { // Discard
        mAutosaveHandler->clearAutosaveFiles(mOpenTabsList);
        for (FileMeta* fm : changedFiles) {
            if (fm->isModified()) {
                closeFileEditors(fm->id());
            }
        }
    }

    return true;
}

RecentData *MainWindow::recent()
{
    return &mRecent;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // leave distraction free mode before exiting so we do not lose widget states
    if (ui->actionDistraction_Free_Mode->isChecked())
        ui->actionDistraction_Free_Mode->setChecked(false);

    PExProjectNode *project = currentProject();
    if (project) project->addRunParametersHistory(mGamsParameterEditor->getCurrentCommandLineData());

    updateAndSaveSettings();
    QVector<FileMeta*> oFiles = mFileMetaRepo.modifiedFiles();
    if (!terminateProcessesConditionally(mProjectRepo.projects())) {
        event->setAccepted(false);
    } else if (requestCloseChanged(oFiles)) {
        on_actionClose_All_triggered();
        closeHelpView();
        mTextMarkRepo.clear();
        delete mSettingsDialog;
        mSettingsDialog = nullptr;
        mTabStyle = nullptr;
    } else {
        event->setAccepted(false);
    }
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
    if (e == Hotkey::Print)
        on_actionPrint_triggered();

    if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_0)) {
        updateFonts(Settings::settings()->toInt(skEdFontSize), Settings::settings()->toString(skEdFontFamily));
    }

    // escape is the close button for focussed widgets
    if (e->key() == Qt::Key_Escape) {

#ifdef QWEBENGINE
        // help widget
        if (mHelpWidget->isVisible()) {
            closeHelpView();
            e->accept(); return;
        }
#endif

        // log widgets
        if (focusWidget() == mSyslog) {
            if (mSyslog->textCursor().hasSelection()) {
                QTextCursor cursor = mSyslog->textCursor();
                cursor.clearSelection();
                mSyslog->setTextCursor(cursor);
            } else {
                setOutputViewVisibility(false);
            }
            e->accept();
            return;
        } else if (focusWidget() == ui->logTabs->currentWidget()
                   || (focusWidget() && focusWidget()->parentWidget() == ui->logTabs->currentWidget())) {
            e->setAccepted(false);
            if (TextView *tv = ViewHelper::toTextView(ui->logTabs->currentWidget())) {
                if (tv->hasSelection()) {
                    tv->clearSelection();
                    e->accept();
                }
            }
            if (!e->isAccepted()) {
                on_logTabs_tabCloseRequested(ui->logTabs->currentIndex());
                ui->logTabs->currentWidget()->setFocus();
                e->accept();
            }
            return;
        } else if (focusWidget() == ui->projectView) {
            setProjectViewVisibility(false);
        } else if (mGamsParameterEditor->isAParameterEditorFocused(focusWidget())) {
            mGamsParameterEditor->deSelectParameters();
        } else if (mRecent.editor() != nullptr) {
            if (option::SolverOptionWidget *so = ViewHelper::toSolverOptionEdit(mRecent.editor())) {
                so->deSelectOptions();
            } else if (option::GamsConfigEditor *guc = ViewHelper::toGamsConfigEditor(mRecent.editor())) {
                guc->deSelectAll();
            }
        }

        // search widget
        if (mSearchDialog->isHidden()) mSearchDialog->on_btn_clear_clicked();
        else mSearchDialog->hide();

        e->accept(); return;
    } // end escape block

    // focus shortcuts
    if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_1)) {
        focusProjectExplorer();
        e->accept(); return;
    } else if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_2)) {
        focusCentralWidget();
        e->accept(); return;
    } else if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_3)) {
        focusCmdLine();
        e->accept(); return;
    } else if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_4)) {
        showTabsMenu();
        e->accept(); return;
    } else if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_5)) {
        focusProcessLogs();
        e->accept(); return;
    } else if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_F12)) {
        toggleDebugMode();
        e->accept(); return;
    }
    QMainWindow::keyPressEvent(e);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasUrls() && (FileMeta::hasExistingFile(e->mimeData()->urls())
                                    || FileMeta::hasExistingFolder(e->mimeData()->urls()))) {
        e->setDropAction(Qt::CopyAction);
        e->accept();
    } else {
        e->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent* e)
{
    if (!e->mimeData()->hasUrls()) return;
    QStringList pathList = FileMeta::pathList(e->mimeData()->urls());
    if (pathList.isEmpty()) return;
    e->accept();

    int answer;
    if (pathList.size() > 25) {
        raise();
        activateWindow();
        QMessageBox msgBox;
        msgBox.setText("You are trying to open " + QString::number(pathList.size()) +
                       " files or folders at once. This may take a long time.");
        msgBox.setInformativeText("Do you want to continue?");
        msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Cancel);
        answer = msgBox.exec();

        if (answer != QMessageBox::Open) return;
    }
    openFiles(pathList, false);
}

bool MainWindow::eventFilter(QObject* sender, QEvent* event)
{
    if (sender == this) {
        if (event->type() == QEvent::ScreenChangeInternal)
            adjustFonts();
        return false;
    }
    QDockWidget* dw = static_cast<QDockWidget*>(sender);
    if (dw->isFloating()) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            keyPressEvent(keyEvent);
            return true;
        }
    } else if (event->type() == QEvent::Resize) {
        mWinStateTimer.start();
    }

    return false;
}

PExFileNode* MainWindow::openFileWithOption(QString fileName, PExProjectNode* knownProject, OpenGroupOption opt, bool focus)
{
    PExProjectNode *curProject = mRecent.project();
    PExProjectNode *project = knownProject;
    PExFileNode *fileNode = nullptr;

    if (opt == ogNone)
        opt = Settings::settings()->toBool(skOpenInCurrent) ? ogCurrentGroup : ogFindGroup;

    FileMeta *fileMeta = (opt == ogNewGroup) ? nullptr : mFileMetaRepo.fileMeta(fileName);
    if (opt == ogFindGroup) {
        if (fileMeta) {
            // found, prefer created or current group (over a third group)
            if (project)
                fileNode = project->findFile(fileMeta);
            else if (curProject)
                fileNode = curProject->findFile(fileMeta);
            if (!fileNode)
                fileNode = mProjectRepo.findFile(fileMeta);
        }
    } else if (opt == ogCurrentGroup) {
        if (!project)
            project = curProject;
        if (project)
            fileNode = project->findFile(fileMeta);
    }
    // create the destination group if necessary
    if (!project && !fileNode) {
        QFileInfo fi(fileName);
        project = mProjectRepo.createProject(fi.completeBaseName(), fi.absolutePath(), "");
    }

    // create node if missing
    if (!fileNode) {
        if (project) {
            if (fileMeta) {
                fileNode = mProjectRepo.findOrCreateFileNode(fileMeta, project);
            } else {
                fileNode = mProjectRepo.findOrCreateFileNode(fileName, project);
            }
        } else {
            DEB() << "OOPS, this shouldn't happen: Neither project nor fileNode defined!";
        }
    }

    // open the detected file
    if (fileNode) {
        openFileNode(fileNode, focus);
    } else {
        DEB() << "OOPS, this shouldn't happen: unable to create the fileNode!";
    }

    return fileNode;
}

void MainWindow::openFiles(OpenGroupOption opt)
{
    QString path = currentPath();
    const QStringList files = QFileDialog::getOpenFileNames(this, "Open file(s)", path,
                                                            ViewHelper::dialogFileFilterAll().join(";;"),
                                                            nullptr, DONT_RESOLVE_SYMLINKS_ON_MACOS);
    if (files.isEmpty()) return;

    PExFileNode *firstNode = nullptr;
    PExFileNode *fileNode = nullptr;
    for (const QString &fileName : files) {
        // detect if the file is already present at the scope
        fileNode = openFileWithOption(fileName, nullptr, opt);
        if (!firstNode) firstNode = fileNode;
    }

    // at last: activate the first node
    if (firstNode)
        openFileNode(firstNode, true);
}

PExProjectNode *MainWindow::currentProject()
{
    PExFileNode* node = mProjectRepo.findFileNode(mRecent.editor());
    PExProjectNode *project = (node ? node->assignedProject()
                                    : mProjectRepo.findProject(ViewHelper::groupId(mRecent.editor())));
    if (!project) project = mRecent.project();
    return project;
}

int MainWindow::pinViewTabIndex()
{
    if (!mPinView->isVisible()) return -1;
    QWidget *wid = mPinView->widget();
    FileMeta *fm = mFileMetaRepo.fileMeta(wid);
    if (!fm || fm->editors().size() < 2) return -1;
    wid = fm->editors().at(0) == wid ? fm->editors().at(1) : fm->editors().at(0);
    return ui->mainTabs->indexOf(wid);
}

void MainWindow::openFiles(QStringList files, bool forceNew, OpenGroupOption opt)
{
    if (files.size() == 0) return;
    if (mOpenPermission == opNone) {
        // During initialization only append for later processing
        mDelayedFiles.append(files);
        return;
    }

    if (!forceNew && files.size() == 1) {
        if (files.first().endsWith(".gsp", Qt::CaseInsensitive)) {
            openProject(files.first());
            return;
        }
        FileMeta *file = mFileMetaRepo.fileMeta(files.first());
        if (file) {
            openFile(file);
            return;
        }
    }

    QStringList filesNotFound;
    QList<PExFileNode*> gmsFiles;

    // create project
    if (opt == ogNone)
        opt = Settings::settings()->toBool(skOpenInCurrent) ? ogCurrentGroup : ogFindGroup;
    PExProjectNode *project = (opt == ogCurrentGroup) ? mRecent.project() : nullptr;
    for (const QString &item: files) {
        QDir d(item);
        QFileInfo f(item);

        if (f.isFile()) {
            if (item.endsWith(".gsp", Qt::CaseInsensitive)) {
                openProject(item);
            } else {
                PExFileNode *node = addNode("", item, project);
                openFileNode(node);
                if (node->file()->kind() == FileKind::Gms) gmsFiles << node;
            }
            QApplication::processEvents(QEventLoop::AllEvents, 1);
        } else if (d.exists()) {
            openFolder(item, project);
        } else {
            filesNotFound.append(item);
        }
    }
    // find runnable gms, for now take first one found
    if (gmsFiles.size() > 0) {
        if (project) project->setParameter("gms", gmsFiles.first()->location());
    }

    if (!filesNotFound.empty()) {
        QString msgText("The following files could not be opened:");
        for(const QString &s : qAsConst(filesNotFound))
            msgText.append("\n" + s);
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(msgText);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons()) {
        QWidget* child = childAt(event->pos());
        Q_UNUSED(child)
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QMouseEvent* me = static_cast<QMouseEvent*>(event);
    if (me->button() == Qt::ForwardButton) {
        on_actionGoForward_triggered();
        event->accept();
    } else if (me->button() == Qt::BackButton) {
        on_actionGoBack_triggered();
        event->accept();
    }
}

void MainWindow::customEvent(QEvent *event)
{
    QMainWindow::customEvent(event);
    if (event->type() == option::LineEditCompleteEvent::type())
        (static_cast<option::LineEditCompleteEvent*>(event))->complete();
}

void MainWindow::dockWidgetShow(QDockWidget *dw, bool show)
{
    if (show) {
        dw->setVisible(show);
        dw->raise();
    } else {
        dw->hide();
    }
}

void MainWindow::execute(QString commandLineStr, std::unique_ptr<AbstractProcess> process)
{
    PExProjectNode* project = currentProject();
    if (!project) {
        DEB() << "Nothing to be executed.";
        return;
    }
    bool ready = executePrepare(project, commandLineStr, std::move(process));
    if (ready) execution(project);
}


bool MainWindow::executePrepare(PExProjectNode* project, QString commandLineStr, std::unique_ptr<AbstractProcess> process)
{
    Settings *settings = Settings::settings();
    project->addRunParametersHistory( mGamsParameterEditor->getCurrentCommandLineData() );
    project->clearErrorTexts();
    if (QWidget *wid = currentEdit())
        wid->setFocus();

    // gather modified files and autosave or request to save
    QVector<FileMeta*> modifiedFiles;
    if (settings->toBool(skAutosaveOnRun)) {
        modifiedFiles = mFileMetaRepo.modifiedFiles();
    } else {
        for (PExFileNode *node: project->listFiles()) {
            if (node->file()->isOpen() && !modifiedFiles.contains(node->file()) && node->file()->isModified())
                modifiedFiles << node->file();
        }
    }

    bool doSave = !modifiedFiles.isEmpty();
    if (doSave && !settings->toBool(skAutosaveOnRun)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        if (modifiedFiles.size() > 1)
            msgBox.setText(modifiedFiles.first()->location()+" and "+QString::number(modifiedFiles.size()-1)+" other files have been modified.");
        else
            msgBox.setText(QDir::toNativeSeparators(modifiedFiles.first()->location())+" has been modified.");
        msgBox.setInformativeText("Do you want to save your changes before running?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        QAbstractButton* discardButton = msgBox.addButton(tr("Discard Changes and Run"), QMessageBox::ResetRole);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Cancel) {
            return false;
        } else if (msgBox.clickedButton() == discardButton) {
            for (FileMeta *file: qAsConst(modifiedFiles))
                if (file->kind() != FileKind::Log && file->kind() != FileKind::PrO) {
                    try {
                        file->load(file->codecMib());
                    } catch (Exception&) {
                        // TODO(JM) add reaction on exception
                    }
                }
            doSave = false;
        }
    }
    if (doSave) {
        for (FileMeta *file: qAsConst(modifiedFiles)) file->save();
    }

    // clear the TextMarks for this group
    QSet<TextMark::Type> markTypes;
    markTypes << TextMark::error << TextMark::link << TextMark::target;
    for (PExFileNode *node: project->listFiles())
        mTextMarkRepo.removeMarks(node->file()->id(), node->assignedProject()->id(), markTypes);

    // prepare the log
    PExLogNode* logNode = mProjectRepo.logNode(project);
    markTypes << TextMark::bookmark;
    mTextMarkRepo.removeMarks(logNode->file()->id(), logNode->assignedProject()->id(), markTypes);

    logNode->resetLst();
    if (!logNode->file()->isOpen()) {
        QWidget *wid = logNode->file()->createEdit(ui->logTabs, logNode->assignedProject(), logNode->file()->codecMib());
        logNode->file()->addToTab(ui->logTabs, wid, logNode->file()->codecMib());
        wid->setFont(getEditorFont(fgLog));
        if (TextView* tv = ViewHelper::toTextView(wid))
            tv->setLineWrapMode(settings->toBool(skEdLineWrapProcess) ? AbstractEdit::WidgetWidth : AbstractEdit::NoWrap);
    }
    if (TextView* tv = ViewHelper::toTextView(logNode->file()->editors().first())) {
        MainWindow::connect(tv, &TextView::selectionChanged, this, &MainWindow::updateStatusPos, Qt::UniqueConnection);
        MainWindow::connect(tv, &TextView::blockCountChanged, this, &MainWindow::updateStatusLineCount, Qt::UniqueConnection);
        MainWindow::connect(tv, &TextView::loadAmountChanged, this, &MainWindow::updateStatusLoadAmount, Qt::UniqueConnection);
    }
    // cleanup bookmarks
    const QVector<QString> cleanupKinds {"gdx",  "gsp", "log", "lst", "ls2", "lxi", "ref"};
    markTypes = QSet<TextMark::Type>() << TextMark::bookmark;
    for (const QString &kind: cleanupKinds) {
        if (project->hasParameter(kind)) {
            FileMeta *file = mFileMetaRepo.fileMeta(project->parameter(kind));
            if (file) mTextMarkRepo.removeMarks(file->id(), markTypes);
        }
    }

    if (settings->toBool(skEdClearLog)) logNode->clearLog();

    disconnect(ui->logTabs, &QTabWidget::currentChanged, this, &MainWindow::tabBarClicked);
    if (!ui->logTabs->children().contains(logNode->file()->editors().first())) {
        ui->logTabs->addTab(logNode->file()->editors().first(), logNode->name(NameModifier::editState));
    }
    ui->logTabs->setCurrentWidget(logNode->file()->editors().first());
    ui->dockProcessLog->setVisible(true);
    connect(ui->logTabs, &QTabWidget::currentChanged, this, &MainWindow::tabBarClicked);

    // select gms-file and working dir to run
    QString gmsFilePath = project->parameter("gms");
    if (gmsFilePath == "") {
        appendSystemLogWarning("No runnable GMS file found in group ["+project->name()+"].");
        ui->actionShow_System_Log->trigger();
        return false;
    }
    FileMeta *runMeta = mFileMetaRepo.fileMeta(gmsFilePath);
    PExFileNode *runNode = project->findFile(runMeta);
    logNode->file()->setCodecMib(runNode ? runNode->file()->codecMib() : -1);
    QString workDir = project->workDir();

    // prepare the options and process and run it
    QList<option::OptionItem> itemList = mGamsParameterEditor->getOptionTokenizer()->tokenize(commandLineStr);
    option::Option *opt = mGamsParameterEditor->getOptionTokenizer()->getOption();
    if (process)
        project->setProcess(std::move(process));
    AbstractProcess* groupProc = project->process();
    int logOption = 0;
    groupProc->setParameters(project->analyzeParameters(gmsFilePath, groupProc->defaultParameters(), itemList, opt, logOption));
    logNode->prepareRun(logOption);
    logNode->setJumpToLogEnd(true);

    groupProc->setGroupId(project->id());
    groupProc->setWorkingDirectory(workDir);

    // disable MIRO menus
    if (dynamic_cast<miro::AbstractMiroProcess*>(groupProc) ) {
        setMiroRunning(true);
        connect(groupProc, &AbstractProcess::finished, this, [this](){setMiroRunning(false);});
    }
    connect(groupProc, &AbstractProcess::newProcessCall, this, &MainWindow::newProcessCall);
    connect(groupProc, &AbstractProcess::finished, this, &MainWindow::postGamsRun, Qt::UniqueConnection);

    logNode->linkToProcess(groupProc);
    return true;
}

void MainWindow::execution(PExProjectNode *project)
{
    AbstractProcess* groupProc = project->process();
    groupProc->execute();
    ui->toolBar->repaint();

    ui->dockProcessLog->raise();
}

void MainWindow::updateRunState()
{
    updateMiroEnabled(false);
    mGamsParameterEditor->updateRunState(isActiveProjectRunnable(), isRecentGroupRunning());
    ui->actionExport_Project->setEnabled(mRecent.project() && mRecent.project()->childCount());
}

#ifdef QWEBENGINE
help::HelpWidget *MainWindow::helpWidget() const
{
    return mHelpWidget;
}
#endif

void MainWindow::setMainGms(PExFileNode *node)
{
    PExProjectNode *project = node->assignedProject();
    if (project) {
        project->setRunnableGms(node->file());
        updateRunState();
    }
}

void MainWindow::parameterRunChanged()
{
    if (isActiveProjectRunnable() && !isRecentGroupRunning())
        mGamsParameterEditor->runDefaultAction();
}

void MainWindow::initDelayedElements()
{
    adjustFonts();
    Settings *settings = Settings::settings();
    projectRepo()->read(settings->toList(skProjects));

    if (settings->toBool(skRestoreTabs)) {
        QVariantMap joTabs = settings->toMap(skTabs);
        if (!readTabs(joTabs)) return;
        if (int ind = settings->toInt(skPinViewTabIndex)) {
            openPinView(ind, Qt::Orientation(settings->toInt(skPinOrientation)));
        }
    }
    mHistory.files().clear();
    const QVariantList joHistory = settings->toList(skHistory);
    for (const QVariant &jRef: joHistory) {
        if (!jRef.canConvert(QVariant::Map)) continue;
        QVariantMap map = jRef.toMap();
        if (map.contains("file"))
            mHistory.files() << map.value("file").toString();
    }
    openDelayedFiles();
    watchProjectTree();
    PExFileNode *node = mProjectRepo.findFileNode(ui->mainTabs->currentWidget());
    if (node) openFileNode(node, true);
    historyChanged();
}

void MainWindow::openDelayedFiles()
{
    QStringList files = mDelayedFiles;
    mDelayedFiles.clear();
    mOpenPermission = opAll;
    openFiles(files, false);
}

void MainWindow::newProjectDialog()
{
    QString path = mRecent.project() ? mRecent.project()->location() : CommonPaths::defaultWorkingDir();
    QFileDialog *dialog = new QFileDialog(this, QString("New Project"), path);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::DirectoryOnly);

    connect(dialog, &QFileDialog::fileSelected, this, [this](const QString &projectPath) { createProject(projectPath); });
    connect(dialog, &QFileDialog::finished, this, [dialog]() { dialog->deleteLater(); });
    dialog->setModal(true);
    dialog->open();
}

void MainWindow::updateRecentEdit(QWidget *old, QWidget *now)
{
    Q_UNUSED(old)
    QWidget *wid = now;
    while (wid && wid->parentWidget()) {
        if (wid->parentWidget() == ui->splitter) {
            PinKind pinKind = wid == ui->mainTabs ? pkNone : PinKind(mPinView->orientation());
            mRecent.setEditor(wid == ui->mainTabs ? ui->mainTabs->currentWidget() : mPinView->widget());
            if (FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editor())) {
                fm->editToTop(mRecent.editor());
                ui->actionPin_Right->setEnabled(fm->isPinnable());
                ui->actionPin_Below->setEnabled(fm->isPinnable());
            }
            mSearchDialog->editorChanged(mRecent.editor());
            mNavigationHistory->setCurrentEdit(mRecent.editor(), pinKind);
            if (mStartedUp)
                mProjectRepo.editorActivated(mRecent.editor(), false);
            updateRunState();
            break;
        }
        wid = wid->parentWidget();
    }
}

void MainWindow::openProjectOptions(PExProjectNode *project)
{
    if (!project) return;
    FileMeta *fm = project->projectOptionsFileMeta();
    if (!fm) {
        fm = mFileMetaRepo.findOrCreateFileMeta(project->location()+"/["+project->name()+"]", &FileType::from(FileKind::PrO));
        project->setProjectOptionsFileMeta(fm);
    }
    openFile(fm, true, project);
}

void MainWindow::createProject(QString projectPath)
{
    // create empty project
    QFileInfo fi(projectPath);
    PExProjectNode *project = mProjectRepo.createProject(fi.completeBaseName(), projectPath, QString());
    openProjectOptions(project);
}

void MainWindow::on_actionRun_triggered()
{
    execute(mGamsParameterEditor->on_runAction(option::RunActionState::Run), std::make_unique<GamsProcess>());
}

void MainWindow::on_actionRun_with_GDX_Creation_triggered()
{
    execute(mGamsParameterEditor->on_runAction(option::RunActionState::RunWithGDXCreation), std::make_unique<GamsProcess>());
}

void MainWindow::on_actionCompile_triggered()
{
    execute(mGamsParameterEditor->on_runAction(option::RunActionState::Compile), std::make_unique<GamsProcess>());
}

void MainWindow::on_actionCompile_with_GDX_Creation_triggered()
{
    execute(mGamsParameterEditor->on_runAction(option::RunActionState::CompileWithGDXCreation), std::make_unique<GamsProcess>());
}

void MainWindow::on_actionRunNeos_triggered()
{
    showNeosStartDialog();
}

void MainWindow::on_actionRunEngine_triggered()
{
    showEngineStartDialog();
}

QString MainWindow::readGucValue(QString key)
{
    QString res;
    const QStringList paths = CommonPaths::gamsStandardPaths(CommonPaths::StandardConfigPath);
    for (int i = paths.size() ; i > 0 ; --i) {
        const QString &path = paths.at(i-1);
        option::GamsUserConfig *guc = new option::GamsUserConfig(path+"/gamsconfig.yaml");
        if (guc && guc->isAvailable()) {
            const QList<option::EnvVarConfigItem *> vars = guc->readEnvironmentVariables();
            for (option::EnvVarConfigItem *var : vars) {
                if (var->key == key) {
                    res = var->value;
                    break;
                }
            }
        }
        delete guc;
        if (!res.isEmpty()) break;
    }
    return res;
}

void MainWindow::initCompleterActions()
{
    mFileMetaRepo.completer()->addActions(ui->menuFile->actions());
    mFileMetaRepo.completer()->addActions(ui->menuEdit->actions());
    mFileMetaRepo.completer()->addActions(ui->menu_GAMs->actions());
    mFileMetaRepo.completer()->addActions(ui->menuMIRO->actions());
    mFileMetaRepo.completer()->addActions(ui->menuTools->actions());
    mFileMetaRepo.completer()->addActions(ui->menuView->actions());
    mFileMetaRepo.completer()->addActions(ui->menuHelp->actions());
    mFileMetaRepo.completer()->addActions(ui->menuAdvanced->actions());
    mFileMetaRepo.completer()->addActions(ui->menuHelp->actions());
}

void MainWindow::showNeosStartDialog()
{
    neos::NeosProcess *neosPtr = createNeosProcess();
    if (!neosPtr) return;
    if (mNeosMail.isEmpty())
        mNeosMail = readGucValue("NEOS_EMAIL");
    if (mNeosMail.isEmpty()) {
        auto env = QProcessEnvironment::systemEnvironment();
        mNeosMail = env.value("NEOS_EMAIL");
    }
    neos::NeosStartDialog *dialog = new neos::NeosStartDialog(mNeosMail, this);
    dialog->setProcess(neosPtr);
    connect(dialog, &neos::NeosStartDialog::rejected, dialog, &neos::NeosStartDialog::deleteLater);
    connect(dialog, &neos::NeosStartDialog::accepted, dialog, &neos::NeosStartDialog::deleteLater);
    connect(dialog, &neos::NeosStartDialog::accepted, this, &MainWindow::prepareNeosProcess);
    connect(dialog, &neos::NeosStartDialog::eMailChanged, this, [this](const QString &eMail) {
        mNeosMail = eMail;
    });
    connect(dialog, &neos::NeosStartDialog::noDialogFlagChanged, this, [this](bool noDialog) {
        mNeosNoDialog = noDialog;
    });

    if (mNeosNoDialog && Settings::settings()->toBool(SettingsKey::skNeosAcceptTerms)
            && Settings::settings()->toBool(SettingsKey::skNeosAutoConfirm)
            && !qApp->keyboardModifiers().testFlag(Qt::ControlModifier)) {
        dialog->accept();
    } else {
        dialog->open();
    }
}

neos::NeosProcess *MainWindow::createNeosProcess()
{
    PExProjectNode *project = currentProject();
    if (!project) return nullptr;
    auto neosProcess = std::make_unique<neos::NeosProcess>(new neos::NeosProcess());
    neosProcess->setWorkingDirectory(mRecent.project()->workDir());
    mGamsParameterEditor->on_runAction(option::RunActionState::RunNeos);
    project->setProcess(std::move(neosProcess));
    neos::NeosProcess *neosPtr = static_cast<neos::NeosProcess*>(project->process());
    connect(neosPtr, &neos::NeosProcess::procStateChanged, this, &MainWindow::remoteProgress);
    return neosPtr;
}

void MainWindow::prepareNeosProcess()
{
    PExProjectNode *project = currentProject();
    if (!project) return;
    updateAndSaveSettings();
    neos::NeosProcess *neosPtr = static_cast<neos::NeosProcess*>(project->process());
    neosPtr->setStarting();
    if (!executePrepare(project, mGamsParameterEditor->getCurrentCommandLineData()))
        return;
    if (!mIgnoreSslErrors) {
        connect(neosPtr, &neos::NeosProcess::sslValidation, this, &MainWindow::sslValidation);
        neosPtr->validate();
    } else {
        updateAndSaveSettings();
        execution(project);
    }
}

void MainWindow::sslValidation(QString errorMessage)
{
    if (mIgnoreSslErrors || errorMessage.isEmpty()) {
        updateAndSaveSettings();
        PExProjectNode *project = currentProject();
        if (!project) return;
        execution(project);
    } else {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setWindowTitle("SSL Error");
        msgBox->setText("The following SSL error occurred");
        msgBox->setInformativeText(errorMessage);
        msgBox->setStandardButtons(QMessageBox::Ignore | QMessageBox::Abort);
        msgBox->setDefaultButton(QMessageBox::Abort);
        connect(msgBox, &QMessageBox::buttonClicked, this, &MainWindow::sslUserDecision);
        msgBox->setModal(true);
        msgBox->open();
    }
}

void MainWindow::sslUserDecision(QAbstractButton *button)
{
    PExProjectNode *project = currentProject();
    if (!project) return;
    QMessageBox *msgBox = qobject_cast<QMessageBox*>(sender());
    if (msgBox && msgBox->standardButton(button) == QMessageBox::Ignore) {
        if (neos::NeosProcess *neosProc = static_cast<neos::NeosProcess*>(project->process())) {
            neosProc->setIgnoreSslErrors();
            mIgnoreSslErrors = true;
            updateAndSaveSettings();
            execution(project);
        }
    } else {
        project->setProcess(nullptr);
    }
}

void MainWindow::showEngineStartDialog()
{
    // prepare process
    engine::EngineProcess *proc = createEngineProcess();
    if (!proc) return;
    proc->setAuthToken(mEngineAuthToken);

    // prepare dialog
    engine::EngineStartDialog *dialog = new engine::EngineStartDialog(this);
    dialog->initData(Settings::settings()->toString(SettingsKey::skEngineUrl),
                     Settings::settings()->toString(SettingsKey::skEngineUser),
                     Settings::settings()->toInt(SettingsKey::skEngineAuthExpire),
                     Settings::settings()->toBool(SettingsKey::skEngineIsSelfCert),
                     Settings::settings()->toString(SettingsKey::skEngineNamespace),
                     Settings::settings()->toString(SettingsKey::skEngineUserInstance),
                     Settings::settings()->toBool(SettingsKey::skEngineForceGdx));

    connect(proc, &engine::EngineProcess::authorized, this, [this, dialog](const QString &token) {
        mEngineAuthToken = token;
        Settings::settings()->setString(SettingsKey::skEngineUrl, dialog->url());
        Settings::settings()->setString(SettingsKey::skEngineUser, dialog->user());
        Settings::settings()->setBool(SettingsKey::skEngineIsSelfCert, dialog->isCertAccepted());
        if (Settings::settings()->toBool(SettingsKey::skEngineStoreUserToken))
            Settings::settings()->setString(SettingsKey::skEngineUserToken, mEngineAuthToken);
        else
            Settings::settings()->setString(SettingsKey::skEngineUserToken, QString());
        updateAndSaveSettings();
    });
    connect(proc, &engine::EngineProcess::releaseGdxFile, this, [this](const QString &gdxFilePath) {
        FileMeta * fm = mFileMetaRepo.fileMeta(gdxFilePath);
        if (fm) {
            for (QWidget *wid : fm->editors()) {
                if (gdxviewer::GdxViewer *gdx = ViewHelper::toGdxViewer(wid)) {
                    gdx->invalidate();
                }
            }
        }
    });
    connect(proc, &engine::EngineProcess::reloadGdxFile, this, [this](const QString &gdxFilePath) {
        FileMeta * fm = mFileMetaRepo.fileMeta(gdxFilePath);
        if (fm) {
            for (QWidget *wid : fm->editors()) {
                if (gdxviewer::GdxViewer *gdx = ViewHelper::toGdxViewer(wid)) {
                    gdx->reload(fm->codec());
                }
            }
        }
    });

    dialog->setModal(true);
    dialog->setProcess(proc);
    dialog->setHiddenMode(mEngineNoDialog && !qApp->keyboardModifiers().testFlag(Qt::ControlModifier));
    if (Settings::settings()->toBool(SettingsKey::skEngineIsSelfCert))
        dialog->setAcceptCert();
    connect(dialog, &engine::EngineStartDialog::submit, this, &MainWindow::engineSubmit);
    dialog->start();
}

void MainWindow::engineSubmit(bool start)
{
    engine::EngineStartDialog *dialog = qobject_cast<engine::EngineStartDialog*>(sender());
    if (!dialog) return;
    if (start) {
        Settings::settings()->setString(SettingsKey::skEngineNamespace, dialog->nSpace());
        Settings::settings()->setString(SettingsKey::skEngineUserInstance, dialog->userInstance());
        Settings::settings()->setBool(SettingsKey::skEngineForceGdx, dialog->forceGdx());
        mEngineNoDialog = dialog->isAlways();
        prepareEngineProcess();
    } else {
        dialog->close();
    }
    dialog->deleteLater();
}

engine::EngineProcess *MainWindow::createEngineProcess()
{
    updateAndSaveSettings();
    PExProjectNode *project = currentProject();
    if (!project) {
        DEB() << "Could not create GAMS Engine process";
        return nullptr;
    }
    auto engineProcess = std::make_unique<engine::EngineProcess>(new engine::EngineProcess());
    connect(engineProcess.get(), &engine::EngineProcess::procStateChanged, this, &MainWindow::remoteProgress);
    engineProcess->setWorkingDirectory(mRecent.project()->workDir());
    QString commandLineStr = mGamsParameterEditor->getCurrentCommandLineData();
    const QList<option::OptionItem> itemList = mGamsParameterEditor->getOptionTokenizer()->tokenize(commandLineStr);
    for (const option::OptionItem &item : itemList) {
        if (item.key.compare("previousWork", Qt::CaseInsensitive) == 0)
            engineProcess->setHasPreviousWorkOption(true);
    }
    project->setProcess(std::move(engineProcess));
    return qobject_cast<engine::EngineProcess*>(project->process());
}

void MainWindow::prepareEngineProcess()
{
    PExProjectNode *project = currentProject();
    if (!project) return;
    AbstractProcess* process = project->process();
    engine::EngineProcess *engineProcess = qobject_cast<engine::EngineProcess*>(process);
    if (!engineProcess) return;
    mGamsParameterEditor->on_runAction(option::RunActionState::RunEngine);

    updateAndSaveSettings();
    executePrepare(project, mGamsParameterEditor->getCurrentCommandLineData());
    execution(project);
}

void MainWindow::on_actionInterrupt_triggered()
{
    PExProjectNode *project = currentProject();
    if (!project) return;
    mGamsParameterEditor->on_interruptAction();
    AbstractProcess* process = project->process();
    QtConcurrent::run(process, &AbstractProcess::interrupt);
}

void MainWindow::on_actionStop_triggered()
{
    PExProjectNode *project = currentProject();
    if (!project) return;
    mGamsParameterEditor->on_stopAction();
    AbstractProcess* process = project->process();
    QtConcurrent::run(process, &GamsProcess::terminate);
}

void MainWindow::changeToLog(PExAbstractNode *node, bool openOutput, bool createMissing)
{
    Settings *settings = Settings::settings();
    bool moveToEnd = false;
    PExLogNode* logNode = mProjectRepo.logNode(node);
    if (!logNode) return;

    if (createMissing) {
        moveToEnd = true;
        if (!logNode->file()->isOpen()) {
            QWidget *wid = logNode->file()->createEdit(ui->logTabs, logNode->assignedProject(), logNode->file()->codecMib());
            logNode->file()->addToTab(ui->logTabs, wid, logNode->file()->codecMib());
            wid->setFont(getEditorFont(fgLog));
            if (TextView * tv = ViewHelper::toTextView(wid))
                tv->setLineWrapMode(settings->toBool(skEdLineWrapProcess) ? AbstractEdit::WidgetWidth
                                                                          : AbstractEdit::NoWrap);
        }
        if (TextView* tv = ViewHelper::toTextView(logNode->file()->editors().first())) {
            MainWindow::connect(tv, &TextView::selectionChanged, this, &MainWindow::updateStatusPos, Qt::UniqueConnection);
            MainWindow::connect(tv, &TextView::blockCountChanged, this, &MainWindow::updateStatusLineCount, Qt::UniqueConnection);
            MainWindow::connect(tv, &TextView::loadAmountChanged, this, &MainWindow::updateStatusLoadAmount, Qt::UniqueConnection);
        }
    }
    if (logNode->file()->isOpen()) {
        if (TextView* logEdit = ViewHelper::toTextView(logNode->file()->editors().first())) {
            if (openOutput) setOutputViewVisibility(true);
            if (ui->logTabs->currentWidget() != logEdit) {
                if (ui->logTabs->currentWidget() != resultsView())
                    ui->logTabs->setCurrentWidget(logEdit);
            }
            if (moveToEnd) {
                logEdit->jumpTo(logEdit->lineCount()-1, 0);
            }
        }
    }
}

void MainWindow::storeTree()
{
    updateAndSaveSettings();
}

void MainWindow::cloneBookmarkMenu(QMenu *menu)
{
    menu->addAction(ui->actionToggleBookmark);
}

void MainWindow::editableFileSizeCheck(const QFile &file, bool &canOpen)
{
    qint64 maxSize = Settings::settings()->toInt(skEdEditableMaxSizeMB) *1024*1024;
    int factor = (sizeof (void*) == 2) ? 16 : 23;
    if (mFileMetaRepo.askBigFileEdit() && file.exists() && file.size() > maxSize) {
        QString text = ("File size of " + QString::number(qreal(maxSize)/1024/1024, 'f', 1)
                        + " MB exceeded by " + file.fileName() + "\n"
                        + "About " + QString::number(qreal(file.size())/1024/1024*factor, 'f', 1)
                        + " MB of memory need to be allocated.\n"
                        + "Opening this file can take a long time during which Studio will be unresponsive.");
        int choice = QMessageBox::question(nullptr, "File size of " + QString::number(qreal(maxSize)/1024/1024, 'f', 1)
                                           + " MB exceeded", text, "Open anyway", "Always open", "Cancel", 2, 2);
        if (choice == 2) {
            canOpen = false;
            return;
        }
        if (choice == 1) mFileMetaRepo.setAskBigFileEdit(false);
    }
    canOpen = true;
}

void MainWindow::newProcessCall(const QString &text, const QString &call)
{
    SysLogLocator::systemLog()->append(text + " " + call, LogMsgType::Info);
}

void MainWindow::invalidateTheme()
{
    for (FileMeta *fm: mFileMetaRepo.fileMetas())
        fm->invalidateTheme();
    if (mTabStyle) {
        TabBarStyle *old = mTabStyle;
        mTabStyle = new TabBarStyle(ui->mainTabs, ui->logTabs, QApplication::style()->objectName());
        delete old;
    }
    repaint();
}

void MainWindow::ensureInScreen()
{
    QRect appGeo = geometry();
    QRect appFGeo = frameGeometry();
    QMargins margins(appGeo.left() - appFGeo.left(), appGeo.top() - appFGeo.top(),
                     appFGeo.right() - appGeo.right(), appFGeo.bottom() - appGeo.bottom());
    QRect screenGeo = QGuiApplication::primaryScreen()->availableVirtualGeometry();
    QVector<QRect> frames;
    for (QScreen *screen : QGuiApplication::screens()) {
        QRect rect = screen->availableGeometry();
        QRect sect = rect.intersected(appGeo);
        if (100*sect.height()*sect.width() / (appGeo.height()*appGeo.width()) > 3)
            frames << rect;
    }
    if (frames.size() == 1)
        screenGeo = frames.at(0);
    screenGeo -= margins;

    if (appGeo.width() > screenGeo.width()) appGeo.setWidth(screenGeo.width());
    if (appGeo.height() > screenGeo.height()) appGeo.setHeight(screenGeo.height());
    if (appGeo.x() < screenGeo.x()) appGeo.moveLeft(screenGeo.x());
    if (appGeo.y() < screenGeo.y()) appGeo.moveTop(screenGeo.y());
    if (appGeo.right() > screenGeo.right()) appGeo.moveLeft(screenGeo.right()-appGeo.width());
    if (appGeo.bottom() > screenGeo.bottom()) appGeo.moveTop(screenGeo.bottom()-appGeo.height());
    if (appGeo != geometry()) setGeometry(appGeo);

}

void MainWindow::raiseEdit(QWidget *widget)
{
    while (widget && widget != this) {
        widget->raise();
        widget = widget->parentWidget();
    }
}

void MainWindow::openFile(FileMeta* fileMeta, bool focus, PExProjectNode *project, int codecMib,
                          bool forcedAsTextEditor, NewTabStrategy tabStrategy)
{
    if (!fileMeta) return;
    QWidget* edit = nullptr;
    QTabWidget* tabWidget = fileMeta->kind() == FileKind::Log ? ui->logTabs : ui->mainTabs;
    if (!fileMeta->editors().empty()) {
        edit = fileMeta->editors().constFirst();
    }

    // open edit if existing or create one
    if (edit) {
        if (project)
            ViewHelper::setGroupId(edit, project->id());
    } else {
        if (!project) {
            QVector<PExFileNode*> nodes = mProjectRepo.fileNodes(fileMeta->id());
            if (nodes.size()) {
                if (nodes.first()->assignedProject())
                    project = nodes.first()->assignedProject();
            } else {
                QFileInfo file(fileMeta->location());
                project = mProjectRepo.createProject(file.completeBaseName(), file.absolutePath(),
                                                     file.absoluteFilePath())->toProject();
                nodes.append(mProjectRepo.findOrCreateFileNode(file.absoluteFilePath(), project));
            }
        }
        try {
            if (codecMib == -1) codecMib = fileMeta->codecMib();
            edit = fileMeta->createEdit(tabWidget, project, codecMib, forcedAsTextEditor);
            fileMeta->addToTab(tabWidget, edit, codecMib, tabStrategy);
            QTimer::singleShot(0, this, [this, edit, fileMeta]() {
                edit->setFont(getEditorFont(fileMeta->fontGroup()));
            });

        } catch (Exception &e) {
            appendSystemLogError(e.what());
            return;
        }
        if (!edit) {
            DEB() << "Error: could not create editor for '" << fileMeta->location() << "'";
            return;
        }
        initEdit(fileMeta, edit);
    }
    // set keyboard focus to editor
    if (focus && edit) {
        if (edit == mPinView->widget()) {
            edit->setFocus();
        } else {
            tabWidget->setCurrentWidget(edit);
            tabWidget->currentWidget()->setFocus();
            if (tabWidget == ui->mainTabs && tabWidget->indexOf(edit) >= 0) {
                if (!ViewHelper::toGamsConfigEditor((edit)))
                    activeTabChanged(tabWidget->indexOf(edit));
            }
        }
        raiseEdit(edit);
        updateMenuToCodec(fileMeta->codecMib());
    }

    if (tabWidget != ui->logTabs) {
        // if there is already a log -> show it
        PExFileNode* fileNode = mProjectRepo.findFileNode(edit);
        changeToLog(fileNode, false, false);
        updateRecentEdit(mRecent.editor(), edit);
    }
    addToOpenedFiles(fileMeta->location());
}

void MainWindow::initEdit(FileMeta* fileMeta, QWidget *edit)
{
    if (ViewHelper::toCodeEdit(edit)) {
        CodeEdit* ce = ViewHelper::toCodeEdit(edit);
        connect(ce, &CodeEdit::requestAdvancedActions, this, &MainWindow::getAdvancedActions);
        connect(ce, &CodeEdit::cloneBookmarkMenu, this, &MainWindow::cloneBookmarkMenu);
        connect(ce, &CodeEdit::searchFindNextPressed, mSearchDialog, &search::SearchDialog::on_searchNext);
        connect(ce, &CodeEdit::searchFindPrevPressed, mSearchDialog, &search::SearchDialog::on_searchPrev);
        ce->addAction(ui->actionRun);
    }
    if (TextView *tv = ViewHelper::toTextView(edit)) {
        connect(tv, &TextView::searchFindNextPressed, mSearchDialog, &search::SearchDialog::on_searchNext);
        connect(tv, &TextView::searchFindPrevPressed, mSearchDialog, &search::SearchDialog::on_searchPrev);

    }
    if (ViewHelper::toCodeEdit(edit)) {
        AbstractEdit *ae = ViewHelper::toAbstractEdit(edit);
        if (!ae->isReadOnly())
            connect(fileMeta, &FileMeta::changed, this, &MainWindow::fileChanged, Qt::UniqueConnection);
    } else if (fileMeta->kind() == FileKind::PrO || fileMeta->kind() == FileKind::Opt
               || fileMeta->kind() == FileKind::Guc|| fileMeta->kind() == FileKind::Efi) {
        connect(fileMeta, &FileMeta::changed, this, &MainWindow::fileChanged, Qt::UniqueConnection);
    } else if (fileMeta->kind() == FileKind::Ref) {
        reference::ReferenceViewer *refView = ViewHelper::toReferenceViewer(edit);
        connect(refView, &reference::ReferenceViewer::jumpTo, this, &MainWindow::on_referenceJumpTo);
    }
}

void MainWindow::openFileNode(PExFileNode *node, bool focus, int codecMib, bool forcedAsTextEditor, NewTabStrategy tabStrategy)
{
    if (!node) return;
    openFile(node->file(), focus, node->assignedProject(), codecMib, forcedAsTextEditor, tabStrategy);
}

void MainWindow::reOpenFileNode(PExFileNode *node, bool focus, int codecMib, bool forcedAsTextEditor)
{
    FileMeta* fc = node->file();
    if (!fc) return;

    int ret = QMessageBox::Discard;
    if (fc->editors().size() == 1 && fc->isModified()) {
        // only ask, if this is the last editor of this file
        ret = showSaveChangesMsgBox(node->file()->name()+" has been modified.");
    }

    if (ret == QMessageBox::Save) {
        mAutosaveHandler->clearAutosaveFiles(mOpenTabsList);
        fc->save();
        closeFileEditors(fc->id());
    } else if (ret == QMessageBox::Discard) {
        mAutosaveHandler->clearAutosaveFiles(mOpenTabsList);
        closeFileEditors(fc->id());
    } else if (ret == QMessageBox::Cancel) {
        return;
    }

    openFileNode(node, focus, codecMib, forcedAsTextEditor);
}

void MainWindow::closeProject(PExProjectNode* project)
{
    if (!project) return;
    if (!terminateProcessesConditionally(QVector<PExProjectNode*>() << project))
        return;
    QVector<FileMeta*> changedFiles;
    QVector<FileMeta*> openFiles;
    for (PExFileNode *node: project->listFiles()) {
        if (node->isModified()) changedFiles << node->file();
        if (node->file()->isOpen()) openFiles << node->file();
    }

    if (requestCloseChanged(changedFiles)) {
        for (FileMeta *file: qAsConst(openFiles)) {
            closeFileEditors(file->id());
        }
        for (PExFileNode *node: project->listFiles()) {
            mProjectRepo.closeNode(node);
        }
        PExLogNode* log = (project && project->hasLogNode()) ? project->logNode() : nullptr;
        if (log) {
            QWidget* edit = log->file()->editors().isEmpty() ? nullptr : log->file()->editors().first();
            if (edit) {
                log->file()->removeEditor(edit);
                int index = ui->logTabs->indexOf(edit);
                if (index >= 0) ui->logTabs->removeTab(index);
            }
        }
        if (FileMeta *prOp = project->projectOptionsFileMeta()) {
            closeFileEditors(prOp->id());
        }
        mProjectRepo.closeGroup(project);
    }
}

void MainWindow::neosProgress(AbstractProcess *proc, ProcState progress)
{
    PExProjectNode *project = mProjectRepo.asProject(proc->groupId());
    if (!project || !project->runnableGms()) return;
    QString gmsFilePath = project->runnableGms()->location();
    PExFileNode *gdxNode = project->findFile(gmsFilePath.left(gmsFilePath.lastIndexOf('.'))+"/out.gdx");
    if (gdxNode && gdxNode->file()->isOpen()) {
        if (gdxviewer::GdxViewer *gv = ViewHelper::toGdxViewer(gdxNode->file()->editors().first())) {
            if (progress == ProcState::Proc5GetResult) {
                gv->releaseFile();
            } else if (progress == ProcState::ProcIdle) {
                gv->setHasChanged(true);
                gv->reload(gdxNode->file()->codec());
            }
        }
    }
}

void MainWindow::remoteProgress(AbstractProcess *proc, ProcState progress)
{
    PExProjectNode *project = mProjectRepo.asProject(proc->groupId());
    if (!project || !project->runnableGms()) return;
    const QList<PExFileNode*> gdxNodes = project->findFiles(FileKind::Gdx);
    for (PExFileNode *gdxNode : gdxNodes) {
        if (gdxNode->file()->isOpen()) {
            if (gdxviewer::GdxViewer *gv = ViewHelper::toGdxViewer(gdxNode->file()->editors().first())) {
                if (progress == ProcState::Proc5GetResult) {
                    gv->releaseFile();
                } else if (progress == ProcState::ProcIdle) {
                    gv->setHasChanged(true);
                    gv->reload(gdxNode->file()->codec());
                }
            }
        }
    }
}

/// Asks user for confirmation if a file is modified before calling closeFile
/// \param file
///
void MainWindow::closeNodeConditionally(PExFileNode* node)
{
    // count nodes to the same file
    int nodeCountToFile = mProjectRepo.fileNodes(node->file()->id()).count();
    PExGroupNode *group = node->parentNode();
    PExProjectNode *project = node->assignedProject();
    if (project && !terminateProcessesConditionally(QVector<PExProjectNode*>() << project))
        return;
    // not the last OR not modified OR permitted
    if (nodeCountToFile > 1 || !node->isModified() || requestCloseChanged(QVector<FileMeta*>() << node->file())) {
        if (nodeCountToFile == 1)
            closeFileEditors(node->file()->id());
        mProjectRepo.closeNode(node);
    }
    mProjectRepo.purgeGroup(group);
}

/// Closes all open editors and tabs related to a file and remove option history
/// \param fileId
///
void MainWindow::closeFileEditors(const FileId fileId)
{
    FileMeta* fm = mFileMetaRepo.fileMeta(fileId);
    if (!fm) return;

    if (ViewHelper::fileId(mPinView->widget()) == fileId) {
        closePinView();
    }

    // add to recently closed tabs
    if (fm->kind() != FileKind::PrO)
        mClosedTabs << fm->location();
    int lastIndex = mWp->isVisible() ? 1 : 0;

    NavigationHistoryLocator::navigationHistory()->stopRecord();
    // close all related editors, tabs and clean up
    while (!fm->editors().isEmpty()) {
        QWidget *edit = fm->editors().constFirst();
        if (mRecent.editor() == edit) {
            if (mRecent.project()) {
               PExProjectNode *project = mRecent.project()->assignedProject();
               project->addRunParametersHistory( mGamsParameterEditor->getCurrentCommandLineData() );
            }
        }
        lastIndex = ui->mainTabs->indexOf(edit);
        ui->mainTabs->removeTab(lastIndex);
        if (edit == mRecent.editor())
            mSearchDialog->editorChanged(nullptr);
        mRecent.removeEditor(edit);
        fm->removeEditor(edit);
        edit->deleteLater();
    }
    if (fm->kind() != FileKind::PrO)
        mClosedTabsIndexes << lastIndex;
    // if the file has been removed, remove nodes
    if (!fm->exists(true)) fileDeletedExtern(fm->id());
    if (fm->kind() == FileKind::PrO) {
        fm->deleteLater();
    }
    NavigationHistoryLocator::navigationHistory()->startRecord();
}

void MainWindow::openFilePath(const QString &filePath, bool focus, int codecMib, bool forcedAsTextEditor, NewTabStrategy tabStrategy)
{
    if (!QFileInfo::exists(filePath)) {
        EXCEPT() << "File not found: " << filePath;
    }
    PExFileNode *fileNode = mProjectRepo.findFile(filePath);
    if (!fileNode) {
        fileNode = mProjectRepo.findOrCreateFileNode(filePath);
        if (!fileNode)
            EXCEPT() << "Could not create node for file: " << filePath;
    }

    openFileNode(fileNode, focus, codecMib, forcedAsTextEditor, tabStrategy);
}

PExFileNode* MainWindow::addNode(const QString &path, const QString &fileName, PExProjectNode* project)
{
    PExFileNode *node = nullptr;
    if (!fileName.isEmpty()) {
        QFileInfo fInfo(path, fileName);
        FileType fType = FileType::from(fInfo.fileName());

        if (fType == FileKind::Gsp) {
            // Placeholder to read the project and create all nodes for associated files
        } else {
            node = mProjectRepo.findOrCreateFileNode(fInfo.absoluteFilePath(), project);
        }
    }
    return node;
}

void MainWindow::on_referenceJumpTo(reference::ReferenceItem item)
{
    QFileInfo fi(item.location);
    if (fi.isFile()) {
        PExFileNode* fn = mProjectRepo.findFileNode(mRecent.editor());
        if (fn) {
            PExProjectNode* project =  fn->assignedProject();
            mProjectRepo.findOrCreateFileNode(fi.absoluteFilePath(), project);
        }
        openFilePath(fi.absoluteFilePath(), true);
        CodeEdit *codeEdit = ViewHelper::toCodeEdit(mRecent.editor());
        if (codeEdit) {
            int line = (item.lineNumber > 0 ? item.lineNumber-1 : 0);
            int column = (item.columnNumber > 0 ? item.columnNumber-1 : 0);
            codeEdit->jumpTo(line, column);
        }
    }
}

void MainWindow::on_actionSettings_triggered()
{
    if (!mSettingsDialog) {
        mSettingsDialog = new SettingsDialog(this);
        mSettingsDialog->setModal(true);
        connect(mSettingsDialog, &SettingsDialog::themeChanged, this, &MainWindow::invalidateTheme);
        connect(mSettingsDialog, &SettingsDialog::userGamsTypeChanged, this,[this]() {
            QStringList suffixes = FileType::validateSuffixList(Settings::settings()->toString(skUserGamsTypes));
            mFileMetaRepo.setUserGamsTypes(suffixes);
        });
        connect(mSettingsDialog, &SettingsDialog::editorFontChanged, this, &MainWindow::updateFonts);
        connect(mSettingsDialog, &SettingsDialog::editorLineWrappingChanged, this, &MainWindow::updateEditorLineWrapping);
        connect(mSettingsDialog, &SettingsDialog::editorTabSizeChanged, this, &MainWindow::updateTabSize);
        connect(mSettingsDialog, &SettingsDialog::reactivateEngineDialog, this, [this]() {
            mEngineNoDialog = false;
        });
        connect(mSettingsDialog, &SettingsDialog::persistToken, this, [this]() {
            Settings::settings()->setString(skEngineUserToken, mEngineAuthToken);
        });
        connect(mSettingsDialog, &SettingsDialog::finished, this, [this]() {
            updateAndSaveSettings();
            if (mSettingsDialog->hasDelayedBaseThemeChange()) {
                mSettingsDialog->delayBaseThemeChange(false);
                ViewHelper::updateBaseTheme();
            }
            ui->actionOpenAlternative->setText(COpenAltText.at(Settings::settings()->toBool(skOpenInCurrent) ? 0 : 1));
            ui->actionOpenAlternative->setToolTip(COpenAltText.at(Settings::settings()->toBool(skOpenInCurrent) ? 2 : 3));
            mFileMetaRepo.completer()->setCasing(CodeCompleterCasing(Settings::settings()->toInt(skEdCompleterCasing)));
            if (mSettingsDialog->miroSettingsEnabled())
                updateMiroEnabled();
        });
    }
    mSettingsDialog->setMiroSettingsEnabled(!mMiroRunning);
    mSettingsDialog->open();

}

void MainWindow::on_actionSearch_triggered()
{
    toggleSearchDialog();
}

void MainWindow::toggleSearchDialog()
{
    // help view
    if (ui->dockHelpView->isAncestorOf(QApplication::focusWidget()) ||
        ui->dockHelpView->isAncestorOf(QApplication::activeWindow())) {
#ifdef QWEBENGINE
        mHelpWidget->on_searchHelp();
#endif
    // parameter editor
    } else if (mGamsParameterEditor->isAParameterEditorFocused(QApplication::focusWidget()) ||
               mGamsParameterEditor->isAParameterEditorFocused(QApplication::activeWindow())) {
                mGamsParameterEditor->selectSearchField();
    } else {
        //  other alternative editors
        PExFileNode *fn = mProjectRepo.findFileNode(mRecent.editor());
        if (fn /*  && !fn->file()->document() */ ) { // TODO(JM) We could offer search in text mode
            if (fn->file()->kind() == FileKind::Gdx) {
                gdxviewer::GdxViewer *gdx = ViewHelper::toGdxViewer(mRecent.editor());
                gdx->selectSearchField();
                return;
            }
            if (reference::ReferenceViewer* refViewer = ViewHelper::toReferenceViewer(mRecent.editor())) {
                refViewer->selectSearchField();
                return;
            }
            if (option::SolverOptionWidget *sow = ViewHelper::toSolverOptionEdit(mRecent.editor())) {
                sow->selectSearchField();
                return;
            }
            if (efi::EfiEditor *efi = ViewHelper::toEfiEditor(mRecent.editor())) {
                efi->selectFilter();
                return;
            }
        }

        // e.g. needed for KDE to raise the search dialog when minimized
        if (mSearchDialog->isMinimized()) {
            mSearchDialog->setWindowState(Qt::WindowMaximized);
        }
        // toggle visibility
        if (mSearchDialog->isVisible()) {
            // e.g. needed for macOS to rasise search dialog when minimized
            mSearchDialog->raise();
            mSearchDialog->activateWindow();
            mSearchDialog->autofillSearchDialog();
        } else {
            if (mSearchWidgetPos.isNull()) { // first time opening
                int margin = 25;

                int wDiff = frameGeometry().width() - geometry().width();
                int hDiff = frameGeometry().height() - geometry().height();

                int wSize = mSearchDialog->width() + wDiff;
                int hSize = mSearchDialog->height() + hDiff;

                QPoint p(qMin(pos().x() + (width() - margin),
                            QGuiApplication::primaryScreen()->virtualGeometry().width()) - wSize,
                        qMin(pos().y() + hDiff + margin,
                            QGuiApplication::primaryScreen()->virtualGeometry().height() - hSize)
                       );
                mSearchWidgetPos = p;
            }

            mSearchDialog->show();
            mSearchDialog->move(mSearchWidgetPos);
       }
    }
}

void MainWindow::updateResults(search::SearchResultModel* results)
{
    int index = ui->logTabs->indexOf(resultsView()); // did widget exist before?

    delete mResultsView;
    mResultsView = new search::ResultsView(results, this);
    connect(mResultsView, &search::ResultsView::updateMatchLabel, searchDialog(), &search::SearchDialog::updateMatchLabel, Qt::UniqueConnection);
    connect(mSearchDialog, &search::SearchDialog::selectResult, mResultsView, &search::ResultsView::selectItem);

    QString nr;
    if (results->size() > MAX_SEARCH_RESULTS-1) nr = QString::number(MAX_SEARCH_RESULTS) + "+";
    else nr = QString::number(results->size());

    QString pattern = results->searchRegex().pattern().replace("\n", "");
    QString title("Results: " + pattern + " (" + nr + ")");

    ui->dockProcessLog->show();
    ui->dockProcessLog->activateWindow();
    ui->dockProcessLog->raise();

    if (index != -1) ui->logTabs->removeTab(index); // remove old result page

    ui->logTabs->addTab(mResultsView, title); // add new result page
    ui->logTabs->setCurrentWidget(mResultsView);

    mResultsView->resizeColumnsToContent();
}

void MainWindow::closeResultsView()
{
    int index = ui->logTabs->indexOf(mResultsView);
    if (index == -1) return;

    ui->logTabs->removeTab(index);
    searchDialog()->search()->resetResults();

    delete mResultsView;
    mResultsView = nullptr;
}

void MainWindow::openPinView(int tabIndex, Qt::Orientation orientation)
{
    if (tabIndex < 0 || tabIndex >= ui->mainTabs->tabBar()->count()) return;
    if (!mFileMetaRepo.fileMeta(ui->mainTabs->widget(tabIndex))) return;

    QWidget *wid = ui->mainTabs->widget(tabIndex);
    if (!wid) return;
    PExGroupNode *group = mProjectRepo.asGroup(ViewHelper::groupId(wid));
    if (!group) return;
    PExProjectNode *pro = group->assignedProject();
    if (!pro) return;
    FileMeta *fm = mFileMetaRepo.fileMeta(wid);
    if (!fm || !fm->isPinnable()) return;
    closePinView();

    QWidget *newWid = fm->createEdit(mPinView, pro);
    newWid->setFont(getEditorFont(fm->fontGroup()));
    mPinView->setWidget(newWid);
    mPinView->setFontGroup(fm->fontGroup());
    mPinView->setFileName(fm->name(NameModifier::editState), QDir::toNativeSeparators(fm->location()));
    mPinView->showAndAdjust(orientation);
    Settings::settings()->setInt(skPinViewTabIndex, tabIndex);
    initEdit(fm, newWid);
}

void MainWindow::invalidateResultsView()
{
    if (resultsView()) resultsView()->setOutdated();
}

void MainWindow::setGroupFontSize(FontGroup fontGroup, qreal fontSize, QString fontFamily)
{
//    if (mGroupFontSize.value(fontGroup, 0) != fontSize) {
    if (fontGroup == fgTable && mInitialTableFontSize < 0) {
        mInitialTableFontSize = ui->centralWidget->font().pointSizeF();
        if (fontSize < 0) fontSize = mInitialTableFontSize;
    }
    QFont f = getEditorFont(fontGroup, fontFamily, fontSize);
    if (fontGroup == fgLog) {
        for (QWidget* log: openLogs()) {
            log->setFont(f);
        }
        mSyslog->setFont(f);
    } else {
        for (QWidget* edit: openEditors()) {
            if (fontGroup == fgText) {
                if (AbstractEdit *ae = ViewHelper::toAbstractEdit(edit)) {
                    ae->setFont(f);
                } else if (lxiviewer::LxiViewer *lxi = ViewHelper::toLxiViewer(edit)) {
                    lxi->textView()->setFont(f);
                } else if (TextView *tv = ViewHelper::toTextView(edit)) {
                    tv->edit()->setFont(f);
                }
            } else if (fontGroup == fgTable) {
                if (AbstractView *av = ViewHelper::toAbstractView(edit))
                    av->setFont(f);
            }
        }
        if (fontGroup == fgTable) {
            mGamsParameterEditor->dockChild()->setFont(f);
        }
    }
//    }
}

void MainWindow::scrollSynchronize(QWidget *sendingEdit, int dx, int dy)
{
    if ((!dx && !dy) || !sendingEdit || !mPinView->isVisible() || !mPinView->isScrollLocked())
        return;

    // Only if edit has focus
    QWidget *wid = QApplication::focusWidget();
    bool focussed = mRecent.editor() == sendingEdit;
    while (wid && !focussed) {
        if (wid == sendingEdit)
            focussed = true;
        else if (!wid->parentWidget()) break;
        else wid = wid->parentWidget();
    }
    if (!focussed) return;

    QWidget *edit = otherEdit();
    if (!edit) return;

    // sync scroll
    if (lxiviewer::LxiViewer *lxi = ViewHelper::toLxiViewer(edit))
        lxi->textView()->scrollSynchronize(dx, dy);
    else if (AbstractEdit *ae = ViewHelper::toAbstractEdit(edit))
        ae->scrollSynchronize(dx, dy);
    else if (TextView *tv = ViewHelper::toTextView(edit))
        tv->scrollSynchronize(dx, dy);
}

void MainWindow::extraSelectionsUpdated()
{
    QWidget *edit = otherEdit();
    FileMeta *fm = mFileMetaRepo.fileMeta(edit);
    if (!fm || fm->fontGroup() != fgText) return;
    if (AbstractEdit *ae = ViewHelper::toAbstractEdit(edit))
        ae->updateExtraSelections();
    else if (TextView *tv = ViewHelper::toTextView(edit))
        tv->updateExtraSelections();
}

void MainWindow::updateFonts(qreal fontSize, QString fontFamily)
{
    setGroupFontSize(fgText, fontSize, fontFamily);
    setGroupFontSize(fgLog, fontSize, fontFamily);
    setGroupFontSize(fgTable, fontSize + mTableFontSizeDif);
    mWp->zoomReset();
}

void MainWindow::updateEditorLineWrapping()
{
    Settings *settings = Settings::settings();
    QPlainTextEdit::LineWrapMode wrapModeEditor = settings->toBool(skEdLineWrapEditor) ? QPlainTextEdit::WidgetWidth
                                                                                       : QPlainTextEdit::NoWrap;
    QPlainTextEdit::LineWrapMode wrapModeProcess = settings->toBool(skEdLineWrapProcess) ? QPlainTextEdit::WidgetWidth
                                                                                         : QPlainTextEdit::NoWrap;
    QWidgetList editList = mFileMetaRepo.editors();
    for (int i = 0; i < editList.size(); i++) {
        if (AbstractEdit* ed = ViewHelper::toAbstractEdit(editList.at(i))) {
            emit ed->blockCountChanged(0); // force redraw for line number area
            ed->setLineWrapMode(ViewHelper::editorType(ed) == EditorType::syslog ? wrapModeProcess
                                                                                 : wrapModeEditor);
        }
        if (TextView* tv = ViewHelper::toTextView(editList.at(i))) {
            emit tv->blockCountChanged(); // force redraw for line number area
            tv->setLineWrapMode(ViewHelper::editorType(tv) == EditorType::log ? wrapModeProcess
                                                                              : wrapModeEditor);
        }
    }
}

void MainWindow::updateTabSize(int size)
{
    for (QWidget* edit: openEditors()) {
        if (AbstractEdit *ed = ViewHelper::toAbstractEdit(edit))
            ed->updateTabSize(size);
        else if (TextView *tv = ViewHelper::toTextView(edit))
            tv->edit()->updateTabSize(size);
    }
    for (QWidget* log: openLogs()) {
        if (TextView *tv = ViewHelper::toTextView(log))
            tv->edit()->updateTabSize(size);
    }

    mSyslog->updateTabSize();

}

bool MainWindow::readTabs(const QVariantMap &tabData)
{
    QString curTab;
    if (tabData.contains("mainTabRecent")) {
        QString location = tabData.value("mainTabRecent").toString();
        if (QFileInfo::exists(location)) {
            openFilePath(location, true);
            mOpenTabsList << location;
            curTab = location;
        } else if (location == "WELCOME_PAGE") {
            showWelcomePage();
        }
    }
    QApplication::processEvents(QEventLoop::AllEvents, 10);
    if (tabData.contains("mainTabs") && tabData.value("mainTabs").canConvert(QVariant::List)) {
        NewTabStrategy tabStrategy = curTab.isEmpty() ? tabAtEnd : tabBeforeCurrent;
        QVariantList tabArray = tabData.value("mainTabs").toList();
        for (int i = 0; i < tabArray.size(); ++i) {
            QVariantMap tabObject = tabArray.at(i).toMap();
            if (tabObject.contains("location")) {
                QString location = tabObject.value("location").toString();
                if (curTab == location) {
                    tabStrategy = tabAtEnd;
                    continue;
                }
                if (QFileInfo::exists(location)) {
                    openFilePath(location, false, -1, false, tabStrategy);
                    mOpenTabsList << location;
                }
                if (i % 10 == 0) QApplication::processEvents(QEventLoop::AllEvents, 1);
                if (ui->mainTabs->count() <= i)
                    return false;
            }
        }
    }
    QTimer::singleShot(0, this, SLOT(initAutoSave()));
    return true;
}

void MainWindow::writeTabs(QVariantMap &tabData) const
{
    QVariantList tabArray;
    for (int i = 0; i < ui->mainTabs->count(); ++i) {
        QWidget *wid = ui->mainTabs->widget(i);
        if (!wid || wid == mWp) continue;
        FileMeta *fm = mFileMetaRepo.fileMeta(wid);
        if (!fm || fm->kind() == FileKind::PrO) continue;
        QVariantMap tabObject;
        tabObject.insert("location", fm->location());
        tabArray << tabObject;
    }
    tabData.insert("mainTabs", tabArray);

    FileMeta *fm = mRecent.editor() ? mFileMetaRepo.fileMeta(mRecent.editor()) : nullptr;
    if (fm) {
        if (fm->kind() == FileKind::PrO)
            fm = mFileMetaRepo.fileMeta(mRecent.persistentEditor());
        if (fm) tabData.insert("mainTabRecent", fm->location());
    } else if (ui->mainTabs->currentWidget() == mWp)
        tabData.insert("mainTabRecent", "WELCOME_PAGE");
}

void MainWindow::goToLine(int result)
{
    CodeEdit *codeEdit = ViewHelper::toCodeEdit(mRecent.editor());
    TextView *tv = ViewHelper::toTextView(mRecent.editor());
    if (result) {
        if (codeEdit)
            codeEdit->jumpTo(mGotoDialog->lineNumber());
        if (tv)
            tv->jumpTo(mGotoDialog->lineNumber(), 0);
    }
}

search::ResultsView *MainWindow::resultsView() const
{
    return mResultsView;
}

void MainWindow::on_actionGo_To_triggered()
{
    if (!currentEdit()) return;
    CodeEdit *codeEdit = ViewHelper::toCodeEdit(mRecent.editor());
    TextView *tv = ViewHelper::toTextView(mRecent.editor());
    if (!tv && !codeEdit) return;
    int numberLines = codeEdit ? codeEdit->blockCount() : tv ? tv->knownLines() : 1000000;
    mGotoDialog->maxLineCount(numberLines);
    mGotoDialog->open();
}

void MainWindow::on_actionRedo_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    if (ce) ce->extendedRedo();
}

void MainWindow::on_actionUndo_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    if (ce) ce->extendedUndo();
}

void MainWindow::on_actionPaste_triggered()
{
    CodeEdit *ce = ViewHelper::toCodeEdit(focusWidget());
    if (!ce || ce->isReadOnly()) return;
    ce->pasteClipboard();
}

void MainWindow::on_actionCopy_triggered()
{
    if (!focusWidget()) return;

#ifdef QWEBENGINE
    // Check if focusWidget is inside mHelpWidget (can be nested)
    QWidget *wid = focusWidget();
    while (wid && wid != mHelpWidget)
        wid = wid->parentWidget();
    if (mHelpWidget && wid == mHelpWidget) {
        mHelpWidget->copySelection();
        return;
    }
#endif

    // KEEP-ORDER: FIRST check focus THEN check recent-edit (in descending inheritance)

    if (TextView *tv = ViewHelper::toTextView(focusWidget())) {
        tv->copySelection();
    } else if (focusWidget() == mSyslog) {
        mSyslog->copy();
    } else if (gdxviewer::GdxViewer *gdx = ViewHelper::toGdxViewer(mRecent.editor())) {
        gdx->copyAction();
    } else if (option::SolverOptionWidget *sow = ViewHelper::toSolverOptionEdit(mRecent.editor())) {
        sow->copyAction();
    } else if (TextView *tv = ViewHelper::toTextView(mRecent.editor())) {
        tv->copySelection();
    } else if (CodeEdit *ce = ViewHelper::toCodeEdit(mRecent.editor())) {
        ce->copySelection();
    } else if (AbstractEdit *ae = ViewHelper::toAbstractEdit(mRecent.editor())) {
        ae->copy();
    }
}

void MainWindow::on_actionSelect_All_triggered()
{
    if (focusWidget() == ui->projectView){
        ui->projectView->selectAll();
        return;
    }

    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editor());
    if (!fm || !focusWidget()) return;

    if (fm->kind() == FileKind::Gdx) {
        gdxviewer::GdxViewer *gdx = ViewHelper::toGdxViewer(mRecent.editor());
        gdx->selectAllAction();
    } else if (focusWidget() == mSyslog) {
        mSyslog->selectAll();
    } else if (AbstractEdit *ae = ViewHelper::toAbstractEdit(focusWidget())) {
        ae->selectAll();
    } else if (TextView *tv = ViewHelper::toTextView(mRecent.editor())) {
        tv->selectAllText();
    } else if (option::SolverOptionWidget *so = ViewHelper::toSolverOptionEdit(mRecent.editor())) {
        so->selectAllOptions();
    } else if (option::GamsConfigEditor *guce = ViewHelper::toGamsConfigEditor(mRecent.editor())) {
        guce->selectAll();
    }
}

void MainWindow::on_expandAll()
{
    ui->projectView->expandAll();
}

void MainWindow::on_collapseAll()
{
    ui->projectView->collapseAll();
}

void MainWindow::on_actionCut_triggered()
{
    CodeEdit* ce= ViewHelper::toCodeEdit(focusWidget());
    if (!ce || ce->isReadOnly()) return;
    ce->cutSelection();
}

void MainWindow::on_actionReset_Zoom_triggered()
{
#ifdef QWEBENGINE
    if (helpWidget()->isAncestorOf(QApplication::focusWidget()) ||
        helpWidget()->isAncestorOf(QApplication::activeWindow())) {
        helpWidget()->resetZoom(); // reset help view
    } else
#endif
    {
        // reset all editors
        updateFonts(Settings::settings()->toInt(skEdFontSize), Settings::settings()->toString(skEdFontFamily));
    }
}

void MainWindow::on_actionZoom_Out_triggered()
{
#ifdef QWEBENGINE
    if (helpWidget()->isAncestorOf(QApplication::focusWidget()) ||
        helpWidget()->isAncestorOf(QApplication::activeWindow())) {
        helpWidget()->zoomOut();
    } else
#endif
    {
        zoomWidget(focusWidget(), -1);
    }
}

void MainWindow::on_actionZoom_In_triggered()
{
#ifdef QWEBENGINE
    if (helpWidget()->isAncestorOf(QApplication::focusWidget()) ||
        helpWidget()->isAncestorOf(QApplication::activeWindow())) {
        helpWidget()->zoomIn();
    } else
#endif
    {
        zoomWidget(focusWidget(), 1);
    }
}

void MainWindow::zoomWidget(QWidget *widget, int range)
{
    FontGroup fg;
    while (widget && !ViewHelper::toAbstractView(widget) && !ViewHelper::toLxiViewer(widget)
           && !ViewHelper::toTextView(widget) && !ViewHelper::toAbstractEdit(widget)
           && widget != mSyslog && widget != mWp && widget != centralWidget()
           && widget != mGamsParameterEditor->dockChild())
        widget = widget->parentWidget();
    if (lxiviewer::LxiViewer *lxi = ViewHelper::toLxiViewer(widget))
        widget = lxi;
    if (widget == centralWidget()) {
        if (ui->mainTabs->currentWidget() == mWp)
            emit mWp->zoomRequest(range);
        return;
    }
    FileMeta *fm = mFileMetaRepo.fileMeta(widget);
    if (fm)
        fg = fm->fontGroup();
    else if (widget == mSyslog)
        fg = fgLog;
    else if (widget == mGamsParameterEditor->dockChild())
        fg = fgTable;
    else if (widget == mWp) {
        emit mWp->zoomRequest(range);
        return;
    } else
        return;

    qreal fontSize = widget->font().pointSizeF() + range;
    setGroupFontSize(fg, fontSize);
}

void MainWindow::convertLowerUpper(bool toUpper)
{
    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    QTextCursor textCursor(ce->textCursor());
    int textCursorPosition(ce->textCursor().position());
    textCursor.select(QTextCursor::WordUnderCursor);
    ce->setTextCursor(textCursor);
    if (toUpper)
        ce->convertToUpper();
    else
        ce->convertToLower();
    textCursor.setPosition(textCursorPosition,QTextCursor::MoveAnchor);
    ce->setTextCursor(textCursor);
}

void MainWindow::resetLoadAmount()
{
    mStatusWidgets->setLoadAmount(1.0);
}

void MainWindow::on_actionSet_to_Uppercase_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce= ViewHelper::toCodeEdit(mRecent.editor());
    if (ce) {
        if (ce->textCursor().hasSelection())
            ce->convertToUpper();
        else
            convertLowerUpper(true);
    }
}

void MainWindow::on_actionSet_to_Lowercase_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    if (ce) {
        if (ce->textCursor().hasSelection())
            ce->convertToLower();
        else
            convertLowerUpper(false);
    }
}

void MainWindow::on_actionOverwrite_Mode_toggled(bool overwriteMode)
{
    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    mOverwriteMode = overwriteMode;
    if (ce && !ce->isReadOnly()) {
        ce->setOverwriteMode(overwriteMode);
        updateStatusMode();
    }
}

void MainWindow::on_actionIndent_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    if (!ce || ce->isReadOnly()) return;
    QPoint pos(-1,-1); QPoint anc(-1,-1);
    ce->getPositionAndAnchor(pos, anc);
    ce->indent(Settings::settings()->toInt(skEdTabSize), pos.y()-1, anc.y()-1);
}

void MainWindow::on_actionOutdent_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    if (!ce || ce->isReadOnly()) return;
    QPoint pos(-1,-1); QPoint anc(-1,-1);
    ce->getPositionAndAnchor(pos, anc);
    ce->indent(-Settings::settings()->toInt(skEdTabSize), pos.y()-1, anc.y()-1);
}

void MainWindow::on_actionDuplicate_Line_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly())
        ce->duplicateLine();
}

void MainWindow::on_actionRemove_Line_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly())
        ce->removeLine();
}

void MainWindow::on_actionComment_triggered()
{
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editor());
    if (!fm || !focusWidget() || !mRecent.editor()->isAncestorOf(focusWidget()))
        return;

    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly()) {
        ce->commentLine();
    } else  {
        if (option::SolverOptionWidget *so = ViewHelper::toSolverOptionEdit(mRecent.editor())) {
           so->toggleCommentOption();
        }
    }
}

void MainWindow::toggleDebugMode()
{
    mDebugMode = !mDebugMode;
    mProjectRepo.setDebugMode(mDebugMode);
}

void MainWindow::on_actionRestore_Recently_Closed_Tab_triggered()
{
    if (mClosedTabs.isEmpty())
        return;

    if (mClosedTabs.last()=="WELCOME_PAGE") {
        mClosedTabs.removeLast();
        mClosedTabsIndexes.removeLast();
        showWelcomePage();
        return;
    }
    QFile file(mClosedTabs.last());
    mClosedTabs.removeLast();
    if (file.exists()) {
        openFilePath(file.fileName());
        ui->mainTabs->tabBar()->moveTab(ui->mainTabs->currentIndex(), mClosedTabsIndexes.takeLast());
    } else
        on_actionRestore_Recently_Closed_Tab_triggered();
}

void MainWindow::on_actionSelect_encodings_triggered()
{
    int defCodec = Settings::settings()->toInt(skDefaultCodecMib);
    SelectEncodings se(encodingMIBs(), defCodec, this);
    if (se.exec() == QDialog::Accepted) {
        Settings::settings()->setInt(skDefaultCodecMib, se.defaultCodec());
        setEncodingMIBs(se.selectedMibs());
        Settings::settings()->save();
    }
}

void MainWindow::setExtendedEditorVisibility(bool visible)
{
    ui->actionToggle_Extended_Parameter_Editor->setChecked(visible);
}

void MainWindow::on_actionToggle_Extended_Parameter_Editor_toggled(bool checked)
{
    if (checked) {
        ui->actionToggle_Extended_Parameter_Editor->setIcon(Theme::icon(":/%1/hide"));
        ui->actionToggle_Extended_Parameter_Editor->setToolTip("<html><head/><body><p>Hide Extended Parameter Editor (<span style=\"font-weight:600;\">"+ui->actionToggle_Extended_Parameter_Editor->shortcut().toString()+"</span>)</p></body></html>");
    } else {
        ui->actionToggle_Extended_Parameter_Editor->setIcon(Theme::icon(":/%1/show") );
        ui->actionToggle_Extended_Parameter_Editor->setToolTip("<html><head/><body><p>Show Extended Parameter Editor (<span style=\"font-weight:600;\">"+ui->actionToggle_Extended_Parameter_Editor->shortcut().toString()+"</span>)</p></body></html>");
    }

    mGamsParameterEditor->setEditorExtended(checked);
}

void MainWindow::on_actionReset_Views_triggered()
{
    resetViews();
}

void MainWindow::resetViews()
{
    setWindowState(Qt::WindowNoState);
    ui->actionFull_Screen->setChecked(false);

    const QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
    for (QDockWidget* dock: dockWidgets) {
        dock->setFloating(false);
        dock->setVisible(true);

        if (dock == ui->dockProjectView) {
            addDockWidget(Qt::LeftDockWidgetArea, dock);
            resizeDocks(QList<QDockWidget*>() << dock, {width()/6}, Qt::Horizontal);
        } else if (dock == ui->dockProcessLog) {
            addDockWidget(Qt::RightDockWidgetArea, dock);
            resizeDocks(QList<QDockWidget*>() << dock, {width()/3}, Qt::Horizontal);
        } else if (dock == ui->dockHelpView) {
            dock->setVisible(false);
            addDockWidget(Qt::RightDockWidgetArea, dock);
            resizeDocks(QList<QDockWidget*>() << dock, {width()/3}, Qt::Horizontal);
        }
    }
    mGamsParameterEditor->setEditorExtended(false);
    ui->toolBar->setVisible(true);
    addDockWidget(Qt::TopDockWidgetArea, mGamsParameterEditor->extendedEditor());
    for (QWidget * wid : mFileMetaRepo.editors()) {
        if (lxiviewer::LxiViewer *lxi = ViewHelper::toLxiViewer(wid))
            lxi->resetView();
    }

    Settings::settings()->resetKeys(Settings::viewKeys());
    Settings::settings()->save();
}

void MainWindow::resizeOptionEditor(const QSize &size)
{
    mGamsParameterEditor->resize(size);
}

void MainWindow::setForeground()
{
#if defined (WIN32)
   HWND WinId= HWND(winId());
   if (this->windowState() & Qt::WindowMinimized) {
       this->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
   }
   DWORD foregroundThreadPId = GetWindowThreadProcessId(GetForegroundWindow(),nullptr);
   DWORD mwThreadPId = GetWindowThreadProcessId(WinId,nullptr);
   if (foregroundThreadPId != mwThreadPId) {
       AttachThreadInput(foregroundThreadPId,mwThreadPId,true);
       SetForegroundWindow(WinId);
       AttachThreadInput(foregroundThreadPId, mwThreadPId, false);
   } else {
       SetForegroundWindow(WinId);
   }
#else
    this->show();
    this->raise();
    this->activateWindow();
#endif
}

void MainWindow::setForegroundOSCheck()
{
    if (Settings::settings()->toBool(skForegroundOnDemand))
        setForeground();
}

void MainWindow::on_actionNextTab_triggered()
{
    QWidget *wid = focusWidget();
    QTabWidget *tabs = nullptr;
    while (wid) {
        if (wid == ui->mainTabs) {
           tabs = ui->mainTabs;
           break;
        }
        if (wid == ui->logTabs) {
           tabs = ui->logTabs;
           break;
        }
        wid = wid->parentWidget();
    }
    if (tabs && tabs->count() > 0)
        tabs->setCurrentIndex((tabs->count() + tabs->currentIndex() + 1) % tabs->count());
}

void MainWindow::on_actionPreviousTab_triggered()
{
    QWidget *wid = focusWidget();
    QTabWidget *tabs = nullptr;
    while (wid) {
        if (wid == ui->mainTabs) {
           tabs = ui->mainTabs;
           break;
        }
        if (wid == ui->logTabs) {
           tabs = ui->logTabs;
           break;
        }
        wid = wid->parentWidget();
    }
    if (tabs && tabs->count() > 0)
        tabs->setCurrentIndex((tabs->count() + tabs->currentIndex() - 1) % tabs->count());
}

void MainWindow::on_actionToggleBookmark_triggered()
{
    if (AbstractEdit* edit = ViewHelper::toAbstractEdit(mRecent.editor())) {
        edit->sendToggleBookmark();
    } else if (TextView* tv = ViewHelper::toTextView(mRecent.editor())) {
        tv->edit()->sendToggleBookmark();
    }
}

void MainWindow::on_actionNextBookmark_triggered()
{
    if (AbstractEdit* edit = ViewHelper::toAbstractEdit(mRecent.editor())) {
        edit->sendJumpToNextBookmark();
    } else if (TextView* tv = ViewHelper::toTextView(mRecent.editor())) {
        tv->edit()->sendJumpToNextBookmark();
    }
}

void MainWindow::on_actionPreviousBookmark_triggered()
{
    if (AbstractEdit* edit = ViewHelper::toAbstractEdit(mRecent.editor())) {
        edit->sendJumpToPrevBookmark();
    } else if (TextView* tv = ViewHelper::toTextView(mRecent.editor())) {
        tv->edit()->sendJumpToPrevBookmark();
    }
}

void MainWindow::on_actionRemoveBookmarks_triggered()
{
    mTextMarkRepo.removeBookmarks();
}

void MainWindow::on_actionDeleteScratchDirs_triggered()
{
    PExProjectNode* node = mProjectRepo.findProject(ViewHelper::groupId(mRecent.editor()));
    QString path = node ? QDir::toNativeSeparators(node->workDir()) : currentPath();

    QMessageBox msgBox;
    msgBox.setWindowTitle("Delete scratch directories");
    msgBox.setText("This will delete all scratch directories in your current workspace.\n"
                   "The current working directory is " + path);
    msgBox.setInformativeText("Delete scratch directories now?");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    if (msgBox.exec() == QMessageBox::Yes)
        deleteScratchDirs(path);
}

void MainWindow::deleteScratchDirs(const QString &path)
{
    if (path.isEmpty()) return;

    QDirIterator it(path, QDir::Dirs, QDirIterator::FollowSymlinks);
    QRegularExpression scratchDir("[\\/\\\\]225\\w\\w?$");
    while (it.hasNext()) {
        QDir dir(it.next());
        if (scratchDir.match(it.filePath()).hasMatch()) {
            if (!dir.removeRecursively()) {
                SysLogLocator::systemLog()->append("Could not remove scratch directory " + it.filePath());
            }
        }
    }
}

QFont MainWindow::getEditorFont(FontGroup fGroup, QString fontFamily, qreal pointSize)
{
    if (fGroup == FontGroup::fgTable) {
        if (fontFamily.isEmpty()) fontFamily = font().family();
        if (pointSize < 0.01) pointSize = mGroupFontSize.value(fGroup);
        if (pointSize < 0.01) pointSize = Settings::settings()->toInt(skEdFontSize) + mTableFontSizeDif;
    } else {
        if (fontFamily.isEmpty()) fontFamily = Settings::settings()->toString(skEdFontFamily);
        if (pointSize < 0.01) pointSize = mGroupFontSize.value(fGroup);
        if (pointSize < 0.01) pointSize = Settings::settings()->toInt(skEdFontSize);
    }
    QFont font(fontFamily);
    font.setPointSizeF(pointSize);
    if (fGroup != FontGroup::fgTable)
        font.setHintingPreference(QFont::PreferNoHinting);
    mGroupFontSize.insert(fGroup, pointSize);
    return font;
}

bool MainWindow::isMiroAvailable(bool printError)
{
    if (Settings::settings()->toString(skMiroInstallPath).isEmpty())
        return false;
    QFileInfo fileInfo(Settings::settings()->toString(skMiroInstallPath));
    bool state = fileInfo.exists() && !fileInfo.isDir() && fileInfo.isExecutable();
    if (!state && printError)
        appendSystemLogError("The MIRO installation location does not exist or does not point to a valid MIRO executable. Please check your settings.");
    return state;
}

bool MainWindow::validMiroPrerequisites()
{
    if (!isMiroAvailable()) {
        auto msg = QString("Could not find MIRO at %1. Please check your MIRO settings.")
                .arg(Settings::settings()->toString(skMiroInstallPath));
        SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
        return false;
    }

    return mRecent.project();
}

void MainWindow::openGdxDiffFile()
{
    QString diffFile = mGdxDiffDialog->lastDiffFile();

    QString input1 = mGdxDiffDialog->lastInput1();
    QString input2 = mGdxDiffDialog->lastInput2();
    if (diffFile.isEmpty())
        return;

    FileMeta *fmInput1 = mFileMetaRepo.fileMeta(input1);
    FileMeta *fmInput2 = mFileMetaRepo.fileMeta(input2);
    PExProjectNode* projectDiff   = nullptr;

    // if possible get the group to which both input files belong
    if (fmInput1 && fmInput2) {
        QVector<PExFileNode*> nodesInput1 = mProjectRepo.fileNodes(fmInput1->id());
        QVector<PExFileNode*> nodesInput2 = mProjectRepo.fileNodes(fmInput2->id());

        if (nodesInput1.size() == 1 && nodesInput2.size() == 1) {
            if (nodesInput1.first()->parentNode() == nodesInput2.first()->parentNode())
                projectDiff = nodesInput1.first()->assignedProject();
        }
    }
    // if no group was found, we try to open the file in the first node that contains the it
    if (projectDiff == nullptr) {
        FileMeta *fm = mFileMetaRepo.fileMeta(diffFile);
        if (fm) {
            QVector<PExFileNode*> v = mProjectRepo.fileNodes(fm->id());
            if(v.size() == 1)
                projectDiff = v.first()->assignedProject();
        }
    }
    if (FileMeta* fMeta = mFileMetaRepo.fileMeta(diffFile)) {
        if (fMeta->isOpen()) {
            bool changed = fMeta->refreshMetaData();
            if (gdxviewer::GdxViewer *gdx = ViewHelper::toGdxViewer(fMeta->editors().constFirst())) {
                gdx->setHasChanged(true);
                gdx->reload(fMeta->codec(), changed);
            }
        }
    }
    PExFileNode *node = mProjectRepo.findOrCreateFileNode(diffFile, projectDiff);
    openFile(node->file());
}

void MainWindow::setSearchWidgetPos(const QPoint& searchWidgetPos)
{
    mSearchWidgetPos = searchWidgetPos;
}

void MainWindow::on_actionFull_Screen_triggered()
{
    if (!ui->actionFull_Screen->isChecked()) {
        if (mMaximizedBeforeFullScreen)
            showMaximized();
        else
            showNormal();
    } else {
        mMaximizedBeforeFullScreen = isMaximized();
        showFullScreen();
    }
}

void MainWindow::on_actionDistraction_Free_Mode_toggled(bool checked)
{
    if (checked) { // collapse
        mWidgetStates[0] = ui->dockProjectView->isVisible();
        mWidgetStates[1] = mGamsParameterEditor->isEditorExtended();
        mWidgetStates[2] = ui->dockProcessLog->isVisible();
        mWidgetStates[3] = ui->dockHelpView->isVisible();

        ui->dockProjectView->setVisible(false);
        mGamsParameterEditor->setEditorExtended(false);
        ui->dockProcessLog->setVisible(false);
        ui->dockHelpView->setVisible(false);
    } else { // restore
        ui->dockProjectView->setVisible(mWidgetStates[0]);
        mGamsParameterEditor->setEditorExtended(mWidgetStates[1]);
        ui->dockProcessLog->setVisible(mWidgetStates[2]);
        ui->dockHelpView->setVisible(mWidgetStates[3]);
    }
}

void MainWindow::on_actionShowToolbar_triggered(bool checked)
{
    ui->toolBar->setVisible(checked);
}

void MainWindow::on_actionGoBack_triggered()
{
    CursorHistoryItem item = mNavigationHistory->goBack();
    restoreCursorPosition(item);
    updateCursorHistoryAvailability();
}

void MainWindow::on_actionGoForward_triggered()
{
    CursorHistoryItem item = mNavigationHistory->goForward();
    restoreCursorPosition(item);
    updateCursorHistoryAvailability();
}

void MainWindow::restoreCursorPosition(CursorHistoryItem item)
{
    if (!mNavigationHistory->itemValid(item)) return;

    mNavigationHistory->stopRecord();

    // check if the edit is still open
    if (item.edit == mPinView->widget()) {
        item.edit->setFocus();
    } else if (ui->mainTabs->indexOf(item.edit) > 0) {
        ui->mainTabs->setCurrentWidget(item.edit);
        item.edit->setFocus();
    } else {
        if (!item.filePath.isEmpty()) {
            if (item.pinKind == pkNone) {
                openFilePath(item.filePath, true);
            } else {
                FileMeta *fm = mFileMetaRepo.fileMeta(item.filePath);
                if (!fm->isOpen())
                    openFilePath(item.filePath, false);
                int tabInd = ui->mainTabs->indexOf(fm->editors().first());
                openPinView(tabInd, Qt::Orientation(item.pinKind));
                if (mPinView->widget()) mPinView->widget()->setFocus();
            }
        }
    }

    if (item.lineNr >= 0) {
        // restore text cursor if editor available
        if (CodeEdit* ce = ViewHelper::toCodeEdit(mNavigationHistory->currentEdit()))
            ce->jumpTo(item.lineNr, item.col);
        else if (TextView* tv = ViewHelper::toTextView(mNavigationHistory->currentEdit()))
            tv->jumpTo(item.lineNr, item.col, 0, true);
        // else: nothing to do
    }

    mNavigationHistory->startRecord();
}

void MainWindow::updateCursorHistoryAvailability()
{
    ui->actionGoBack->setEnabled(mNavigationHistory->canGoBackward());
    ui->actionGoForward->setEnabled(mNavigationHistory->canGoForward());
}

void MainWindow::on_actionFoldAllTextBlocks_triggered()
{
    if (CodeEdit* ce = ViewHelper::toCodeEdit(mNavigationHistory->currentEdit())) {
        ce->foldAll();
    }
}

void MainWindow::on_actionUnfoldAllTextBlocks_triggered()
{
    if (CodeEdit* ce = ViewHelper::toCodeEdit(mNavigationHistory->currentEdit())) {
        ce->unfoldAll();
    }
}

void MainWindow::on_actionFoldDcoTextBlocks_triggered()
{
    if (CodeEdit* ce = ViewHelper::toCodeEdit(mNavigationHistory->currentEdit())) {
        ce->foldAll(true);
    }
}

void MainWindow::printDocument()
{
    if (focusWidget() == mRecent.editor()) {
        auto* abstractEdit = ViewHelper::toAbstractEdit(mainTabs()->currentWidget());
        if (!abstractEdit) return;
        abstractEdit->print(&mPrinter);
    } else if (ViewHelper::editorType(recent()->editor()) == EditorType::lxiLstChild) {
        auto* lxiViewer = ViewHelper::toLxiViewer(mainTabs()->currentWidget());
        if (!lxiViewer) return;
        lxiViewer->print(&mPrinter);
    } else if (ViewHelper::editorType(recent()->editor()) == EditorType::txtRo) {
        auto* textViewer = ViewHelper::toTextView(mainTabs()->currentWidget());
        if (!textViewer) return;
        textViewer->print(&mPrinter);
    }
}


void MainWindow::on_actionPrint_triggered()
{
    if (!enabledPrintAction()) return;
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editor());
    if (!fm || !focusWidget()) return;
    int numberLines = 0;
    if (TextView *tv = ViewHelper::toTextView(fm->editors().constFirst())) {
        if (tv->lineCount() == tv->knownLines()) numberLines = tv->lineCount();
    } else {
        numberLines = fm->document()->lineCount();
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle("Print large file?");
    msgBox.setText("The file you intend to print contains " + QString::number(numberLines) + " lines. It might take several minutes to print. Are you sure you want to continue?");
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

#ifdef __APPLE__
    int ret = mPrintDialog->exec();
    if (ret == QDialog::Accepted)
        if (numberLines>5000) {
             int answer = msgBox.exec();
             if (answer == QMessageBox::No)
                 return;
        }
        printDocument();
#else
    if (numberLines>5000) {
        int answer = msgBox.exec();
        if (answer == QMessageBox::No) return;
    }
    mPrintDialog->open(this, SLOT(printDocument()));
#endif
}

bool MainWindow::enabledPrintAction()
{
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editor());
    if (!fm || !focusWidget())
        return false;
    return focusWidget() == mRecent.editor()
            || ViewHelper::editorType(recent()->editor()) == EditorType::lxiLstChild
            || ViewHelper::editorType(recent()->editor()) == EditorType::txtRo;
}

void MainWindow::checkGamsLicense()
{
    const QString errorText = "No GAMS license found. You can install your license by"
                              " copying the license information from the email you"
                              " received after purchasing GAMS into your clipboard, and"
                              " then open the license dialogue (Help / GAMS licensing)."
                              " The license will be recognized and installed automatically."
                              " For more options, please check the GAMS documentation.";
    try {
        support::GamsLicensingDialog::createLicenseFile(this);
        auto dataPaths = support::GamsLicenseInfo().gamsDataLocations();
        auto licenseFile = QDir::toNativeSeparators(CommonPaths::gamsLicenseFilePath(dataPaths));
        if (QFileInfo::exists(licenseFile)) {
            appendSystemLogInfo("GAMS license found at " + licenseFile);
        } else {
            appendSystemLogError(errorText);
        }
    }  catch (Exception *e) {
        appendSystemLogError(e->what());
    }
}

void MainWindow::checkSslLibrary()
{
    if (!QSslSocket::supportsSsl()) {
        QString sslVersion = QSslSocket::sslLibraryVersionString();
        if (sslVersion.isEmpty())
            appendSystemLogWarning("SSL library not found");
        else
            appendSystemLogWarning("Incompatible SSL library found: " + sslVersion);
    }
}

void MainWindow::on_actionMove_Line_Up_triggered()
{
    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    if (!ce || ce->isReadOnly()) return;
    else {
        ce->moveLines(true);
    }
}

void MainWindow::on_actionMove_Line_Down_triggered()
{
    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    if (!ce || ce->isReadOnly()) return;
    else {
        ce->moveLines(false);
    }
}

void MainWindow::on_actionNew_Project_triggered()
{
    newProjectDialog();
}

void MainWindow::on_actionImport_Project_triggered()
{
    importProjectDialog();
}


void MainWindow::on_actionExport_Project_triggered()
{
    PExProjectNode *project = mRecent.project();
    if (!project) return;
    exportProjectDialog(project);
}


void MainWindow::on_actionPin_Right_triggered()
{
    openPinView(ui->mainTabs->currentIndex(), Qt::Horizontal);
}


void MainWindow::on_actionPin_Below_triggered()
{
    openPinView(ui->mainTabs->currentIndex(), Qt::Vertical);
}


}
}
