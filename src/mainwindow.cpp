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
#include <QtConcurrent>
#include <QtWidgets>
#include <QPrintDialog>
#include <QPrinter>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "editors/codeedit.h"
#include "editors/processlogedit.h"
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
#include "search/searchlocator.h"
#include "search/searchresultlist.h"
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
#include "miro/miromodelassemblydialog.h"
#include "process/gamsinstprocess.h"
#include "confirmdialog.h"
#include "fileeventhandler.h"
#include "engine/enginestartdialog.h"
#include "neos/neosstartdialog.h"

#ifdef __APPLE__
#include "../platform/macos/macoscocoabridge.h"
#endif

namespace gams {
namespace studio {

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
      mMiroDeployDialog(new miro::MiroDeployDialog(this)),
      mMiroAssemblyDialog(new miro::MiroModelAssemblyDialog(this))
{
    mTextMarkRepo.init(&mFileMetaRepo, &mProjectRepo);
    Settings *settings = Settings::settings();
    initEnvironment();

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
#else
    ui->actionFull_Screen->setShortcuts({QKeySequence("Alt+Enter"), QKeySequence("Alt+Return")});
#endif
    ui->actionDistraction_Free_Mode->setShortcuts({QKeySequence("Ctrl+Alt+Enter"), QKeySequence("Ctrl+Alt+Return")});

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Equal), this, SLOT(on_actionZoom_In_triggered()));
    ui->actionGoForward->setShortcut(QKeySequence(QKeySequence::Forward));
    ui->actionGoBack->setShortcut(QKeySequence(QKeySequence::Back));
    ui->actionGoForward->shortcuts().append(QKeySequence(Qt::ForwardButton));
    ui->actionGoBack->shortcuts().append(QKeySequence(Qt::BackButton));

    // Status Bar
    QFont font = ui->statusBar->font();
    font.setPointSizeF(font.pointSizeF()*0.9);
    ui->statusBar->setFont(font);
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

    mProjectRepo.init(ui->projectView, &mFileMetaRepo, &mTextMarkRepo);
    mFileMetaRepo.init(&mTextMarkRepo, &mProjectRepo);

#ifdef QWEBENGINE
    mHelpWidget = new help::HelpWidget(this);
    ui->dockHelpView->setWidget(mHelpWidget);
    ui->dockHelpView->hide();
#endif

    initToolBar();

    mCodecGroupReload = new QActionGroup(this);
    connect(mCodecGroupReload, &QActionGroup::triggered, this, &MainWindow::codecReload);
    mCodecGroupSwitch = new QActionGroup(this);
    connect(mCodecGroupSwitch, &QActionGroup::triggered, this, &MainWindow::codecChanged);
    connect(ui->mainTabs, &QTabWidget::currentChanged, this, &MainWindow::activeTabChanged);
    connect(ui->mainTabs, &QTabWidget::currentChanged, this, &MainWindow::on_menuFile_aboutToShow);

    connect(&mFileMetaRepo, &FileMetaRepo::fileEvent, this, &MainWindow::fileEvent);
    connect(&mFileMetaRepo, &FileMetaRepo::editableFileSizeCheck, this, &MainWindow::editableFileSizeCheck);
    connect(&mProjectRepo, &ProjectRepo::openFile, this, &MainWindow::openFile);
    connect(&mProjectRepo, &ProjectRepo::setNodeExpanded, this, &MainWindow::setProjectNodeExpanded);
    connect(&mProjectRepo, &ProjectRepo::isNodeExpanded, this, &MainWindow::isProjectNodeExpanded);
    connect(&mProjectRepo, &ProjectRepo::gamsProcessStateChanged, this, &MainWindow::gamsProcessStateChanged);
    connect(&mProjectRepo, &ProjectRepo::getParameterValue, this, &MainWindow::getParameterValue);
    connect(&mProjectRepo, &ProjectRepo::closeFileEditors, this, &MainWindow::closeFileEditors);

    connect(ui->projectView, &QTreeView::customContextMenuRequested, this, &MainWindow::projectContextMenuRequested);
    connect(&mProjectContextMenu, &ProjectContextMenu::closeGroup, this, &MainWindow::closeGroup);
    connect(&mProjectContextMenu, &ProjectContextMenu::renameGroup, &mProjectRepo, &ProjectRepo::renameGroup);
    connect(&mProjectContextMenu, &ProjectContextMenu::closeFile, this, &MainWindow::closeNodeConditionally);
    connect(&mProjectContextMenu, &ProjectContextMenu::addExistingFile, this, &MainWindow::addToGroup);
    connect(&mProjectContextMenu, &ProjectContextMenu::getSourcePath, this, &MainWindow::sendSourcePath);
    connect(&mProjectContextMenu, &ProjectContextMenu::newFileDialog, this, &MainWindow::newFileDialog);
    connect(&mProjectContextMenu, &ProjectContextMenu::setMainFile, this, &MainWindow::setMainGms);
    connect(&mProjectContextMenu, &ProjectContextMenu::openLogFor, this, &MainWindow::changeToLog);
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

    connect(ui->dockProjectView, &QDockWidget::visibilityChanged, this, &MainWindow::projectViewVisibiltyChanged);
    connect(ui->dockProcessLog, &QDockWidget::visibilityChanged, this, &MainWindow::outputViewVisibiltyChanged);
    connect(ui->dockHelpView, &QDockWidget::visibilityChanged, this, &MainWindow::helpViewVisibilityChanged);
    connect(ui->toolBar, &QToolBar::visibilityChanged, this, &MainWindow::toolbarVisibilityChanged);

    ui->dockProjectView->installEventFilter(this);
    ui->dockProcessLog->installEventFilter(this);
    ui->dockHelpView->installEventFilter(this);

    connect(this, &MainWindow::saved, this, &MainWindow::on_actionSave_triggered);
    connect(this, &MainWindow::savedAs, this, &MainWindow::on_actionSave_As_triggered);

    connect(mGdxDiffDialog.get(), &QDialog::accepted, this, &MainWindow::openGdxDiffFile);
    connect(mMiroDeployDialog.get(), &miro::MiroDeployDialog::accepted,
            this, [this](){ miroDeploy(false, miro::MiroDeployMode::None); });
    connect(mMiroDeployDialog.get(), &miro::MiroDeployDialog::testDeploy,
            this, &MainWindow::miroDeploy);
    connect(mMiroDeployDialog.get(), &miro::MiroDeployDialog::newAssemblyFileData,
            this, &MainWindow::writeNewAssemblyFileData);
    connect(mMiroAssemblyDialog.get(), &miro::MiroModelAssemblyDialog::finished,
            this, &MainWindow::miroAssemblyDialogFinish);

    setEncodingMIBs(encodingMIBs());
    ui->menuEncoding->setEnabled(false);

    initIcons();
    restoreFromSettings();
    mSearchDialog = new search::SearchDialog(this);

    // stack help under output
    if (tabifiedDockWidgets(ui->dockHelpView).contains(ui->dockProcessLog)) {
        ui->dockHelpView->setFloating(true);
        tabifyDockWidget(ui->dockProcessLog, ui->dockHelpView);
        ui->dockHelpView->setFloating(false);
    }

    mSyslog = new SystemLogEdit(this);
    ViewHelper::initEditorType(mSyslog, EditorType::syslog);
    mSyslog->setFont(createEditorFont(settings->toString(skEdFontFamily), settings->toInt(skEdFontSize)));
    on_actionShow_System_Log_triggered();

    mNavigationHistory = new NavigationHistory(this);
    NavigationHistoryLocator::provide(mNavigationHistory);
    connect(mNavigationHistory, &NavigationHistory::historyChanged, this, &MainWindow::updateCursorHistoryAvailability);

    initWelcomePage();

    QPalette pal = ui->projectView->palette();
    pal.setColor(QPalette::Highlight, Qt::transparent);
    ui->projectView->setPalette(pal);

    mNavigationHistory->startRecord();
    QPushButton *tabMenu = new QPushButton(Scheme::icon(":/%1/menu"), "", ui->mainTabs);
    connect(tabMenu, &QPushButton::pressed, this, &MainWindow::showMainTabsMenu);
    tabMenu->setMaximumWidth(40);
    ui->mainTabs->setCornerWidget(tabMenu);
    tabMenu = new QPushButton(Scheme::icon(":/%1/menu"), "", ui->logTabs);
    connect(tabMenu, &QPushButton::pressed, this, &MainWindow::showLogTabsMenu);
    tabMenu->setMaximumWidth(40);
    ui->logTabs->setCornerWidget(tabMenu);
    ui->mainTabs->setUsesScrollButtons(true);

    // set up services
    search::SearchLocator::provide(mSearchDialog);
    SysLogLocator::provide(mSyslog);
    QTimer::singleShot(0, this, &MainWindow::openInitialFiles);

    ui->menuMIRO->setEnabled(isMiroAvailable());

    // Themes
#ifdef __APPLE__
    ViewHelper::setAppearance(MacOSCocoaBridge::isDarkMode() ? 1 : 0);
#else
    // this needs to be re-called for studio startup, as the call when loading settings is too early
    ViewHelper::setAppearance();
#endif
    connect(Scheme::instance(), &Scheme::changed, this, &MainWindow::invalidateScheme);
    invalidateScheme();
    initGamsStandardPaths();
    updateRunState();

    checkGamsLicense();
}

void MainWindow::watchProjectTree()
{
    connect(&mProjectRepo, &ProjectRepo::changed, this, &MainWindow::storeTree);
    connect(&mProjectRepo, &ProjectRepo::childrenChanged, this, [this]() {
        mRecent.setEditor(mRecent.editor(), this);
        updateRunState();
    });
    mStartedUp = true;
}

MainWindow::~MainWindow()
{
    killTimer(mTimerID);
    delete mWp;
    delete ui;
    delete mNavigationHistory;
    FileType::clear();
}

void MainWindow::setInitialFiles(QStringList files)
{
    mInitialFiles = files;
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

void MainWindow::initIcons()
{
    setWindowIcon(Scheme::icon(":/img/gams-w"));
    ui->actionCompile->setIcon(Scheme::icon(":/%1/code"));
    ui->actionCompile_with_GDX_Creation->setIcon(Scheme::icon(":/%1/code-gdx"));
    ui->actionCopy->setIcon(Scheme::icon(":/%1/copy"));
    ui->actionCut->setIcon(Scheme::icon(":/%1/cut"));
    ui->actionClose->setIcon(Scheme::icon(":/%1/remove"));
    ui->actionExit_Application->setIcon(Scheme::icon(":/%1/door-open"));
    ui->actionGAMS_Library->setIcon(Scheme::icon(":/%1/books"));
    ui->actionGDX_Diff->setIcon(Scheme::icon(":/%1/gdxdiff"));
    ui->actionHelp_View->setIcon(Scheme::icon(":/%1/question"));
    ui->actionInterrupt->setIcon(Scheme::icon(":/%1/stop"));
    ui->actionNew->setIcon(Scheme::icon(":/%1/file"));
    ui->actionNextBookmark->setIcon(Scheme::icon(":/%1/forward"));
    ui->actionOpen->setIcon(Scheme::icon(":/%1/folder-open-bw"));
    ui->actionPaste->setIcon(Scheme::icon(":/%1/paste"));
    ui->actionPreviousBookmark->setIcon(Scheme::icon(":/%1/backward"));
    ui->actionProcess_Log->setIcon(Scheme::icon(":/%1/output"));
    ui->actionProject_View->setIcon(Scheme::icon(":/%1/project"));
    ui->actionRedo->setIcon(Scheme::icon(":/%1/redo"));
    ui->actionReset_Zoom->setIcon(Scheme::icon(":/%1/search-off"));
    ui->actionRun->setIcon(Scheme::icon(":/%1/play"));
    ui->actionRun_with_GDX_Creation->setIcon(Scheme::icon(":/%1/run-gdx"));
    ui->actionRunNeos->setIcon(Scheme::icon(":/img/neos", false, ":/img/neos-g"));
    ui->actionRunEngine->setIcon(Scheme::icon(":/img/engine", false, ":/img/engine-g"));
    ui->actionSave->setIcon(Scheme::icon(":/%1/save"));
    ui->actionSearch->setIcon(Scheme::icon(":/%1/search"));
    ui->actionSettings->setIcon(Scheme::icon(":/%1/cog"));
    ui->actionStop->setIcon(Scheme::icon(":/%1/kill"));
    ui->actionTerminal->setIcon(Scheme::icon(":/%1/terminal"));
    ui->actionToggleBookmark->setIcon(Scheme::icon(":/%1/bookmark"));
    ui->actionToggle_Extended_Parameter_Editor->setIcon(Scheme::icon(":/%1/show"));
    ui->actionUndo->setIcon(Scheme::icon(":/%1/undo"));
    ui->actionUpdate->setIcon(Scheme::icon(":/%1/update"));
    ui->actionZoom_In->setIcon(Scheme::icon(":/%1/search-plus"));
    ui->actionZoom_Out->setIcon(Scheme::icon(":/%1/search-minus"));
    ui->actionShowToolbar->setIcon(Scheme::icon(":/%1/hammer"));
    ui->actionHelp->setIcon(Scheme::icon(":/%1/book"));
    ui->actionChangelog->setIcon(Scheme::icon(":/%1/new"));
    ui->actionGoForward->setIcon(Scheme::icon(":/%1/forward"));
    ui->actionGoBack->setIcon(Scheme::icon(":/%1/backward"));
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

    ProjectGroupNode *group = mProjectRepo.createGroup(fi.completeBaseName(), fi.absolutePath(), "");
    ProjectFileNode *node = addNode("", filePath, group);
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
#ifdef __APPLE__
        // reload theme when switching OS theme
        Scheme::instance()->setActiveScheme(MacOSCocoaBridge::isDarkMode() ? 1 : 0, Scheme::StudioScope);
        Scheme::instance()->setActiveScheme(MacOSCocoaBridge::isDarkMode() ? 1 : 0, Scheme::EditorScope);
#endif
    }
    return QMainWindow::event(event);
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event)
    // TODO(JM) this can be replaced later, Qt 5.14. provides ::screen() getter
    QScreen *screen = window()->windowHandle()->screen();
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

    // TODO(JM) this can be replaced later, Qt 5.14. provides ::screen() getter
    QScreen *screen = window()->windowHandle()->screen();
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
    mInstProcess = new process::GamsInstProcess(this);
    connect(mInstProcess, &process::GamsInstProcess::finished,
            this, &MainWindow::gamsInstFinished);
    connect(mInstProcess, &process::GamsInstProcess::newProcessCall,
            this, &MainWindow::newProcessCall);
    mInstProcess->execute();
}

void MainWindow::gamsInstFinished(NodeId origin, int exitCode)
{
    Q_UNUSED(origin)
    if (exitCode) return;
    if (!mInstProcess) return;
    CommonPaths::setGamsStandardPaths(mInstProcess->configPaths(), CommonPaths::StandardConfigPath);
    CommonPaths::setGamsStandardPaths(mInstProcess->dataPaths(), CommonPaths::StandardDataPath);
    mInstProcess->deleteLater();
    mInstProcess = nullptr;
}

void MainWindow::getParameterValue(QString param, QString &value)
{
    bool joker = param.endsWith('*');
    if (joker) param.resize(param.size()-1);
    QString params = mGamsParameterEditor->getCurrentCommandLineData();
    params = mGamsParameterEditor->getOptionTokenizer()->normalize(params);
    QList<option::OptionItem> parList = mGamsParameterEditor->getOptionTokenizer()->tokenize(params);
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

void MainWindow::addToGroup(ProjectGroupNode* group, const QString& filepath)
{
    openFileNode(mProjectRepo.findOrCreateFileNode(filepath, group), true);
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
    QFileInfo file(model->files().first());
    QString inputFile = file.completeBaseName() + ".gms";

    openModelFromLib(glbFile, model->nameWithSuffix(), inputFile);
}

void MainWindow::openModelFromLib(const QString &glbFile, const QString &modelName, const QString &inputFile, bool forceOverwrite)
{
    QString gmsFilePath = Settings::settings()->toString(skDefaultWorkspace) + "/" + inputFile;
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
            case 0: // open
                openFileNode(addNode("", gmsFilePath));
                return;
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
    mLibProcess->setWorkingDirectory(Settings::settings()->toString(skDefaultWorkspace));

    QStringList args {
        "-lib",
        QDir::toNativeSeparators(gamsSysDir.filePath(glbFile)),
        (modelName.isEmpty() ? QString::number(-1) : modelName),
        QDir::toNativeSeparators(Settings::settings()->toString(skDefaultWorkspace))
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
    QStringList strMibs = mibList.split(",");
    for (QString mib: strMibs) {
        if (mib.length()) mibs << mib.toInt();
    }
    setEncodingMIBs(mibs, active);
}

void MainWindow::setEncodingMIBs(QList<int> mibs, int active)
{
    while (mCodecGroupSwitch->actions().size()) {
        QAction *act = mCodecGroupSwitch->actions().last();
        if (ui->menuconvert_to->actions().contains(act))
            ui->menuconvert_to->removeAction(act);
        mCodecGroupSwitch->removeAction(act);
    }
    while (mCodecGroupReload->actions().size()) {
        QAction *act = mCodecGroupReload->actions().last();
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

void MainWindow::gamsProcessStateChanged(ProjectGroupNode* group)
{
    if (mRecent.group() == group) updateRunState();

    ProjectRunGroupNode* runGroup = group->toRunGroup();
    ProjectLogNode* log = runGroup->logNode();

    QTabBar::ButtonPosition closeSide = QTabBar::ButtonPosition(style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, nullptr, this));
    for (int i = 0; i < ui->logTabs->children().size(); i++) {
        if (mFileMetaRepo.fileMeta(ui->logTabs->widget(i)) == log->file()) {

            if (runGroup->gamsProcessState() == QProcess::Running)
                ui->logTabs->tabBar()->tabButton(i, closeSide)->hide();
            else if (runGroup->gamsProcessState() == QProcess::NotRunning)
                ui->logTabs->tabBar()->tabButton(i, closeSide)->show();
        }
    }
}

void MainWindow::projectContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = ui->projectView->indexAt(pos);
    QModelIndexList list = ui->projectView->selectionModel()->selectedIndexes();
    if (!index.isValid() && list.isEmpty()) return;
    QVector<ProjectAbstractNode*> nodes;
    for (NodeId id: mProjectRepo.treeModel()->selectedIds()) {
        nodes << mProjectRepo.node(id);
    }
    if (nodes.empty()) return;

    mProjectContextMenu.setNodes(nodes);
    mProjectContextMenu.setParent(this);
    mProjectContextMenu.exec(ui->projectView->viewport()->mapToGlobal(pos));
}

void MainWindow::mainTabContextMenuRequested(const QPoint& pos)
{
    int tabIndex = ui->mainTabs->tabBar()->tabAt(pos);
    mMainTabContextMenu.setTabIndex(tabIndex);
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
    for (QDockWidget* dw : dwList) {
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
    for (QDockWidget* dw : dwList) {
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

void MainWindow::updateEditorPos()
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

void MainWindow::updateEditorMode()
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

void MainWindow::updateEditorBlockCount()
{
    if (AbstractEdit* edit = ViewHelper::toAbstractEdit(mRecent.editor()))
        mStatusWidgets->setLineCount(edit->blockCount());
    else if (TextView *tv = ViewHelper::toTextView(mRecent.editor()))
        mStatusWidgets->setLineCount(tv->lineCount());
}

void MainWindow::updateLoadAmount()
{
    if (TextView *tv = ViewHelper::toTextView(mRecent.editor())) {
        qreal amount = qAbs(qreal(tv->knownLines()) / tv->lineCount());
        mStatusWidgets->setLoadAmount(amount);
    }
}

void MainWindow::updateEditorItemCount()
{
    option::SolverOptionWidget* edit = ViewHelper::toSolverOptionEdit(mRecent.editor());
    if (edit) mStatusWidgets->setLineCount(edit->getItemCount());
}

void MainWindow::currentDocumentChanged(int from, int charsRemoved, int charsAdded)
{
    if (!searchDialog()->searchTerm().isEmpty())
        searchDialog()->on_documentContentChanged(from, charsRemoved, charsAdded);
}

void MainWindow::getAdvancedActions(QList<QAction*>* actions)
{
    QList<QAction*> act(ui->menuAdvanced->actions());
    *actions = act;
}

void MainWindow::newFileDialog(QVector<ProjectGroupNode*> groups, const QString& solverName)
{
    QString path = (!groups.isEmpty()) ? groups.first()->location() : currentPath();
    if (path.isEmpty()) path = ".";

    if (mRecent.editFileId() >= 0) {
        FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editFileId());
        if (fm) path = QFileInfo(fm->location()).path();
    }

    if (solverName.isEmpty()) {
        // find a free file name
        int nr = 1;
        while (QFileInfo(path, QString("new%1.gms").arg(nr)).exists()) ++nr;
        path += QString("/new%1.gms").arg(nr);
    } else {
        int nr = 1;
        QString suffix = "opt";
        QString filename = QString("%1.%2").arg(solverName).arg(suffix);
        while (QFileInfo(path, filename).exists()) {
            ++nr;  // note: "op1" is invalid
            if (nr<10) suffix = "op";
            else if (nr<100) suffix = "o";
            else suffix = "";
            filename = QString("%1.%2%3").arg(solverName).arg(suffix).arg(nr);
        }
        path += QString("/%1").arg(filename);
    }

    QString filePath = solverName.isEmpty()
                             ? QFileDialog::getSaveFileName(this, "Create new file...",
                                                            path,
                                                            ViewHelper::dialogFileFilterUserCreated().join(";;"), nullptr, QFileDialog::DontConfirmOverwrite)
                             : QFileDialog::getSaveFileName(this, QString("Create new %1 option file...").arg(solverName),
                                                            path,
                                                            tr(QString("%1 option file (%1*);;All files (*)").arg(solverName).toLatin1()),
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

    if (!groups.isEmpty()) { // add file to each selected group
        for (ProjectGroupNode *group: groups)
            openFileNode(addNode("", filePath, group));
    } else { // create new group
        ProjectGroupNode *group = mProjectRepo.createGroup(fi.completeBaseName(), fi.absolutePath(), "");
        ProjectFileNode* node = addNode("", filePath, group);
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
    newFileDialog();
}

void MainWindow::on_actionOpen_triggered()
{
    QString path = currentPath();
    QStringList files = QFileDialog::getOpenFileNames(this, "Open file", path,
                                                      ViewHelper::dialogFileFilterAll().join(";;"),
                                                      nullptr,
                                                      DONT_RESOLVE_SYMLINKS_ON_MACOS);
    openFiles(files);
}

void MainWindow::on_actionOpenNew_triggered()
{
    QString path = currentPath();
    QStringList files = QFileDialog::getOpenFileNames(this, "Open file", path,
                                                      ViewHelper::dialogFileFilterAll().join(";;"),
                                                      nullptr,
                                                      DONT_RESOLVE_SYMLINKS_ON_MACOS);
    openFiles(files, true);
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
    ProjectFileNode *node = mProjectRepo.findFileNode(mRecent.editor());
    if (!node) return;
    FileMeta *fileMeta = node->file();
    int choice = 0;
    QString filePath = fileMeta->location();
    QFileInfo fi(filePath);
    while (choice < 1) {
        QStringList filters;
        if (fileMeta->kind() == FileKind::Opt) {
            filters << tr( QString("%1 option files (%1*)").arg(fi.baseName()).toLatin1() );
            filters << tr("All files (*)");
            filePath = QFileDialog::getSaveFileName(this, "Save file as...",
                                                    filePath, filters.join(";;"),
                                                    &filters.first(),
                                                    QFileDialog::DontConfirmOverwrite);
        } else {
            filters = ViewHelper::dialogFileFilterAll();

            QString selFilter = filters.first();
            for (QString f: filters) {
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
                                                      .arg(QFileInfo(filePath).completeBaseName()).arg(QFileInfo(fileMeta->location()).completeBaseName()).arg(QFileInfo(filePath).fileName())
                                               , "Select other", "Continue", "Abort", 0, 2);
        if (choice == 0)
            continue;
        else if (choice == 2)
                 break;

        choice = 1;
        if (FileType::from(fileMeta->kind()) != FileType::from(QFileInfo(filePath).suffix())) {
            if (fileMeta->kind() == FileKind::Opt)
                choice = QMessageBox::question(this, "Invalid Option File Suffix"
                                                   , QString("'%1' is not a valid option file suffix. Saved file '%2' may not be displayed properly.")
                                                          .arg(QFileInfo(filePath).suffix()).arg(QFileInfo(filePath).fileName())
                                                   , "Select other", "Continue", "Abort", 0, 2);
            else
                choice = QMessageBox::question(this, "Different File Type"
                                                   , QString("Suffix '%1' is of different type than source file suffix '%2'. Saved file '%3' may not be displayed properly.")
                                                          .arg(QFileInfo(filePath).suffix()).arg(QFileInfo(fileMeta->location()).suffix()).arg(QFileInfo(filePath).fileName())
                                                   , "Select other", "Continue", "Abort", 0, 2);
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
                if (ProjectFileNode* pfn = mProjectRepo.findFile(filePath, node->assignedRunGroup()))
                    mProjectRepo.closeNode(pfn);

                mProjectRepo.saveNodeAs(node, filePath);

                if (oldKind == node->file()->kind()) { // if old == new
                    ui->mainTabs->tabBar()->setTabText(ui->mainTabs->currentIndex(), fileMeta->name(NameModifier::editState));
                } else { // reopen in new editor
                    int index = ui->mainTabs->currentIndex();
                    openFileNode(node, true);
                    on_mainTabs_tabCloseRequested(index);
                }
                mStatusWidgets->setFileName(filePath);

                updateAndSaveSettings();
            }
        }
        if (choice == 1) {
//            mRecent.path = QFileInfo(filePath).path();
            ProjectFileNode* newNode =
                    mProjectRepo.findOrCreateFileNode(filePath, node->assignedRunGroup());
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
        mStatusWidgets->setEncoding(fm->codecMib());
    }
}

void MainWindow::codecReload(QAction *action)
{
    if (!focusWidget()) return;
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editFileId());
    if (fm && fm->kind() == FileKind::Log) return;
    if (fm && fm->kind() == FileKind::Guc) return;
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
            mStatusWidgets->setEncoding(fm->codecMib());
        }
    }
}

void MainWindow::loadCommandLines(ProjectFileNode* oldfn, ProjectFileNode* fn)
{
    if (oldfn) { // switch from a non-welcome page
        ProjectRunGroupNode* oldgroup = oldfn->assignedRunGroup();
        if (!oldgroup) return;
        oldgroup->addRunParametersHistory( mGamsParameterEditor->getCurrentCommandLineData() );

        if (!fn) { // switch to a welcome page
            mGamsParameterEditor->loadCommandLine(QStringList());
            return;
        }

        ProjectRunGroupNode* group = fn->assignedRunGroup();
        if (!group) return;
        if (group == oldgroup) return;

        mGamsParameterEditor->loadCommandLine( group->getRunParametersHistory() );

    } else { // switch from a welcome page
        if (!fn) { // switch to a welcome page
            mGamsParameterEditor->loadCommandLine(QStringList());
            return;
        }

        ProjectRunGroupNode* group = fn->assignedRunGroup();
        if (!group) return;
        mGamsParameterEditor->loadCommandLine( group->getRunParametersHistory() );
    }
}

void MainWindow::activeTabChanged(int index)
{
    ProjectFileNode* oldTab = mProjectRepo.findFileNode(mRecent.editor());
    QWidget *editWidget = (index < 0 ? nullptr : ui->mainTabs->widget(index));
    ProjectFileNode* node = mProjectRepo.findFileNode(editWidget);
    if (mStartedUp)
        mProjectRepo.editorActivated(editWidget, focusWidget() != ui->projectView);
    mRecent.setEditor(editWidget, this);
    if (CodeEdit* ce = ViewHelper::toCodeEdit(editWidget))
        ce->updateExtraSelections();
    else if (TextView* tv = ViewHelper::toTextView(editWidget))
        tv->updateExtraSelections();

    if (node) {
        changeToLog(node, false, false);
        mStatusWidgets->setFileName(QDir::toNativeSeparators(node->location()));
        mStatusWidgets->setEncoding(node->file()->codecMib());

        if (AbstractEdit* edit = ViewHelper::toAbstractEdit(editWidget)) {
            mStatusWidgets->setLineCount(edit->blockCount());
            ui->menuEncoding->setEnabled(node && !edit->isReadOnly());
            ui->menuconvert_to->setEnabled(node && !edit->isReadOnly());
        } else if (TextView* tv = ViewHelper::toTextView(editWidget)) {
            ui->menuEncoding->setEnabled(true);
            ui->menuconvert_to->setEnabled(false);
            try {
                mStatusWidgets->setLineCount(tv->lineCount());
            } catch (Exception &e) {
//                QMessageBox::warning(this, "Exception", e.what());
                if (fileRepo()->fileMeta(tv))
                    closeFileEditors(fileRepo()->fileMeta(tv)->id());
                e.raise();
            }
        } else if (ViewHelper::toGdxViewer(editWidget)) {
            ui->menuconvert_to->setEnabled(false);
            mStatusWidgets->setLineCount(-1);
            node->file()->reload();
        } else if (reference::ReferenceViewer* refViewer = ViewHelper::toReferenceViewer(editWidget)) {
            ui->menuEncoding->setEnabled(false);
            ui->menuconvert_to->setEnabled(false);
            if (ProjectFileNode* fc = mProjectRepo.findFileNode(refViewer)) {
                ui->menuconvert_to->setEnabled(false);
                mStatusWidgets->setFileName(QDir::toNativeSeparators(fc->location()));
                mStatusWidgets->setEncoding(fc->file()->codecMib());
                mStatusWidgets->setLineCount(-1);
                updateMenuToCodec(node->file()->codecMib());
            }
        } else if (option::SolverOptionWidget* solverOptionEditor = ViewHelper::toSolverOptionEdit(editWidget)) {
            ui->menuEncoding->setEnabled(false);
            if (ProjectFileNode* fc = mProjectRepo.findFileNode(solverOptionEditor)) {
                ui->menuEncoding->setEnabled(true);
                ui->menuconvert_to->setEnabled(true);
                mStatusWidgets->setFileName(fc->location());
                mStatusWidgets->setEncoding(fc->file()->codecMib());
                mStatusWidgets->setLineCount(solverOptionEditor->getItemCount());
                node->file()->reload();
                updateMenuToCodec(node->file()->codecMib());
            }
        } else if (option::GamsConfigEditor* gucEditor = ViewHelper::toGamsConfigEditor((editWidget))) {
            ui->menuEncoding->setEnabled(false);
            if (ProjectFileNode* fc = mProjectRepo.findFileNode(gucEditor)) {
                ui->menuEncoding->setEnabled(false);
                ui->menureload_with->setEnabled(false);
                ui->menuconvert_to->setEnabled(false);
                mStatusWidgets->setFileName(fc->location());
                mStatusWidgets->setEncoding(fc->file()->codecMib());
                mStatusWidgets->setLineCount(-1);
                node->file()->reload();
                updateMenuToCodec(node->file()->codecMib());
            }
        }
        updateMenuToCodec(node->file()->codecMib());
    } else {
        ui->menuEncoding->setEnabled(false);
        mStatusWidgets->setFileName("");
        mStatusWidgets->setEncoding(-1);
        mStatusWidgets->setLineCount(-1);
    }

    loadCommandLines(oldTab, node);
    updateRunState();
    searchDialog()->updateReplaceActionAvailability();
    updateToolbar(mainTabs()->currentWidget());

    CodeEdit* ce = ViewHelper::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly()) ce->setOverwriteMode(mOverwriteMode);
    updateEditorMode();

    mNavigationHistory->setActiveTab(editWidget);
    updateCursorHistoryAvailability();
}

void MainWindow::fileChanged(const FileId fileId)
{
    mProjectRepo.fileChanged(fileId);
    FileMeta *fm = mFileMetaRepo.fileMeta(fileId);
    if (!fm) return;
    for (QWidget *edit: fm->editors()) {
        int index = ui->mainTabs->indexOf(edit);
        if (index >= 0) {
            if (fm) ui->mainTabs->setTabText(index, fm->name(NameModifier::editState));
        }
    }
}

void MainWindow::fileClosed(const FileId fileId)
{
    Q_UNUSED(fileId)
}

int MainWindow::fileChangedExtern(FileId fileId)
{
    FileMeta *file = mFileMetaRepo.fileMeta(fileId);
    // file has not been loaded: nothing to do
    if (!file->isOpen()) return 0;
    if (file->kind() == FileKind::Log) return 0;
    if (file->kind() == FileKind::Gdx) {
        QFile f(file->location());
        if (!f.exists()) return 4;
        bool resized = file->compare().testFlag(FileMeta::FdSize);
        file->refreshMetaData();
        for (QWidget *e : file->editors()) {
            if (gdxviewer::GdxViewer *gv = ViewHelper::toGdxViewer(e)) {
                gv->setHasChanged(true);
                int gdxErr = gv->reload(file->codec(), resized);
                if (gdxErr) return 4;
            }
        }
        return 0;
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
    return file->isModified() ? 2 : 1;
}

int MainWindow::fileDeletedExtern(FileId fileId)
{
    FileMeta *file = mFileMetaRepo.fileMeta(fileId);
    if (!file) return 0;
    if (file->exists(true)) return 0;
    mTextMarkRepo.removeMarks(fileId, QSet<TextMark::Type>() << TextMark::all);
    if (!file->isOpen()) {
        QVector<ProjectFileNode*> nodes = mProjectRepo.fileNodes(file->id());
        for (ProjectFileNode* node: nodes) {
            ProjectGroupNode *group = node->parentNode();
            mProjectRepo.closeNode(node);
            if (group->childCount() == 0)
                closeGroup(group);
        }
        mHistory.files().removeAll(file->location());
        historyChanged();
        return 0;
    }
    return 3;
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
    QMap<int, QVector<FileEventData>> remainEvents;
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
            QSet<FileEventData>::const_iterator it = scheduledEvents.find(fileEvent);
            if (it != scheduledEvents.constEnd() && it->time < fileEvent.time)
                scheduledEvents -= fileEvent;
            scheduledEvents += fileEvent;
            continue;
        }
        int remainKind = 0;
        switch (fileEvent.kind) {
        case FileEventKind::changedExtern:
            remainKind = fileChangedExtern(fm->id());
            break;
        case FileEventKind::removedExtern:
            remainKind = fileDeletedExtern(fm->id());
            break;
        default: break;
        }
        if (remainKind > 0) {
            if (!remainEvents.contains(remainKind)) remainEvents.insert(remainKind, QVector<FileEventData>());
            if (!remainEvents[remainKind].contains(fileEvent)) remainEvents[remainKind] << fileEvent;
        }
    }

    // Then ask what to do with the files of each remainKind
    for (auto key: remainEvents.keys()) {
        switch (key) {
        case 1: // changed externally but unmodified internally
        case 2: // changed externally and modified internally
            mFileEventHandler->process(FileEventHandler::Change, remainEvents.value(key));
            break;
        case 3: // removed externally
            mFileEventHandler->process(FileEventHandler::Deletion, remainEvents.value(key));
            break;
        case 4: // file is locked: reschedule event
            for (auto event: remainEvents.value(key))
                scheduledEvents << event;
            break;
        default:
            break;
        }
    }

    // add events that have been skipped due too early processing
    if (!scheduledEvents.isEmpty()) {
        QMutexLocker locker(&mFileMutex);
        for (FileEventData data : scheduledEvents) {
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
    ProjectRunGroupNode* groupNode = mProjectRepo.findRunGroup(origin);
    if (!groupNode) {
        appendSystemLogError("No group attached to process");
        return;
    }

    if (exitCode == ecTooManyScratchDirs) {
        ProjectRunGroupNode* node = mProjectRepo.findRunGroup(ViewHelper::groupId(mRecent.editor()));
        QString path = node ? QDir::toNativeSeparators(node->location()) : currentPath();

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
    groupNode->addNodesForSpecialFiles();

    FileMeta *runMeta = groupNode->runnableGms();
    if (!runMeta) {
        appendSystemLogError("Invalid runable attached to process");
        return;
    }
    if (groupNode && groupNode->hasLogNode()) {
        ProjectLogNode *logNode = groupNode->logNode();
        logNode->logDone();
        if (logNode->file()->editors().size()) {
            if (TextView* tv = ViewHelper::toTextView(logNode->file()->editors().first())) {
                if (Settings::settings()->toBool(skJumpToError)) {
                    int errLine = tv->firstErrorLine();
                    if (errLine >= 0) tv->jumpTo(errLine, 0);
                }
                MainWindow::disconnect(tv, &TextView::selectionChanged, this, &MainWindow::updateEditorPos);
                MainWindow::disconnect(tv, &TextView::blockCountChanged, this, &MainWindow::updateEditorBlockCount);
                MainWindow::disconnect(tv, &TextView::loadAmountChanged, this, &MainWindow::updateLoadAmount);
            }
        }
    }
    if (groupNode && runMeta->exists(true)) {
        QString lstFile = groupNode->parameter("ls2");
        mProjectRepo.findOrCreateFileNode(lstFile, groupNode);
        lstFile = groupNode->parameter("lst");
        bool doFocus = (groupNode == mRecent.group());
        ProjectFileNode* lstNode = mProjectRepo.findOrCreateFileNode(lstFile, groupNode);

        for (QWidget *edit: lstNode->file()->editors())
            if (TextView* tv = ViewHelper::toTextView(edit)) tv->endRun();

        bool alreadyJumped = false;
        if (Settings::settings()->toBool(skJumpToError))
            alreadyJumped = groupNode->jumpToFirstError(doFocus, lstNode);

        if (!alreadyJumped && Settings::settings()->toBool(skOpenLst))
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
    ProjectFileNode *node = mProjectRepo.findFile(mLibProcess->workingDirectory() + "/" + mLibProcess->inputFile());
    if (!node)
        node = addNode(mLibProcess->workingDirectory(), mLibProcess->inputFile());
    if (node) mFileMetaRepo.watch(node->file());
    if (node && !node->file()->editors().isEmpty()) {
        if (node->file()->kind() != FileKind::Log)
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

void MainWindow::on_actionHelp_triggered()
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
    } else {
         QWidget* editWidget = (ui->mainTabs->currentIndex() < 0 ? nullptr : ui->mainTabs->widget((ui->mainTabs->currentIndex())) );
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
                            } else if (iKind == static_cast<int>(syntax::SyntaxKind::Directive)) {
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
    about.setIconPixmap(Scheme::icon(":/img/gams-w24").pixmap(QSize(64, 64)));
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
    fm->setKind("log");
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
        args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
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
    if (fc->editors().size() == 1 && fc->isModified()) {
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
        return;
    }
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
    bool isResults = ui->logTabs->widget(index) == mSearchDialog->resultsView();
    if (isResults) {
        mSearchDialog->clearResults();
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

bool MainWindow::isActiveTabRunnable()
{
    QWidget* editWidget = (ui->mainTabs->currentIndex() < 0 ? nullptr : ui->mainTabs->widget((ui->mainTabs->currentIndex())) );
    if (editWidget) {
       FileMeta* fm = mFileMetaRepo.fileMeta(editWidget);
       if (!fm) { // assuming a welcome page here
           return false;
       } else {
           if (!mRecent.group()) return false;
           ProjectRunGroupNode *runGroup = mRecent.group()->assignedRunGroup();
           return runGroup && runGroup->runnableGms();
       }
    }
    return false;
}

bool MainWindow::isRecentGroupRunning()
{
    if (!mRecent.group()) return false;
    ProjectRunGroupNode *runGroup = mRecent.group()->assignedRunGroup();
    if (!runGroup) return false;
    return (runGroup->gamsProcessState() != QProcess::NotRunning);
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
    if (!QFileInfo(filePath).exists()) return;

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

bool MainWindow::terminateProcessesConditionally(QVector<ProjectRunGroupNode *> runGroups)
{
    if (runGroups.isEmpty()) return true;
    QVector<ProjectRunGroupNode *> runningGroups;
    QStringList runningNames;
    for (ProjectRunGroupNode* runGroup: runGroups) {
        if (runGroup->process() && runGroup->process()->state() != QProcess::NotRunning) {
            runningGroups << runGroup;
            runningNames << runGroup->name();
        }
    }
    if (runningGroups.isEmpty()) return true;
    QString title = runningNames.size() > 1 ? QString::number(runningNames.size())+" processes are running"
                                            : runningNames.first()+" is running";
    QString message = runningNames.size() > 1 ? "processes?\n" : "process?\n";
    while (runningNames.size() > 4) runningNames.removeLast();
    while (runningNames.size() < runningGroups.size()) runningNames << "...";
    message += runningNames.join("\n");
    int choice = QMessageBox::question(this, title,
                          "Do you want to stop the "+message,
                          "Stop", "Cancel");
    if (choice == 1) return false;
    for (ProjectRunGroupNode* runGroup: runningGroups) {
        runGroup->process()->terminate();
    }
    return true;
}

void MainWindow::updateAndSaveSettings()
{
    Settings *settings = Settings::settings();

    // TODO(JM) this can be replaced later, Qt 5.14. provides ::screen() getter
    QScreen *screen = window()->windowHandle()->screen();
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
    settings->setInt(skSearchScope, searchDialog()->selectedScope());

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

    historyChanged();

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

    mMaximizedBeforeFullScreen = settings->toBool(skWinMaximized);
    if (settings->toBool(skWinFullScreen)) {
        setWindowState(windowState() ^ Qt::WindowFullScreen);
    } else if (mMaximizedBeforeFullScreen) {
        setWindowState(windowState() ^ Qt::WindowMaximized);
    }
    ui->actionFull_Screen->setChecked(settings->toBool(skWinFullScreen));
    restoreState(settings->toByteArray(skWinState));

    // tool-/menubar
    setProjectViewVisibility(settings->toBool(skViewProject));
    setOutputViewVisibility(settings->toBool(skViewOutput));
    setExtendedEditorVisibility(settings->toBool(skViewOption));
    setHelpViewVisibility(settings->toBool(skViewHelp));
    setEncodingMIBs(settings->toString(skEncodingMib));

    // help
#ifdef QWEBENGINE
    QVariantList joHelp = settings->toList(skHelpBookmarks);
    QMap<QString, QString> bookmarkMap;
    for (QVariant joVal: joHelp) {
        if (!joVal.canConvert(QVariant::Map)) continue;
        QVariantMap entry = joVal.toMap();
        bookmarkMap.insert(entry.value("location").toString(), entry.value("name").toString());
    }
    helpWidget()->setBookmarkMap(bookmarkMap);
    double hZoom = settings->toDouble(skHelpZoomFactor);
    helpWidget()->setZoomFactor(hZoom > 0.0 ? hZoom : 1.0);
#endif

}

QString MainWindow::currentPath()
{
    if (ui->mainTabs->currentWidget() && ui->mainTabs->currentWidget() != mWp) {
        return mRecent.path();
    }
    return Settings::settings()->toString(skDefaultWorkspace);

}

void MainWindow::on_actionGAMS_Library_triggered()
{
    modeldialog::ModelDialog dialog(Settings::settings()->toString(skUserModelLibraryDir), this);
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
    miroProcess->setWorkingDirectory(mRecent.group()->toRunGroup()->location());
    miroProcess->setModelName(mRecent.group()->toRunGroup()->mainModelName());
    miroProcess->setMiroPath(miro::MiroCommon::path(Settings::settings()->toString(skMiroInstallPath)));
    miroProcess->setMiroMode(miro::MiroMode::Base);

    execute(mGamsParameterEditor->getCurrentCommandLineData(), std::move(miroProcess));
}

void MainWindow::on_actionHypercube_mode_triggered()
{
    if (!validMiroPrerequisites())
        return;

    auto miroProcess = std::make_unique<miro::MiroProcess>(new miro::MiroProcess);
    miroProcess->setSkipModelExecution(ui->actionSkip_model_execution->isChecked());
    miroProcess->setWorkingDirectory(mRecent.group()->toRunGroup()->location());
    miroProcess->setModelName(mRecent.group()->toRunGroup()->mainModelName());
    miroProcess->setMiroPath(miro::MiroCommon::path(Settings::settings()->toString(skMiroInstallPath)));
    miroProcess->setMiroMode(miro::MiroMode::Hypercube);

    execute(mGamsParameterEditor->getCurrentCommandLineData(), std::move(miroProcess));
}

void MainWindow::on_actionConfiguration_mode_triggered()
{
    if (!validMiroPrerequisites())
        return;

    auto miroProcess = std::make_unique<miro::MiroProcess>(new miro::MiroProcess);
    miroProcess->setSkipModelExecution(ui->actionSkip_model_execution->isChecked());
    miroProcess->setWorkingDirectory(mRecent.group()->toRunGroup()->location());
    miroProcess->setModelName(mRecent.group()->toRunGroup()->mainModelName());
    miroProcess->setMiroPath(miro::MiroCommon::path(Settings::settings()->toString(skMiroInstallPath)));
    miroProcess->setMiroMode(miro::MiroMode::Configuration);

    execute(mGamsParameterEditor->getCurrentCommandLineData(), std::move(miroProcess));
}

void MainWindow::on_actionStop_MIRO_triggered()
{
    if (!mRecent.hasValidRunGroup())
        return;
    mRecent.group()->toRunGroup()->process()->terminate();
}

void MainWindow::on_actionCreate_model_assembly_triggered()
{
    openMiroAssemblyDialog(this);
}

void MainWindow::on_actionDeploy_triggered()
{
    if (!validMiroPrerequisites())
        return;

    QString assemblyFile = mRecent.group()->toRunGroup()->location() + "/" +
                           miro::MiroCommon::assemblyFileName(mRecent.group()->toRunGroup()->mainModelName());

    QStringList checkedFiles;
    if (mRecent.hasValidRunGroup()) {
        checkedFiles = miro::MiroCommon::unifiedAssemblyFileContent(assemblyFile,
                                                                mRecent.group()->toRunGroup()->mainModelName(false));
    }

    mMiroDeployDialog->setDefaults();
    mMiroDeployDialog->setAssemblyFileName(assemblyFile);
    mMiroDeployDialog->setWorkingDirectory(mRecent.group()->toRunGroup()->location());
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
        addToGroup(mRecent.group(), mMiroDeployDialog->assemblyFileName());
    }
}

void MainWindow::on_menuMIRO_aboutToShow()
{
    ui->menuMIRO->setEnabled(isMiroAvailable());
}

void MainWindow::miroAssemblyDialogFinish(int result)
{
    if (result != QDialog::Accepted)
        return;

    if (!miro::MiroCommon::writeAssemblyFile(mMiroAssemblyDialog->assemblyFileName(),
                                             mMiroAssemblyDialog->selectedFiles()))
        SysLogLocator::systemLog()->append(QString("Could not write model assembly file: %1")
                                           .arg(mMiroAssemblyDialog->assemblyFileName()),
                                           LogMsgType::Error);
    else
        addToGroup(mRecent.group(), mMiroAssemblyDialog->assemblyFileName());
}

void MainWindow::openMiroAssemblyDialog(QWidget *parent)
{
    QString location;
    QString assemblyFile;
    QStringList checkedFiles;
    if (mRecent.hasValidRunGroup()) {
        location = mRecent.group()->toRunGroup()->location();
        assemblyFile = miro::MiroCommon::assemblyFileName(mRecent.group()->toRunGroup()->location(),
                                                          mRecent.group()->toRunGroup()->mainModelName());
        checkedFiles = miro::MiroCommon::unifiedAssemblyFileContent(assemblyFile,
                                                                    mRecent.group()->toRunGroup()->mainModelName(false));
    }

    mMiroAssemblyDialog->setParent(parent);
    mMiroAssemblyDialog->setAssemblyFileName(assemblyFile);
    mMiroAssemblyDialog->setWorkingDirectory(location);
    mMiroAssemblyDialog->setSelectedFiles(checkedFiles);
    mMiroAssemblyDialog->open();
}

void MainWindow::miroDeploy(bool testDeploy, miro::MiroDeployMode mode)
{
    if (!mRecent.hasValidRunGroup())
        return;

    auto process = std::make_unique<miro::MiroDeployProcess>(new miro::MiroDeployProcess);
    process->setMiroPath(miro::MiroCommon::path( Settings::settings()->toString(skMiroInstallPath)));
    process->setWorkingDirectory(mRecent.group()->toRunGroup()->location());
    process->setModelName(mRecent.group()->toRunGroup()->mainModelName());
    process->setTestDeployment(testDeploy);
    process->setTargetEnvironment(mMiroDeployDialog->targetEnvironment());

    if (testDeploy) {
        switch(mode){
        case miro::MiroDeployMode::Base:
            process->setBaseMode(mMiroDeployDialog->baseMode());
            break;
        case miro::MiroDeployMode::Hypercube:
            process->setTargetEnvironment(miro::MiroTargetEnvironment::MultiUser);
            process->setHypercubeMode(mMiroDeployDialog->hypercubeMode());
            break;
        default:
            break;
        }
    } else {
        process->setBaseMode(mMiroDeployDialog->baseMode());
        process->setHypercubeMode(mMiroDeployDialog->hypercubeMode());
    }

    execute(mGamsParameterEditor->getCurrentCommandLineData(), std::move(process));
}

void MainWindow::setMiroRunning(bool running)
{
    mMiroRunning = running;
    updateMiroEnabled();
}

void MainWindow::updateMiroEnabled()
{
    bool available = isMiroAvailable() && isActiveTabRunnable();
    ui->menuMIRO->setEnabled(available);
    mMiroDeployDialog->setEnabled(available && !mMiroRunning);
    ui->actionBase_mode->setEnabled(available && !mMiroRunning);
    ui->actionHypercube_mode->setEnabled(available && !mMiroRunning);
    ui->actionConfiguration_mode->setEnabled(available && !mMiroRunning);
    ui->actionSkip_model_execution->setEnabled(available && mMiroRunning);
    ui->actionCreate_model_assembly->setEnabled(available && !mMiroRunning);
    ui->actionDeploy->setEnabled(available && !mMiroRunning);
}

void MainWindow::on_projectView_activated(const QModelIndex &index)
{
    ProjectAbstractNode* node = mProjectRepo.node(index);
    if (!node) return;
    if ((node->type() == NodeType::group) || (node->type() == NodeType::runGroup)) {
        ProjectRunGroupNode *runGroup = node->assignedRunGroup();
        if (runGroup && runGroup->runnableGms()) {
            ProjectLogNode* logNode = runGroup->logNode();
            openFileNode(logNode, true, logNode->file()->codecMib());
            ProjectAbstractNode *latestNode = mProjectRepo.node(mProjectRepo.treeModel()->current());
            if (!latestNode || latestNode->assignedRunGroup() != runGroup) {
                openFile(runGroup->runnableGms(), true, runGroup, runGroup->runnableGms()->codecMib());
            }
        }
    } else {
        ProjectFileNode *file = mProjectRepo.asFileNode(index);
        if (file) openFileNode(file);
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

    ProjectFileNode* fc = mProjectRepo.findFileNode(mRecent.editor());
    ProjectRunGroupNode *runGroup = (fc ? fc->assignedRunGroup() : nullptr);
    if (runGroup) runGroup->addRunParametersHistory(mGamsParameterEditor->getCurrentCommandLineData());

    updateAndSaveSettings();
    QVector<FileMeta*> oFiles = mFileMetaRepo.modifiedFiles();
    if (!terminateProcessesConditionally(mProjectRepo.runGroups())) {
        event->setAccepted(false);
    } else if (requestCloseChanged(oFiles)) {
        on_actionClose_All_triggered();
        closeHelpView();
        mTextMarkRepo.clear();
    } else {
        event->setAccepted(false);
    }
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
    if ((e->modifiers() & Qt::ControlModifier) && (e->key() == Qt::Key_0))
        updateFixedFonts(Settings::settings()->toString(skEdFontFamily),
                         Settings::settings()->toInt(skEdFontSize));

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
            setOutputViewVisibility(false);
            e->accept(); return;
        } else if (focusWidget() == ui->logTabs->currentWidget()) {
            on_logTabs_tabCloseRequested(ui->logTabs->currentIndex());
            ui->logTabs->currentWidget()->setFocus();
            e->accept(); return;
        } else if (focusWidget() == ui->projectView) {
            setProjectViewVisibility(false);
        } else if (mGamsParameterEditor->isAParameterEditorFocused(focusWidget())) {
            mGamsParameterEditor->deSelectParameters();
        } else if (mRecent.editor() != nullptr) {
            if (ViewHelper::toSolverOptionEdit(mRecent.editor())) {
                ViewHelper::toSolverOptionEdit(mRecent.editor())->deSelectOptions();
            } else if (ViewHelper::toGamsConfigEditor(mRecent.editor())) {
                       ViewHelper::toGamsConfigEditor(mRecent.editor())->deSelectAll();
            }
        }

        // search widget
        if (mSearchDialog->isHidden()) mSearchDialog->clearSearch();
        else mSearchDialog->hide();

        e->accept(); return;
    }

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
    if (e->mimeData()->hasUrls() && FileMeta::hasExistingFile(e->mimeData()->urls())) {
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
                       " files at once. Depending on the file sizes this may take a long time.");
        msgBox.setInformativeText("Do you want to continue?");
        msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Cancel);
        answer = msgBox.exec();

        if (answer != QMessageBox::Open) return;
    }
    openFiles(pathList);
}

bool MainWindow::eventFilter(QObject* sender, QEvent* event)
{
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

void MainWindow::openFiles(QStringList files, bool forceNew)
{
    if (files.size() == 0) return;

    if (!forceNew && files.size() == 1) {
        FileMeta *file = mFileMetaRepo.fileMeta(files.first());
        if (file) {
            openFile(file);
            return;
        }
    }

    QStringList filesNotFound;
    QList<ProjectFileNode*> gmsFiles;
    QFileInfo firstFile(files.first());

    // create base group
    ProjectGroupNode *group = mProjectRepo.createGroup(firstFile.completeBaseName(), firstFile.absolutePath(), "");
    for (QString item: files) {
        if (QFileInfo(item).exists()) {
            ProjectFileNode *node = addNode("", item, group);
            openFileNode(node);
            if (node->file()->kind() == FileKind::Gms) gmsFiles << node;
            QApplication::processEvents(QEventLoop::AllEvents, 1);
        } else {
            filesNotFound.append(item);
        }
    }
    // find runnable gms, for now take first one found
    QString mainGms;
    if (gmsFiles.size() > 0) {
        ProjectRunGroupNode *prgn = group->toRunGroup();
        if (prgn) prgn->setParameter("gms", gmsFiles.first()->location());
    }

    if (!filesNotFound.empty()) {
        QString msgText("The following files could not be opened:");
        for(QString s : filesNotFound)
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

option::ParameterEditor *MainWindow::gamsParameterEditor() const
{
    return mGamsParameterEditor;
}

void MainWindow::execute(QString commandLineStr, std::unique_ptr<AbstractProcess> process, ProjectFileNode* gmsFileNode)
{
    ProjectFileNode* fileNode = (gmsFileNode ? gmsFileNode : mProjectRepo.findFileNode(mRecent.editor()));
    ProjectRunGroupNode* runGroup = (fileNode ? fileNode->assignedRunGroup() : nullptr);
    if (!runGroup) {
        DEB() << "Nothing to be executed.";
        return;
    }
    bool ready = executePrepare(fileNode, runGroup, commandLineStr, std::move(process), gmsFileNode);
    if (ready) execution(runGroup);
}


bool MainWindow::executePrepare(ProjectFileNode* fileNode, ProjectRunGroupNode* runGroup, QString commandLineStr,
                                std::unique_ptr<AbstractProcess> process, ProjectFileNode* gmsFileNode)
{
    Settings *settings = Settings::settings();
    runGroup->addRunParametersHistory( mGamsParameterEditor->getCurrentCommandLineData() );
    runGroup->clearErrorTexts();

    // gather modified files and autosave or request to save
    QVector<FileMeta*> modifiedFiles;
    for (ProjectFileNode *node: runGroup->listFiles(true)) {
        if (node->file()->isOpen() && !modifiedFiles.contains(node->file()) && node->file()->isModified())
            modifiedFiles << node->file();
    }
    bool doSave = !modifiedFiles.isEmpty();
    if (doSave && !settings->toBool(skAutosaveOnRun)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        if (modifiedFiles.size() > 1)
            msgBox.setText(QDir::toNativeSeparators(modifiedFiles.first()->location())+" has been modified.");
        else
            msgBox.setText(QString::number(modifiedFiles.size())+" files have been modified.");
        msgBox.setInformativeText("Do you want to save your changes before running?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        QAbstractButton* discardButton = msgBox.addButton(tr("Discard Changes and Run"), QMessageBox::ResetRole);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Cancel) {
            return false;
        } else if (msgBox.clickedButton() == discardButton) {
            for (FileMeta *file: modifiedFiles)
                if (file->kind() != FileKind::Log) {
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
        for (FileMeta *file: modifiedFiles) file->save();
    }

    // clear the TextMarks for this group
    QSet<TextMark::Type> markTypes;
    markTypes << TextMark::error << TextMark::link << TextMark::target;
    for (ProjectFileNode *node: runGroup->listFiles(true))
        mTextMarkRepo.removeMarks(node->file()->id(), node->assignedRunGroup()->id(), markTypes);

    // prepare the log
    ProjectLogNode* logNode = mProjectRepo.logNode(runGroup);
    markTypes << TextMark::bookmark;
    mTextMarkRepo.removeMarks(logNode->file()->id(), logNode->assignedRunGroup()->id(), markTypes);

    logNode->resetLst();
    if (!logNode->file()->isOpen()) {
        QWidget *wid = logNode->file()->createEdit(ui->logTabs, logNode->assignedRunGroup(), logNode->file()->codecMib());
        wid->setFont(createEditorFont(settings->toString(skEdFontFamily), settings->toInt(skEdFontSize)));
        if (ViewHelper::toTextView(wid))
            ViewHelper::toTextView(wid)->setLineWrapMode(settings->toBool(skEdLineWrapProcess) ? AbstractEdit::WidgetWidth
                                                                                               : AbstractEdit::NoWrap);
    }
    if (TextView* tv = ViewHelper::toTextView(logNode->file()->editors().first())) {
        MainWindow::connect(tv, &TextView::selectionChanged, this, &MainWindow::updateEditorPos, Qt::UniqueConnection);
        MainWindow::connect(tv, &TextView::blockCountChanged, this, &MainWindow::updateEditorBlockCount, Qt::UniqueConnection);
        MainWindow::connect(tv, &TextView::loadAmountChanged, this, &MainWindow::updateLoadAmount, Qt::UniqueConnection);
    }
    // cleanup bookmarks
    QVector<QString> cleanupKinds;
    cleanupKinds << "gdx" << "gsp" << "log" << "lst" << "ls2" << "lxi" << "ref";
    markTypes = QSet<TextMark::Type>() << TextMark::bookmark;
    for (const QString &kind: cleanupKinds) {
        if (runGroup->hasParameter(kind)) {
            FileMeta *file = mFileMetaRepo.fileMeta(runGroup->parameter(kind));
            if (file) mTextMarkRepo.removeMarks(file->id(), markTypes);
        }
    }

    if (settings->toBool(skEdClearLog)) logNode->clearLog();

    if (!ui->logTabs->children().contains(logNode->file()->editors().first())) {
        ui->logTabs->addTab(logNode->file()->editors().first(), logNode->name(NameModifier::editState));
    }
    ui->logTabs->setCurrentWidget(logNode->file()->editors().first());
    ui->dockProcessLog->setVisible(true);

    // select gms-file and working dir to run
    QString gmsFilePath = (gmsFileNode ? gmsFileNode->location() : runGroup->parameter("gms"));
    if (gmsFilePath == "") {
        appendSystemLogWarning("No runnable GMS file found in group ["+runGroup->name()+"].");
        ui->actionShow_System_Log->trigger();
        return false;
    }
    if (gmsFileNode)
        logNode->file()->setCodecMib(fileNode->file()->codecMib());
    else {
        FileMeta *runMeta = mFileMetaRepo.fileMeta(gmsFilePath);
        ProjectFileNode *runNode = runGroup->findFile(runMeta);
        logNode->file()->setCodecMib(runNode ? runNode->file()->codecMib() : -1);
    }
    QString workDir = gmsFileNode ? QFileInfo(gmsFilePath).path() : runGroup->location();

    // prepare the options and process and run it
    QList<option::OptionItem> itemList = mGamsParameterEditor->getOptionTokenizer()->tokenize(commandLineStr);
    if (process)
        runGroup->setProcess(std::move(process));
    AbstractProcess* groupProc = runGroup->process();
    groupProc->setParameters(runGroup->analyzeParameters(gmsFilePath, groupProc->defaultParameters(), itemList));

    logNode->prepareRun();
    logNode->setJumpToLogEnd(true);

    groupProc->setGroupId(runGroup->id());
    groupProc->setWorkingDirectory(workDir);

    // disable MIRO menus
    if (dynamic_cast<miro::AbstractMiroProcess*>(groupProc) ) {
        setMiroRunning(true);
        connect(groupProc, &AbstractProcess::finished, [this](){setMiroRunning(false);});
    }
    connect(groupProc, &AbstractProcess::newProcessCall, this, &MainWindow::newProcessCall);
    connect(groupProc, &AbstractProcess::finished, this, &MainWindow::postGamsRun, Qt::UniqueConnection);

    logNode->linkToProcess(groupProc);
    return true;
}

void MainWindow::execution(ProjectRunGroupNode *runGroup)
{
    AbstractProcess* groupProc = runGroup->process();
    groupProc->execute();
    ui->toolBar->repaint();

    ui->dockProcessLog->raise();
}

void MainWindow::updateRunState()
{
    updateMiroEnabled();
    mGamsParameterEditor->updateRunState(isActiveTabRunnable(), isRecentGroupRunning());
}

#ifdef QWEBENGINE
help::HelpWidget *MainWindow::helpWidget() const
{
    return mHelpWidget;
}
#endif

void MainWindow::setMainGms(ProjectFileNode *node)
{
    ProjectRunGroupNode *runGroup = node->assignedRunGroup();
    if (runGroup) {
        runGroup->setRunnableGms(node->file());
        updateRunState();
    }
}

void MainWindow::parameterRunChanged()
{
    if (isActiveTabRunnable() && !isRecentGroupRunning())
        mGamsParameterEditor->runDefaultAction();
}

void MainWindow::openInitialFiles()
{
    Settings *settings = Settings::settings();
    projectRepo()->read(settings->toList(skProjects));

    if (settings->toBool(skRestoreTabs)) {
        QVariantMap joTabs = settings->toMap(skTabs);
        if (!readTabs(joTabs)) return;
    }
    mHistory.files().clear();
    QVariantList joHistory = settings->toList(skHistory);
    for (QVariant jRef: joHistory) {
        if (!jRef.canConvert(QVariant::Map)) continue;
        QVariantMap map = jRef.toMap();
        if (map.contains("file"))
            mHistory.files() << map.value("file").toString();
    }

    openFiles(mInitialFiles, false);
    mInitialFiles.clear();
    watchProjectTree();
    ProjectFileNode *node = mProjectRepo.findFileNode(ui->mainTabs->currentWidget());
    if (node) openFileNode(node, true);
    historyChanged();
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

void MainWindow::showNeosStartDialog()
{
    neos::NeosStartDialog *dialog = new neos::NeosStartDialog(this);
    neos::NeosProcess *neosPtr = createNeosProcess();
    if (!neosPtr) return;
    dialog->setProcess(neosPtr);
    connect(dialog, &neos::NeosStartDialog::rejected, dialog, &neos::NeosStartDialog::deleteLater);
    connect(dialog, &neos::NeosStartDialog::accepted, dialog, &neos::NeosStartDialog::deleteLater);
    connect(dialog, &neos::NeosStartDialog::accepted, this, &MainWindow::prepareNeosProcess);
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
    ProjectFileNode* fileNode = mProjectRepo.findFileNode(mRecent.editor());
    ProjectRunGroupNode* runGroup = (fileNode ? fileNode->assignedRunGroup() : nullptr);
    if (!runGroup) return nullptr;
    auto neosProcess = std::make_unique<neos::NeosProcess>(new neos::NeosProcess());
    neosProcess->setWorkingDirectory(mRecent.group()->toRunGroup()->location());
    mGamsParameterEditor->on_runAction(option::RunActionState::RunNeos);
    runGroup->setProcess(std::move(neosProcess));
    neos::NeosProcess *neosPtr = static_cast<neos::NeosProcess*>(runGroup->process());
    connect(neosPtr, &neos::NeosProcess::procStateChanged, this, &MainWindow::remoteProgress);
    return neosPtr;
}

void MainWindow::prepareNeosProcess()
{
    ProjectFileNode* fileNode = mProjectRepo.findFileNode(mRecent.editor());
    ProjectRunGroupNode* runGroup = (fileNode ? fileNode->assignedRunGroup() : nullptr);
    if (!runGroup) return;
    updateAndSaveSettings();
    neos::NeosProcess *neosPtr = static_cast<neos::NeosProcess*>(runGroup->process());
    neosPtr->setStarting();
    if (!executePrepare(fileNode, runGroup, mGamsParameterEditor->getCurrentCommandLineData()))
        return;
    if (!mIgnoreSslErrors) {
        connect(neosPtr, &neos::NeosProcess::sslValidation, this, &MainWindow::sslValidation);
        neosPtr->validate();
    } else {
        updateAndSaveSettings();
        execution(runGroup);
    }
}

void MainWindow::sslValidation(QString errorMessage)
{
    if (mIgnoreSslErrors || errorMessage.isEmpty()) {
        updateAndSaveSettings();
        ProjectFileNode* fileNode = mProjectRepo.findFileNode(mRecent.editor());
        ProjectRunGroupNode *runGroup = (fileNode ? fileNode->assignedRunGroup() : nullptr);
        if (!runGroup) return;
        execution(runGroup);
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
    ProjectFileNode* fileNode = mProjectRepo.findFileNode(mRecent.editor());
    ProjectRunGroupNode *runGroup = (fileNode ? fileNode->assignedRunGroup() : nullptr);
    if (!runGroup) return;
    QMessageBox *msgBox = qobject_cast<QMessageBox*>(sender());
    if (msgBox && msgBox->standardButton(button) == QMessageBox::Ignore) {
        if (neos::NeosProcess *neosProc = static_cast<neos::NeosProcess*>(runGroup->process())) {
            neosProc->setIgnoreSslErrors();
            mIgnoreSslErrors = true;
            updateAndSaveSettings();
            execution(runGroup);
        }
    } else {
        runGroup->setProcess(nullptr);
    }
}

void MainWindow::showEngineStartDialog()
{
    engine::EngineStartDialog *dialog = new engine::EngineStartDialog(this);
    dialog->setLastPassword(mEngineTempPassword);
    dialog->setProcess(createEngineProcess());
    connect(dialog, &engine::EngineStartDialog::ready, this, &MainWindow::engineDialogDecision);
    dialog->setModal(true);

    if (mEngineNoDialog && !qApp->keyboardModifiers().testFlag(Qt::ControlModifier)) {
        dialog->hiddenCheck();
    } else {
        dialog->open();
        dialog->focusEmptyField();
    }
}

void MainWindow::engineDialogDecision(bool start, bool always)
{
    engine::EngineStartDialog *dialog = qobject_cast<engine::EngineStartDialog*>(sender());
    if (!dialog) return;
    if (start) {
        Settings::settings()->setString(SettingsKey::skEngineUrl, dialog->url());
        Settings::settings()->setString(SettingsKey::skEngineNamespace, dialog->nSpace());
        Settings::settings()->setString(SettingsKey::skEngineUser, dialog->user());
        Settings::settings()->setBool(SettingsKey::skEngineForceGdx, dialog->forceGdx());
        mEngineTempPassword = dialog->password();
        mEngineNoDialog = always;
        prepareEngineProcess(dialog->url(), dialog->nSpace(), dialog->user(), dialog->password());
    } else {
        dialog->close();
    }
    dialog->deleteLater();
}

engine::EngineProcess *MainWindow::createEngineProcess()
{
    updateAndSaveSettings();
    ProjectFileNode* fc = mProjectRepo.findFileNode(mRecent.editor());
    ProjectRunGroupNode *runGroup = (fc ? fc->assignedRunGroup() : nullptr);
    if (!runGroup) {
        DEB() << "Could not create GAMS Engine process";
        return nullptr;
    }
    auto engineProcess = std::make_unique<engine::EngineProcess>(new engine::EngineProcess());
    connect(engineProcess.get(), &engine::EngineProcess::procStateChanged, this, &MainWindow::remoteProgress);
    engineProcess->setWorkingDirectory(mRecent.group()->toRunGroup()->location());
    QString commandLineStr = mGamsParameterEditor->getCurrentCommandLineData();
    QList<option::OptionItem> itemList = mGamsParameterEditor->getOptionTokenizer()->tokenize(commandLineStr);
    for (const option::OptionItem &item : itemList) {
        if (item.key.compare("previousWork", Qt::CaseInsensitive) == 0)
            engineProcess->setHasPreviousWorkOption(true);
    }
    runGroup->setProcess(std::move(engineProcess));
    return qobject_cast<engine::EngineProcess*>(runGroup->process());
}

void MainWindow::prepareEngineProcess(QString url, QString nSpace, QString user, QString password)
{
    ProjectFileNode* node = mProjectRepo.findFileNode(mRecent.editor());
    ProjectRunGroupNode *group = (node ? node->assignedRunGroup() : nullptr);
    if (!group) return;
    AbstractProcess* process = group->process();
    engine::EngineProcess *engineProcess = qobject_cast<engine::EngineProcess*>(process);
    if (!engineProcess) return;
    mGamsParameterEditor->on_runAction(option::RunActionState::RunEngine);
    engineProcess->setUrl(url);
    engineProcess->setNamespace(nSpace);
    engineProcess->authenticate(user, password);
    // TODO(JM) create token for the user and store it (if the user allowed it)
//    if (!mIgnoreSslErrors) {
//        engine::EngineProcess *enginePtr = static_cast<engine::EngineProcess*>(runGroup->process());
//        connect(enginePtr, &engine::EngineProcess::sslValidation, this, &MainWindow::sslValidation);
//    } else {
//    }
    updateAndSaveSettings();
    executePrepare(node, group, mGamsParameterEditor->getCurrentCommandLineData());
    execution(group);
}

void MainWindow::on_actionInterrupt_triggered()
{
    ProjectFileNode* node = mProjectRepo.findFileNode(mRecent.editor());
    ProjectRunGroupNode *group = (node ? node->assignedRunGroup() : nullptr);
    if (!group) return;
    mGamsParameterEditor->on_interruptAction();
    AbstractProcess* process = group->process();
    QtConcurrent::run(process, &AbstractProcess::interrupt);
}

void MainWindow::on_actionStop_triggered()
{
    ProjectFileNode* node = mProjectRepo.findFileNode(mRecent.editor());
    ProjectRunGroupNode *group = (node ? node->assignedRunGroup() : nullptr);
    if (!group) return;
    mGamsParameterEditor->on_stopAction();
    AbstractProcess* process = group->process();
    QtConcurrent::run(process, &GamsProcess::terminate);
}

void MainWindow::changeToLog(ProjectAbstractNode *node, bool openOutput, bool createMissing)
{
    Settings *settings = Settings::settings();
    bool moveToEnd = false;
    ProjectLogNode* logNode = mProjectRepo.logNode(node);
    if (!logNode) return;

    if (createMissing) {
        moveToEnd = true;
        if (!logNode->file()->isOpen()) {
            QWidget *wid = logNode->file()->createEdit(ui->logTabs, logNode->assignedRunGroup(), logNode->file()->codecMib());
            wid->setFont(createEditorFont(settings->toString(skEdFontFamily), settings->toInt(skEdFontSize)));
            if (TextView * tv = ViewHelper::toTextView(wid))
                tv->setLineWrapMode(settings->toBool(skEdLineWrapProcess) ? AbstractEdit::WidgetWidth
                                                                          : AbstractEdit::NoWrap);
        }
        if (TextView* tv = ViewHelper::toTextView(logNode->file()->editors().first())) {
            MainWindow::connect(tv, &TextView::selectionChanged, this, &MainWindow::updateEditorPos, Qt::UniqueConnection);
            MainWindow::connect(tv, &TextView::blockCountChanged, this, &MainWindow::updateEditorBlockCount, Qt::UniqueConnection);
            MainWindow::connect(tv, &TextView::loadAmountChanged, this, &MainWindow::updateLoadAmount, Qt::UniqueConnection);
        }
    }
    if (logNode->file()->isOpen()) {
        if (TextView* logEdit = ViewHelper::toTextView(logNode->file()->editors().first())) {
            if (openOutput) setOutputViewVisibility(true);
            if (ui->logTabs->currentWidget() != logEdit) {
                if (ui->logTabs->currentWidget() != searchDialog()->resultsView())
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

void MainWindow::invalidateScheme()
{
    for (FileMeta *fm: mFileMetaRepo.fileMetas())
        fm->invalidateScheme();

    repaint();
}

void MainWindow::ensureInScreen()
{
    QRect screenGeo = QGuiApplication::primaryScreen()->virtualGeometry();
    QRect appGeo = geometry();
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

void MainWindow::openFile(FileMeta* fileMeta, bool focus, ProjectRunGroupNode *runGroup, int codecMib,
                          bool forcedAsTextEditor, NewTabStrategy tabStrategy)
{
    Settings *settings = Settings::settings();
    if (!fileMeta) return;
    QWidget* edit = nullptr;
    QTabWidget* tabWidget = fileMeta->kind() == FileKind::Log ? ui->logTabs : ui->mainTabs;
    if (!fileMeta->editors().empty()) {
        edit = fileMeta->editors().first();
    }

    // open edit if existing or create one
    if (edit) {
        if (runGroup) ViewHelper::setGroupId(edit, runGroup->id());
        else {
            NodeId groupId = ViewHelper::groupId(edit);
            if (groupId.isValid()) runGroup = mProjectRepo.findRunGroup(groupId);
        }
        if (focus) {
            tabWidget->setCurrentWidget(edit);
            raiseEdit(edit);
            if (tabWidget == ui->mainTabs) {
                activeTabChanged(tabWidget->indexOf(edit));
            }
        }
    } else {
        if (!runGroup) {
            QVector<ProjectFileNode*> nodes = mProjectRepo.fileNodes(fileMeta->id());
            if (nodes.size()) {
                if (nodes.first()->assignedRunGroup())
                    runGroup = nodes.first()->assignedRunGroup();
            } else {
                QFileInfo file(fileMeta->location());
                runGroup = mProjectRepo.createGroup(file.completeBaseName(), file.absolutePath(), file.absoluteFilePath())->toRunGroup();
                nodes.append(mProjectRepo.findOrCreateFileNode(file.absoluteFilePath(), runGroup));
            }
        }
        try {
            if (codecMib == -1) codecMib = fileMeta->codecMib();
            edit = fileMeta->createEdit(tabWidget, runGroup, codecMib, forcedAsTextEditor, tabStrategy);
        } catch (Exception &e) {
            appendSystemLogError(e.what());
            return;
        }
        if (!edit) {
            DEB() << "Error: could not create editor for '" << fileMeta->location() << "'";
            return;
        }
        if (ViewHelper::toCodeEdit(edit)) {
            CodeEdit* ce = ViewHelper::toCodeEdit(edit);
            connect(ce, &CodeEdit::requestAdvancedActions, this, &MainWindow::getAdvancedActions);
            connect(ce, &CodeEdit::cloneBookmarkMenu, this, &MainWindow::cloneBookmarkMenu);
            connect(ce, &CodeEdit::searchFindNextPressed, mSearchDialog, &search::SearchDialog::on_searchNext);
            connect(ce, &CodeEdit::searchFindPrevPressed, mSearchDialog, &search::SearchDialog::on_searchPrev);
        }
        if (TextView *tv = ViewHelper::toTextView(edit)) {
            tv->setFont(createEditorFont(settings->toString(skEdFontFamily), settings->toInt(skEdFontSize)));
            connect(tv, &TextView::searchFindNextPressed, mSearchDialog, &search::SearchDialog::on_searchNext);
            connect(tv, &TextView::searchFindPrevPressed, mSearchDialog, &search::SearchDialog::on_searchPrev);

        }
        if (ViewHelper::toCodeEdit(edit)) {
            AbstractEdit *ae = ViewHelper::toAbstractEdit(edit);
            ae->setFont(createEditorFont(settings->toString(skEdFontFamily), settings->toInt(skEdFontSize)));
            if (!ae->isReadOnly())
                connect(fileMeta, &FileMeta::changed, this, &MainWindow::fileChanged, Qt::UniqueConnection);
        } else if (ViewHelper::toSolverOptionEdit(edit)) {
                   connect(fileMeta, &FileMeta::changed, this, &MainWindow::fileChanged, Qt::UniqueConnection);
        } else if (ViewHelper::toGamsConfigEditor(edit)) {
                  connect(fileMeta, &FileMeta::changed, this, &MainWindow::fileChanged, Qt::UniqueConnection);
        }
        if (focus) {
            tabWidget->setCurrentWidget(edit);
            raiseEdit(edit);
            updateMenuToCodec(fileMeta->codecMib());
        }
        if (fileMeta->kind() == FileKind::Ref) {
            reference::ReferenceViewer *refView = ViewHelper::toReferenceViewer(edit);
            connect(refView, &reference::ReferenceViewer::jumpTo, this, &MainWindow::on_referenceJumpTo);
        }

    }
    // set keyboard focus to editor
    if (tabWidget->currentWidget())
        if (focus) tabWidget->currentWidget()->setFocus();

    if (tabWidget != ui->logTabs) {
        // if there is already a log -> show it
        ProjectFileNode* fileNode = mProjectRepo.findFileNode(edit);
        changeToLog(fileNode, false, false);
        mRecent.setEditor(tabWidget->currentWidget(), this);
    }
    addToOpenedFiles(fileMeta->location());
}

void MainWindow::openFileNode(ProjectFileNode *node, bool focus, int codecMib, bool forcedAsTextEditor, NewTabStrategy tabStrategy)
{
    if (!node) return;
    openFile(node->file(), focus, node->assignedRunGroup(), codecMib, forcedAsTextEditor, tabStrategy);
}

void MainWindow::reOpenFileNode(ProjectFileNode *node, bool focus, int codecMib, bool forcedAsTextEditor)
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

void MainWindow::closeGroup(ProjectGroupNode* group)
{
    if (!group) return;
    ProjectGroupNode *parentGroup = group->parentNode();
    if (parentGroup && parentGroup->type() == NodeType::root) parentGroup = nullptr;
    ProjectRunGroupNode *runGroup = group->assignedRunGroup();
    if (!terminateProcessesConditionally(QVector<ProjectRunGroupNode*>() << runGroup))
        return;
    QVector<FileMeta*> changedFiles;
    QVector<FileMeta*> openFiles;
    for (ProjectFileNode *node: group->listFiles(true)) {
        if (node->isModified()) changedFiles << node->file();
        if (node->file()->isOpen()) openFiles << node->file();
    }

    if (requestCloseChanged(changedFiles)) {
        for (FileMeta *file: openFiles) {
            closeFileEditors(file->id());
        }
        ProjectLogNode* log = (runGroup && runGroup->hasLogNode()) ? runGroup->logNode() : nullptr;
        if (log) {
            QWidget* edit = log->file()->editors().isEmpty() ? nullptr : log->file()->editors().first();
            if (edit) {
                log->file()->removeEditor(edit);
                int index = ui->logTabs->indexOf(edit);
                if (index >= 0) ui->logTabs->removeTab(index);
            }
        }
        mProjectRepo.closeGroup(group);
    }
    mProjectRepo.purgeGroup(parentGroup);
}

void MainWindow::neosProgress(AbstractProcess *proc, ProcState progress)
{
    ProjectRunGroupNode *runGroup = mProjectRepo.asRunGroup(proc->groupId());
    if (!runGroup || !runGroup->runnableGms()) return;
    QString gmsFilePath = runGroup->runnableGms()->location();
    ProjectFileNode *gdxNode = runGroup->findFile(gmsFilePath.left(gmsFilePath.lastIndexOf('.'))+"/out.gdx");
    if (gdxNode && gdxNode->file()->isOpen()) {
        if (gdxviewer::GdxViewer *gv = ViewHelper::toGdxViewer(gdxNode->file()->editors().first())) {
            if (progress == ProcState::Proc4GetResult) {
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
    ProjectRunGroupNode *runGroup = mProjectRepo.asRunGroup(proc->groupId());
    if (!runGroup || !runGroup->runnableGms()) return;
    QString gmsFilePath = runGroup->runnableGms()->location();
    QList<ProjectFileNode*> gdxNodes = runGroup->findFiles(FileKind::Gdx, true);
    for (ProjectFileNode *gdxNode : gdxNodes) {
        if (gdxNode->file()->isOpen()) {
            if (gdxviewer::GdxViewer *gv = ViewHelper::toGdxViewer(gdxNode->file()->editors().first())) {
                if (progress == ProcState::Proc4GetResult) {
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
void MainWindow::closeNodeConditionally(ProjectFileNode* node)
{
    // count nodes to the same file
    int nodeCountToFile = mProjectRepo.fileNodes(node->file()->id()).count();
    ProjectGroupNode *group = node->parentNode();
    ProjectRunGroupNode *runGroup = node->assignedRunGroup();
    if (runGroup && !terminateProcessesConditionally(QVector<ProjectRunGroupNode*>() << runGroup))
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

    // add to recently closed tabs
    mClosedTabs << fm->location();
    int lastIndex = mWp->isVisible() ? 1 : 0;

    NavigationHistoryLocator::navigationHistory()->stopRecord();
    // close all related editors, tabs and clean up
    while (!fm->editors().isEmpty()) {
        QWidget *edit = fm->editors().first();
        if (mRecent.editor() == edit) mRecent.reset();
        lastIndex = ui->mainTabs->indexOf(edit);
        ui->mainTabs->removeTab(lastIndex);
        fm->removeEditor(edit);
        edit->deleteLater();
    }
    mClosedTabsIndexes << lastIndex;
    // if the file has been removed, remove nodes
    if (!fm->exists(true)) fileDeletedExtern(fm->id());
    NavigationHistoryLocator::navigationHistory()->startRecord();
}

void MainWindow::openFilePath(const QString &filePath, bool focus, int codecMib, bool forcedAsTextEditor, NewTabStrategy tabStrategy)
{
    if (!QFileInfo(filePath).exists()) {
        EXCEPT() << "File not found: " << filePath;
    }
    ProjectFileNode *fileNode = mProjectRepo.findFile(filePath);

    if (!fileNode) {
        fileNode = mProjectRepo.findOrCreateFileNode(filePath);
        if (!fileNode)
            EXCEPT() << "Could not create node for file: " << filePath;
    }

    openFileNode(fileNode, focus, codecMib, forcedAsTextEditor, tabStrategy);
}

ProjectFileNode* MainWindow::addNode(const QString &path, const QString &fileName, ProjectGroupNode* group)
{
    ProjectFileNode *node = nullptr;
    if (!fileName.isEmpty()) {
        QFileInfo fInfo(path, fileName);
        FileType fType = FileType::from(fInfo.suffix());

        if (fType == FileKind::Gsp) {
            // Placeholder to read the project and create all nodes for associated files
        } else {
            node = mProjectRepo.findOrCreateFileNode(fInfo.absoluteFilePath(), group);
        }
    }
    return node;
}

void MainWindow::on_referenceJumpTo(reference::ReferenceItem item)
{
    QFileInfo fi(item.location);
    if (fi.isFile()) {
        ProjectFileNode* fn = mProjectRepo.findFileNode(mRecent.editor());
        if (fn) {
           ProjectRunGroupNode* runGroup =  fn->assignedRunGroup();
           mProjectRepo.findOrCreateFileNode(fi.absoluteFilePath(), runGroup);
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
    SettingsDialog sd(this);
    sd.setMiroSettingsEnabled(!mMiroRunning);
    connect(&sd, &SettingsDialog::editorFontChanged, this, &MainWindow::updateFixedFonts);
    connect(&sd, &SettingsDialog::editorLineWrappingChanged, this, &MainWindow::updateEditorLineWrapping);
    sd.exec();
    sd.disconnect();
    updateAndSaveSettings();
    if (sd.miroSettingsEnabled())
        ui->menuMIRO->setEnabled(isMiroAvailable());
}

void MainWindow::on_actionSearch_triggered()
{
    openSearchDialog();
}

void MainWindow::openSearchDialog()
{
    if (ui->dockHelpView->isAncestorOf(QApplication::focusWidget()) ||
        ui->dockHelpView->isAncestorOf(QApplication::activeWindow())) {
#ifdef QWEBENGINE
        mHelpWidget->on_searchHelp();
#endif
    } else if (mGamsParameterEditor->isAParameterEditorFocused(QApplication::focusWidget()) ||
               mGamsParameterEditor->isAParameterEditorFocused(QApplication::activeWindow())) {
                mGamsParameterEditor->selectSearchField();
    } else {
       ProjectFileNode *fc = mProjectRepo.findFileNode(mRecent.editor());
       if (fc) {
           if (fc->file()->kind() == FileKind::Gdx) {
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
       }

       // e.g. needed for KDE to raise the search dialog when minimized
       if (mSearchDialog->isMinimized())
           mSearchDialog->setWindowState(Qt::WindowMaximized);
       // toggle visibility
       if (mSearchDialog->isVisible()) {
           // e.g. needed for macOS to rasise search dialog when minimized
           mSearchDialog->raise();
           mSearchDialog->activateWindow();
           mSearchDialog->autofillSearchField();
       } else {

           if (mSearchWidgetPos.isNull()) {
               int sbs;
               if (mRecent.editor() && ViewHelper::toAbstractEdit(mRecent.editor())
                       && ViewHelper::toAbstractEdit(mRecent.editor())->verticalScrollBar()->isVisible())
                   sbs = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2;
               else
                   sbs = 2;

               QPoint c = mapToGlobal(centralWidget()->pos());

               int wDiff = frameGeometry().width() - geometry().width();
               int hDiff = frameGeometry().height() - geometry().height();

               int wSize = mSearchDialog->width() + wDiff;
               int hSize = mSearchDialog->height() + hDiff;

               QPoint p(qMin(c.x() + (centralWidget()->width() - sbs), QGuiApplication::primaryScreen()->virtualGeometry().width()) - wSize,
                        qMin(c.y() + centralWidget()->height(), QGuiApplication::primaryScreen()->virtualGeometry().height()) - hSize
                        );
               mSearchWidgetPos = p;
           }

           mSearchDialog->move(mSearchWidgetPos);
           mSearchDialog->show();
       }
    }
}

void MainWindow::showResults(search::SearchResultList* results)
{
    int index = ui->logTabs->indexOf(searchDialog()->resultsView()); // did widget exist before?

    searchDialog()->setResultsView(new search::ResultsView(results, this));
    connect(searchDialog()->resultsView(), &search::ResultsView::updateMatchLabel, searchDialog(), &search::SearchDialog::updateNrMatches, Qt::UniqueConnection);

    QString nr;
    if (results->size() > MAX_SEARCH_RESULTS-1) nr = QString::number(MAX_SEARCH_RESULTS) + "+";
    else nr = QString::number(results->size());

    QString title("Results: " + results->searchRegex().pattern() + " (" + nr + ")");

    ui->dockProcessLog->show();
    ui->dockProcessLog->activateWindow();
    ui->dockProcessLog->raise();

    if (index != -1) ui->logTabs->removeTab(index); // remove old result page

    ui->logTabs->addTab(searchDialog()->resultsView(), title); // add new result page
    ui->logTabs->setCurrentWidget(searchDialog()->resultsView());
}

void MainWindow::closeResultsPage()
{
    int index = ui->logTabs->indexOf(searchDialog()->resultsView());
    if (index != -1) ui->logTabs->removeTab(index);
    mSearchDialog->setResultsView(nullptr);
}

void MainWindow::updateFixedFonts(const QString &fontFamily, int fontSize)
{
    QFont font = createEditorFont(fontFamily, fontSize);
    for (QWidget* edit: openEditors()) {
        if (ViewHelper::toCodeEdit(edit))
            ViewHelper::toAbstractEdit(edit)->setFont(font);
        else if (ViewHelper::toTextView(edit))
            ViewHelper::toTextView(edit)->edit()->setFont(font);
    }
    for (QWidget* log: openLogs()) {
        if (ViewHelper::toTextView(log))
            ViewHelper::toTextView(log)->edit()->setFont(font);
        else
            log->setFont(font);
    }

    mSyslog->setFont(font);
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
            ed->blockCountChanged(0); // force redraw for line number area
            ed->setLineWrapMode(ViewHelper::editorType(ed) == EditorType::syslog ? wrapModeProcess
                                                                                 : wrapModeEditor);
        }
        if (TextView* tv = ViewHelper::toTextView(editList.at(i))) {
            tv->blockCountChanged(); // force redraw for line number area
            tv->setLineWrapMode(ViewHelper::editorType(tv) == EditorType::log ? wrapModeProcess
                                                                              : wrapModeEditor);
        }
    }
}

bool MainWindow::readTabs(const QVariantMap &tabData)
{
    QString curTab;
    if (tabData.contains("mainTabRecent")) {
        QString location = tabData.value("mainTabRecent").toString();
        if (QFileInfo(location).exists()) {
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
                if (QFileInfo(location).exists()) {
                    openFilePath(location, false, -1, false, tabStrategy);
                    mOpenTabsList << location;
                }
                if (i % 10 == 0) QApplication::processEvents(QEventLoop::AllEvents, 1);
                if (ui->mainTabs->count() <= i)
                    return false;
            }
        }
    }
    if (ui->mainTabs->currentWidget() == mWp) mRecent.reset();
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
        if (!fm) continue;
        QVariantMap tabObject;
        tabObject.insert("location", fm->location());
        tabArray << tabObject;
    }
    tabData.insert("mainTabs", tabArray);

    FileMeta *fm = mRecent.editor() ? mFileMetaRepo.fileMeta(mRecent.editor()) : nullptr;
    if (fm)
        tabData.insert("mainTabRecent", fm->location());
    else if (ui->mainTabs->currentWidget() == mWp)
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
void MainWindow::on_actionGo_To_triggered()
{
    if (ui->mainTabs->currentWidget() == mWp) return;
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
    if (wid == mHelpWidget) {
        mHelpWidget->copySelection();
        return;
    }
#endif

    // KEEP-ORDER: FIRST check focus THEN check recent-edit (in descending inheritance)

    if (TextView *tv = ViewHelper::toTextView(focusWidget())) {
        tv->copySelection();
    } else if (gdxviewer::GdxViewer *gdx = ViewHelper::toGdxViewer(mRecent.editor())) {
        gdx->copyAction();
    } else if (option::SolverOptionWidget *sow = ViewHelper::toSolverOptionEdit(mRecent.editor())) {
        sow->copyAction();
    } else if (focusWidget() == mSyslog) {
        mSyslog->copy();
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
        Settings *settings = Settings::settings();
        updateFixedFonts(settings->toString(skEdFontFamily), settings->toInt(skEdFontSize)); // reset all editors
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
        if (AbstractEdit *ae = ViewHelper::toAbstractEdit(QApplication::focusWidget())) {
            int pix = ae->fontInfo().pixelSize();
            if (pix == ae->fontInfo().pixelSize()) ae->zoomOut();
        }
        if (TextView *tv = ViewHelper::toTextView(QApplication::focusWidget())) {
            int pix = tv->fontInfo().pixelSize();
            if (pix == tv->fontInfo().pixelSize()) tv->zoomOut();
        }
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
        if (AbstractEdit *ae = ViewHelper::toAbstractEdit(QApplication::focusWidget())) {
            int pix = ae->fontInfo().pixelSize();
            ae->zoomIn();
            if (pix == ae->fontInfo().pixelSize() && ae->fontInfo().pointSize() > 1) ae->zoomIn();
        }
        if (TextView *tv = ViewHelper::toTextView(QApplication::focusWidget())) {
            int pix = tv->fontInfo().pixelSize();
            tv->zoomIn();
            if (pix == tv->fontInfo().pixelSize() && tv->fontInfo().pointSize() > 1) tv->zoomIn();
        }
    }
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
        updateEditorMode();
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
        ui->actionToggle_Extended_Parameter_Editor->setIcon(Scheme::icon(":/%1/hide"));
        ui->actionToggle_Extended_Parameter_Editor->setToolTip("<html><head/><body><p>Hide Extended Parameter Editor (<span style=\"font-weight:600;\">Ctrl+ALt+L</span>)</p></body></html>");
    } else {
        ui->actionToggle_Extended_Parameter_Editor->setIcon(Scheme::icon(":/%1/show") );
        ui->actionToggle_Extended_Parameter_Editor->setToolTip("<html><head/><body><p>Show Extended Parameter Editor (<span style=\"font-weight:600;\">Ctrl+ALt+L</span>)</p></body></html>");
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

    QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
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
    ProjectRunGroupNode* node = mProjectRepo.findRunGroup(ViewHelper::groupId(mRecent.editor()));
    QString path = node ? QDir::toNativeSeparators(node->location()) : currentPath();

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

QFont MainWindow::createEditorFont(const QString &fontFamily, int pointSize)
{
    QFont font(fontFamily, pointSize);
    font.setHintingPreference(QFont::PreferNoHinting);
    return font;
}

bool MainWindow::isMiroAvailable()
{
    if (Settings::settings()->toString(skMiroInstallPath).isEmpty())
        return false;
    QFileInfo fileInfo(Settings::settings()->toString(skMiroInstallPath));
    return fileInfo.exists();
}

bool MainWindow::validMiroPrerequisites()
{
    if (!isMiroAvailable()) {
        auto msg = QString("Could not find MIRO at %1. Please check your MIRO settings.")
                .arg(Settings::settings()->toString(skMiroInstallPath));
        SysLogLocator::systemLog()->append(msg, LogMsgType::Error);
        return false;
    }

    return mRecent.hasValidRunGroup();
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
    ProjectGroupNode* pgDiff   = nullptr;

    // if possible get the group to which both input files belong
    if (fmInput1 && fmInput2) {
        QVector<ProjectFileNode*> nodesInput1 = mProjectRepo.fileNodes(fmInput1->id());
        QVector<ProjectFileNode*> nodesInput2 = mProjectRepo.fileNodes(fmInput2->id());

        if (nodesInput1.size() == 1 && nodesInput2.size() == 1) {
            if (nodesInput1.first()->parentNode() == nodesInput2.first()->parentNode())
                pgDiff = nodesInput1.first()->parentNode();
        }
    }
    // if no group was found, we try to open the file in the first node that contains the it
    if (pgDiff == nullptr) {
        FileMeta *fm = mFileMetaRepo.fileMeta(diffFile);
        if (fm) {
            QVector<ProjectFileNode*> v = mProjectRepo.fileNodes(fm->id());
            if(v.size() == 1)
                pgDiff = v.first()->parentNode();
        }
    }
    ProjectFileNode *node = mProjectRepo.findOrCreateFileNode(diffFile, pgDiff);
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

    // check if tab is still opened
    if (ui->mainTabs->indexOf(item.tab) > 0) {
        ui->mainTabs->setCurrentWidget(item.tab);
        item.tab->setFocus();
    } else {
        if (!item.filePath.isEmpty()) {
            openFilePath(item.filePath, true);
        }
    }

    if (item.lineNr >= 0) {
        // restore text cursor if editor available
        if (CodeEdit* ce = ViewHelper::toCodeEdit(mNavigationHistory->currentTab()))
            ce->jumpTo(item.lineNr, item.col);
        else if (TextView* tv = ViewHelper::toTextView(mNavigationHistory->currentTab()))
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
    if (CodeEdit* ce = ViewHelper::toCodeEdit(mNavigationHistory->currentTab())) {
        ce->foldAll();
    }
}

void MainWindow::on_actionUnfoldAllTextBlocks_triggered()
{
    if (CodeEdit* ce = ViewHelper::toCodeEdit(mNavigationHistory->currentTab())) {
        ce->unfoldAll();
    }
}

void MainWindow::on_actionPrint_triggered()
{
    QPrinter printer;
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editor());
    if (!fm || !focusWidget()) return;
    if (focusWidget() == mRecent.editor()) {
        auto* abstractEdit = ViewHelper::toAbstractEdit(mainTabs()->currentWidget());
        if (!abstractEdit) return;
        QPrintDialog dialog(&printer, this);
        if (dialog.exec() == QDialog::Rejected) return;
        abstractEdit->print(&printer);
    } else if (ViewHelper::editorType(recent()->editor()) == EditorType::lxiLst) {
        auto* lxiViewer = ViewHelper::toLxiViewer(mainTabs()->currentWidget());
        if (!lxiViewer) return;
        QPrintDialog dialog(&printer, this);
        if (dialog.exec() == QDialog::Rejected) return;
        lxiViewer->print(&printer);
    } else if (ViewHelper::editorType(recent()->editor()) == EditorType::txtRo) {
        auto* textViewer = ViewHelper::toTextView(mainTabs()->currentWidget());
        if (!textViewer) return;
        QPrintDialog dialog(&printer, this);
        if (dialog.exec() == QDialog::Rejected) return;
        textViewer->print(&printer);
    }
}

bool MainWindow::enabledPrintAction()
{
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editor());
    if (!fm || !focusWidget())
        return false;
    return focusWidget() == mRecent.editor()
            || ViewHelper::editorType(recent()->editor()) == EditorType::lxiLst
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
        auto licenseFile = QDir::toNativeSeparators(CommonPaths::gamsLicenseFilePath());
        if (QFileInfo::exists(licenseFile)) {
            appendSystemLogInfo("GAMS license found at " + licenseFile);
        } else {
            appendSystemLogError(errorText);
        }
    }  catch (Exception e) {
        appendSystemLogError(e.what());
    }
}

}
}

