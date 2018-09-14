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
#include <QtConcurrent>
#include <QShortcut>
#include <QtWidgets>

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
#include "gamsprocess.h"
#include "gamslibprocess.h"
#include "lxiviewer/lxiviewer.h"
#include "gdxviewer/gdxviewer.h"
#include "locators/searchlocator.h"
#include "locators/settingslocator.h"
#include "locators/sysloglocator.h"
#include "logger.h"
#include "studiosettings.h"
#include "settingsdialog.h"
#include "search/searchdialog.h"
#include "search/searchresultlist.h"
#include "resultsview.h"
#include "gotodialog.h"
#include "updatedialog.h"
#include "checkforupdatewrapper.h"
#include "autosavehandler.h"
#include "distributionvalidator.h"
#include "tabdialog.h"

namespace gams {
namespace studio {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      mFileMetaRepo(this),
      mProjectRepo(this),
      mTextMarkRepo(&mFileMetaRepo, &mProjectRepo, this),

      mAutosaveHandler(new AutosaveHandler(this))
{
    mSettings = SettingsLocator::settings();
    mHistory = new HistoryData();
//    QFile css(":/data/style.css");
//    if (css.open(QFile::ReadOnly | QFile::Text)) {
//        this->setStyleSheet(css.readAll());
//    }

    ui->setupUi(this);

    mFileTimer.setSingleShot(true);
    mFileTimer.setInterval(100);
    connect(&mFileTimer, &QTimer::timeout, this, &MainWindow::processFileEvents);
    mTimerID = startTimer(60000);

    setAcceptDrops(true);
    ui->actionRedo->setShortcuts(ui->actionRedo->shortcuts() << QKeySequence("Ctrl+Shift+Z"));
#ifdef __APPLE__
    ui->actionNextTab->setShortcut(QKeySequence("Ctrl+}"));
    ui->actionPreviousTab->setShortcut(QKeySequence("Ctrl+{"));
#endif

    QFont font = ui->statusBar->font();
    font.setPointSizeF(font.pointSizeF()*0.9);
    ui->statusBar->setFont(font);
    mStatusWidgets = new StatusWidgets(this);
    int iconSize = fontInfo().pixelSize()*2-1;
    ui->projectView->setModel(mProjectRepo.treeModel());
    ui->projectView->setRootIndex(mProjectRepo.treeModel()->rootModelIndex());
    ui->projectView->setHeaderHidden(true);
    ui->projectView->setItemDelegate(new TreeItemDelegate(ui->projectView));
    ui->projectView->setIconSize(QSize(qRound(iconSize*0.8), qRound(iconSize*0.8)));
    ui->projectView->setContextMenuPolicy(Qt::CustomContextMenu);

//    mTextMarkRepo = new TextMarkRepo(&mProjectRepo, this);
    mProjectRepo.init(&mFileMetaRepo, &mTextMarkRepo);
    mFileMetaRepo.init(&mTextMarkRepo, &mProjectRepo);

    // TODO(JM) it is possible to put the QTabBar into the docks title:
    //          if we override the QTabWidget it should be possible to extend it over the old tab-bar-space
//    ui->dockLogView->setTitleBarWidget(ui->tabLog->tabBar());

    mHelpWidget = new HelpWidget(this);
    ui->dockHelpView->setWidget(mHelpWidget);
    ui->dockHelpView->hide();

    mGamsOptionWidget = new OptionWidget(ui->actionRun, ui->actionRun_with_GDX_Creation,
                                         ui->actionCompile, ui->actionCompile_with_GDX_Creation,
                                         ui->actionInterrupt, ui->actionStop,
                                         this);
    ui->dockOptionEditor->setWidget(mGamsOptionWidget);
    ui->dockOptionEditor->show();

    mCodecGroupReload = new QActionGroup(this);
    connect(mCodecGroupReload, &QActionGroup::triggered, this, &MainWindow::codecReload);
    mCodecGroupSwitch = new QActionGroup(this);
    connect(mCodecGroupSwitch, &QActionGroup::triggered, this, &MainWindow::codecChanged);
    connect(ui->mainTab, &QTabWidget::currentChanged, this, &MainWindow::activeTabChanged);

    connect(&mFileMetaRepo, &FileMetaRepo::fileEvent, this, &MainWindow::fileEvent);
    connect(&mProjectRepo, &ProjectRepo::changed, this, &MainWindow::storeTree);
    connect(&mProjectRepo, &ProjectRepo::openFile, this, &MainWindow::openFile);
    connect(&mProjectRepo, &ProjectRepo::setNodeExpanded, this, &MainWindow::setProjectNodeExpanded);
    connect(&mProjectRepo, &ProjectRepo::isNodeExpanded, this, &MainWindow::isProjectNodeExpanded);
    connect(&mProjectRepo, &ProjectRepo::gamsProcessStateChanged, this, &MainWindow::gamsProcessStateChanged);

    connect(ui->projectView->selectionModel(), &QItemSelectionModel::currentChanged, &mProjectRepo, &ProjectRepo::setSelected);
    connect(ui->projectView, &QTreeView::customContextMenuRequested, this, &MainWindow::projectContextMenuRequested);
    connect(&mProjectContextMenu, &ProjectContextMenu::closeGroup, this, &MainWindow::closeGroup);
//    connect(&mProjectContextMenu, &ProjectContextMenu::renameGroup, this, &MainWindow::renameGroup);
    connect(&mProjectContextMenu, &ProjectContextMenu::closeFile, this, &MainWindow::closeNodeConditionally);
    connect(&mProjectContextMenu, &ProjectContextMenu::addExistingFile, this, &MainWindow::addToGroup);
    connect(&mProjectContextMenu, &ProjectContextMenu::getSourcePath, this, &MainWindow::sendSourcePath);
    connect(&mProjectContextMenu, &ProjectContextMenu::runFile, this, &MainWindow::runGmsFile);
    connect(&mProjectContextMenu, &ProjectContextMenu::setMainFile, this, &MainWindow::setMainGms);
    connect(&mProjectContextMenu, &ProjectContextMenu::openLogFor, this, &MainWindow::changeToLog);
    connect(ui->dockProjectView, &QDockWidget::visibilityChanged, this, &MainWindow::projectViewVisibiltyChanged);
    connect(ui->dockLogView, &QDockWidget::visibilityChanged, this, &MainWindow::outputViewVisibiltyChanged);
    connect(ui->dockHelpView, &QDockWidget::visibilityChanged, this, &MainWindow::helpViewVisibilityChanged);
    connect(ui->dockOptionEditor, &QDockWidget::visibilityChanged, this, &MainWindow::optionViewVisibiltyChanged);

    setEncodingMIBs(encodingMIBs());
    ui->menuEncoding->setEnabled(false);
    mSettings->loadSettings(this);
    mRecent.path = mSettings->defaultWorkspace();
    mSearchDialog = new SearchDialog(this);

    if (mSettings->resetSettingsSwitch()) mSettings->resetSettings();

    // stack help under output
    tabifyDockWidget(ui->dockHelpView, ui->dockLogView);

    mSyslog = new SystemLogEdit(this);
    mSyslog->setFont(QFont(mSettings->fontFamily(), mSettings->fontSize()));
    ui->logTabs->addTab(mSyslog, "System");

    initTabs();
    QPushButton *tabMenu = new QPushButton(QIcon(":/img/menu"), "", ui->mainTab);
    connect(tabMenu, &QPushButton::pressed, this, &MainWindow::showMainTabsMenu);
    tabMenu->setMaximumWidth(40);
    ui->mainTab->setCornerWidget(tabMenu);
    tabMenu = new QPushButton(QIcon(":/img/menu"), "", ui->logTabs);
    connect(tabMenu, &QPushButton::pressed, this, &MainWindow::showLogTabsMenu);
    tabMenu->setMaximumWidth(40);
    ui->logTabs->setCornerWidget(tabMenu);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_K), this, SLOT(showTabsMenu()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F12), this, SLOT(toggleDebugMode()));

    // set up services
    SearchLocator::provide(mSearchDialog);
    SettingsLocator::provide(mSettings);
    SysLogLocator::provide(mSyslog);
}

void MainWindow::delayedFileRestoration()
{
    mSettings->restoreTabsAndProjects(this);
    mSettings->restoreLastFilesUsed(this);
}

MainWindow::~MainWindow()
{
    killTimer(mTimerID);
    delete mWp;
    delete ui;
    FileType::clear();
}

void MainWindow::initTabs()
{
//    QPalette pal = ui->projectView->palette();
//    pal.setColor(QPalette::Highlight, Qt::transparent);
//    ui->projectView->setPalette(pal);

    mWp = new WelcomePage(history(), this);
    connect(mWp, &WelcomePage::openFilePath, this, &MainWindow::openFilePath);
    if (mSettings->skipWelcomePage())
        mWp->hide();
    else
        showWelcomePage();

}

void MainWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    mAutosaveHandler->saveChangedFiles();
    mSettings->saveSettings(this);
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate) {
        processFileEvents();
    }
    return QMainWindow::event(event);
}

void MainWindow::addToGroup(ProjectGroupNode* group, const QString& filepath)
{
    mProjectRepo.findOrCreateFileNode(filepath, group);
}

void MainWindow::sendSourcePath(QString &source)
{
    source = mRecent.path;
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
    ui->actionOutput_View->setChecked(visibility);
    ui->dockLogView->setVisible(visibility);
}

void MainWindow::setProjectViewVisibility(bool visibility)
{
    ui->actionProject_View->setChecked(visibility);
    ui->dockProjectView->setVisible(visibility);
}

void MainWindow::setOptionEditorVisibility(bool visibility)
{
    ui->actionOption_View->setChecked(visibility);
    ui->dockOptionEditor->setVisible(visibility);
}

void MainWindow::setHelpViewVisibility(bool visibility)
{
    if (!visibility)
        mHelpWidget->clearStatusBar();
    else
        mHelpWidget->setFocus();
    ui->actionHelp_View->setChecked(visibility);
    ui->dockHelpView->setVisible(visibility);
}

bool MainWindow::outputViewVisibility()
{
    return ui->actionOutput_View->isChecked();
}

bool MainWindow::projectViewVisibility()
{
    return ui->actionProject_View->isChecked();
}

bool MainWindow::optionEditorVisibility()
{
    return ui->actionOption_View->isChecked();
}

bool MainWindow::helpViewVisibility()
{
    return ui->actionHelp_View->isChecked();
}

void MainWindow::on_actionOutput_View_triggered(bool checked)
{
    dockWidgetShow(ui->dockLogView, checked);
}

void MainWindow::on_actionProject_View_triggered(bool checked)
{
    dockWidgetShow(ui->dockProjectView, checked);
}

void MainWindow::on_actionOption_View_triggered(bool checked)
{
    dockWidgetShow(ui->dockOptionEditor, checked);
    if(!checked) ui->dockOptionEditor->setFloating(false);
}

void MainWindow::on_actionHelp_View_triggered(bool checked)
{
    dockWidgetShow(ui->dockHelpView, checked);
}

void MainWindow::checkOptionDefinition(bool checked)
{
    mGamsOptionWidget->checkOptionDefinition(checked);
}

bool MainWindow::isOptionDefinitionChecked()
{
    return mGamsOptionWidget->isOptionDefinitionChecked();
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
    for (int i = 0; i < ui->mainTab->count(); ++i) {
        res << ui->mainTab->widget(i);
    }
    return res;
}

QList<AbstractEdit*> MainWindow::openLogs()
{
    QList<AbstractEdit*> resList;
    for (int i = 0; i < ui->logTabs->count(); i++) {
        AbstractEdit* ed = FileMeta::toAbstractEdit(ui->logTabs->widget(i));
        if (ed) resList << ed;
    }
    return resList;
}

void MainWindow::receiveAction(QString action)
{
    if (action == "createNewFile")
        on_actionNew_triggered();
    else if(action == "browseModLib")
        on_actionGAMS_Library_triggered();
}

void MainWindow::openModelFromLib(QString glbFile, LibraryItem* model)
{
    QFileInfo file(model->files().first());
    QString inputFile = file.completeBaseName() + ".gms";

    openModelFromLib(glbFile, model->name(), inputFile);
}

void MainWindow::openModelFromLib(const QString &glbFile, const QString &modelName, const QString &inputFile)
{
    QDir gamsSysDir(CommonPaths::systemDir());
    mLibProcess = new GAMSLibProcess(this);
    mLibProcess->setGlbFile(gamsSysDir.filePath(glbFile));
    mLibProcess->setModelName(modelName);
    mLibProcess->setInputFile(inputFile);
    mLibProcess->setTargetDir(mSettings->defaultWorkspace());
    mLibProcess->execute();

    // This log is passed to the system-wide log
    connect(mLibProcess, &GamsProcess::newStdChannelData, this, &MainWindow::appendSystemLog);
    connect(mLibProcess, &GamsProcess::finished, this, &MainWindow::postGamsLibRun);
}

void MainWindow::receiveModLibLoad(QString gmsFile)
{
    openModelFromLib("gamslib_ml/gamslib.glb", gmsFile, gmsFile + ".gms");
}

void MainWindow::receiveOpenDoc(QString doc, QString anchor)
{
    QString link = CommonPaths::systemDir() + "/" + doc;
    QUrl result = QUrl::fromLocalFile(link);

    if (!anchor.isEmpty())
        result = QUrl(result.toString() + "#" + anchor);

    helpWidget()->on_urlOpened(result);

    on_actionHelp_View_triggered(true);
}

SearchDialog* MainWindow::searchDialog() const
{
    return mSearchDialog;
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
    if (mRecent.group == group) updateRunState();
}

void MainWindow::projectContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = ui->projectView->indexAt(pos);
    if (!index.isValid()) return;
    mProjectContextMenu.setNode(mProjectRepo.node(index));
    mProjectContextMenu.setParent(this);
    mProjectContextMenu.exec(ui->projectView->viewport()->mapToGlobal(pos));
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
    ui->actionOutput_View->setChecked(visibility || tabifiedDockWidgets(ui->dockLogView).count());
}

void MainWindow::projectViewVisibiltyChanged(bool visibility)
{
    ui->actionProject_View->setChecked(visibility || tabifiedDockWidgets(ui->dockProjectView).count());
}

void MainWindow::optionViewVisibiltyChanged(bool visibility)
{
    ui->actionOption_View->setChecked(visibility || tabifiedDockWidgets(ui->dockOptionEditor).count());
}

void MainWindow::helpViewVisibilityChanged(bool visibility)
{
    ui->actionHelp_View->setChecked(visibility || tabifiedDockWidgets(ui->dockHelpView).count());
}

void MainWindow::showMainTabsMenu()
{
    TabDialog *tabDialog = new TabDialog(ui->mainTab, this);
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
    QWidget * wid = focusWidget();
    while (wid) {
        if (wid == ui->mainTab) {
            showMainTabsMenu();
            break;
        }
        if (wid == ui->logTabs) {
            showLogTabsMenu();
            break;
        }
        wid = wid->parentWidget();
    }
}

void MainWindow::updateEditorPos()
{
    QPoint pos;
    QPoint anchor;
    AbstractEdit* edit = FileMeta::toAbstractEdit(mRecent.editor());
    CodeEdit *ce = FileMeta::toCodeEdit(edit);
    if (ce) {
        ce->getPositionAndAnchor(pos, anchor);
        mStatusWidgets->setPosAndAnchor(pos, anchor);
    } else if (edit) {
        QTextCursor cursor = edit->textCursor();
        pos = QPoint(cursor.positionInBlock()+1, cursor.blockNumber()+1);
        if (cursor.hasSelection()) {
            cursor.setPosition(cursor.anchor());
            anchor = QPoint(cursor.positionInBlock()+1, cursor.blockNumber()+1);
        }
    }
    mStatusWidgets->setPosAndAnchor(pos, anchor);
}

void MainWindow::updateEditorMode()
{
    CodeEdit* edit = FileMeta::toCodeEdit(mRecent.editor());
    if (!edit || edit->isReadOnly()) {
        mStatusWidgets->setEditMode(EditMode::Readonly);
    } else {
        mStatusWidgets->setEditMode(edit->overwriteMode() ? EditMode::Overwrite : EditMode::Insert);
    }
}

void MainWindow::updateEditorBlockCount()
{
    AbstractEdit* edit = FileMeta::toAbstractEdit(mRecent.editor());
    if (edit) mStatusWidgets->setLineCount(edit->blockCount());
}

void MainWindow::currentDocumentChanged(int from, int charsRemoved, int charsAdded)
{
    searchDialog()->on_documentContentChanged(from, charsRemoved, charsAdded);
}

void MainWindow::getAdvancedActions(QList<QAction*>* actions)
{
    QList<QAction*> act(ui->menuAdvanced->actions());
    *actions = act;
}

void MainWindow::on_actionNew_triggered()
{
    QString path = mRecent.path;
    if (mRecent.editFileId >= 0) {
        FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editFileId);
        if (fm) path = QFileInfo(fm->location()).path();
    }
    QString filePath = QFileDialog::getSaveFileName(this, "Create new file...", path,
                                                    tr("GAMS code (*.gms *.inc );;"
                                                       "Text files (*.txt);;"
                                                       "All files (*.*)"));

    if (filePath == "") return;
    QFileInfo fi(filePath);

    if (fi.suffix().isEmpty())
        filePath += ".gms";
    QFile file(filePath);

    if (!file.exists()) { // new
        file.open(QIODevice::WriteOnly);
        file.close();
    } else { // replace old
        file.resize(0);
    }

    if (ProjectFileNode *fc = addNode("", filePath)) {
        fc->file()->save();
        openFileNode(fc);
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString path = QFileInfo(mRecent.path).path();
    QStringList fNames = QFileDialog::getOpenFileNames(this, "Open file", path,
                                                       tr("GAMS code (*.gms *.inc *.gdx *.lst *.opt *ref);;"
                                                          "Text files (*.txt);;"
                                                          "All files (*.*)"),
                                                       nullptr,
                                                       DONT_RESOLVE_SYMLINKS_ON_MACOS);

    for (QString item: fNames) {
        openFileNode( addNode("", item) );
    }
}

void MainWindow::on_actionSave_triggered()
{
    FileMeta* fm = mFileMetaRepo.fileMeta(mRecent.editFileId);
    if (!fm) return;
    if (fm->kind() == FileKind::Log /* TODO(JM) .. or file was new */ ) {
        on_actionSave_As_triggered();
    } else if (fm->isModified()) {
        fm->save();
    }
}

void MainWindow::on_actionSave_As_triggered()
{
    ProjectFileNode *node = mProjectRepo.findFileNode(mRecent.editor());
    if (!node) return;
    FileMeta *fileMeta = node->file();
    QString path = QFileInfo(fileMeta->location()).path();
    auto filePath = QFileDialog::getSaveFileName(this,
                                                 "Save file as...",
                                                 path,
                                                 tr("GAMS code (*.gms *.inc);;"
                                                    "Text files (*.txt);;"
                                                    "All files (*.*)"));
    if (!filePath.isEmpty()) {
        mRecent.path = QFileInfo(filePath).path();

        if(fileMeta->location().endsWith(".gms", Qt::CaseInsensitive) && !filePath.endsWith(".gms", Qt::CaseInsensitive)) {
            filePath = filePath + ".gms";
        } else if (fileMeta->location().endsWith(".gdx", Qt::CaseInsensitive) && !filePath.endsWith(".gdx", Qt::CaseInsensitive)) {
            filePath = filePath + ".gdx";
        } else if (fileMeta->location().endsWith(".lst", Qt::CaseInsensitive) && !filePath.endsWith(".lst", Qt::CaseInsensitive)) {
            filePath = filePath + ".lst";
        } // TODO: check if there are others to add


        // perform copy when file is a gdx file
        if (fileMeta->kind() == FileKind::Gdx) {
            if (QFile::exists(filePath))
                QFile::remove(filePath);
            QFile::copy(fileMeta->location(), filePath);
        } else {
            fileMeta->saveAs(filePath);
            openFileNode(node, true);
            mStatusWidgets->setFileName(fileMeta->location());
            mSettings->saveSettings(this);
        }
    }
}

void MainWindow::on_actionSave_All_triggered()
{
    for (FileMeta* fm: mFileMetaRepo.openFiles())
        fm->save();
}

void MainWindow::on_actionClose_triggered()
{
    on_mainTab_tabCloseRequested(ui->mainTab->currentIndex());
}

void MainWindow::on_actionClose_All_triggered()
{
    disconnect(ui->mainTab, &QTabWidget::currentChanged, this, &MainWindow::activeTabChanged);
    if (ui->mainTab->count() > 1)
        ui->mainTab->tabBar()->moveTab(ui->mainTab->currentIndex(), ui->mainTab->count()-1);

    for(int i = ui->mainTab->count(); i > 0; i--) {
        on_mainTab_tabCloseRequested(0);
    }
    connect(ui->mainTab, &QTabWidget::currentChanged, this, &MainWindow::activeTabChanged);
}

void MainWindow::on_actionClose_All_Except_triggered()
{
    int except = ui->mainTab->currentIndex();
    for(int i = ui->mainTab->count(); i >= 0; i--) {
        if(i != except) {
            on_mainTab_tabCloseRequested(i);
        }
    }
}

void MainWindow::codecChanged(QAction *action)
{
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editFileId);
    if (fm) {
        if (fm->document() && !fm->isReadOnly()) {
            fm->setCodecMib(action->data().toInt());
        }
        updateMenuToCodec(action->data().toInt());
        mStatusWidgets->setEncoding(fm->codecMib());
    }
}

void MainWindow::codecReload(QAction *action)
{
    if (!focusWidget()) return;
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editFileId);
    if (fm && fm->kind() == FileKind::Log) return;

    if (fm && fm->codecMib() != action->data().toInt()) {
        bool reload = true;
        if (fm->isModified()) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(fm->location()+" has been modified.");
            msgBox.setInformativeText("Do you want to discard your changes and reload it with Character Set "
                                      + action->text() + "?");
            msgBox.addButton(tr("Discard and Reload"), QMessageBox::ResetRole);
            msgBox.setStandardButtons(QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            reload = msgBox.exec();
        }
        if (reload) {
            fm->load(action->data().toInt());
            updateMenuToCodec(action->data().toInt());
            mStatusWidgets->setEncoding(fm->codecMib());
        }
    }
}

void MainWindow::loadCommandLineOptions(ProjectFileNode* oldfn, ProjectFileNode* fn)
{
    if (oldfn) { // switch from a non-welcome page
        ProjectRunGroupNode* oldgroup = oldfn->assignedRunGroup();
        if (!oldgroup) return;
        oldgroup->addRunParametersHistory( mGamsOptionWidget->getCurrentCommandLineData() );

        if (!fn) { // switch to a welcome page
            QStringList runParametersHistory;
            mGamsOptionWidget->loadCommandLineOption(runParametersHistory);
            return;
        }

        ProjectRunGroupNode* group = fn->assignedRunGroup();
        if (!group) return;
        if (group == oldgroup) return;

        mGamsOptionWidget->loadCommandLineOption( group->getRunParametersHistory() );

    } else { // switch from a welcome page
        if (!fn) { // switch to a welcome page
            QStringList runParametersHistory;
            mGamsOptionWidget->loadCommandLineOption(runParametersHistory);
            return;
        }

        ProjectRunGroupNode* group = fn->assignedRunGroup();
        if (!group) return;

        mGamsOptionWidget->loadCommandLineOption( group->getRunParametersHistory() );
    }
}

void MainWindow::activeTabChanged(int index)
{
    // remove highlights from old tab
    ProjectFileNode* oldTab = mProjectRepo.findFileNode(mRecent.editor());
//    if (oldTab)
//        oldTab->removeTextMarks(TextMark::match, true);

    // TODO(JM) eventually crashes on close
    mRecent.setEditor(nullptr, this);

    QWidget *editWidget = (index < 0 ? nullptr : ui->mainTab->widget(index));
    AbstractEdit* edit = FileMeta::toAbstractEdit(editWidget);
    ProjectFileNode* node = mProjectRepo.findFileNode(editWidget);

    loadCommandLineOptions(oldTab, mProjectRepo.findFileNode(editWidget));
    updateRunState();

    // TODO(JM) missing common base class for all viewers/editors
    if (node) {
        mRecent.editFileId = node->file()->id();
        mStatusWidgets->setFileName(node->location());
        mStatusWidgets->setEncoding(node->file()->codecMib());
        if (edit) {
            mRecent.setEditor(edit, this);
            mRecent.group = mProjectRepo.asGroup(edit->groupId());

            // TODO(JM) find current ProjectFileNode to load commandlineoptions
            if (!edit->isReadOnly()) {
                ui->menuEncoding->setEnabled(true);
            }
            updateMenuToCodec(node->file()->codecMib());
            mStatusWidgets->setLineCount(edit->blockCount());
            ui->menuEncoding->setEnabled(node && !edit->isReadOnly());
        } else if (gdxviewer::GdxViewer *gdxViewer = FileMeta::toGdxViewer(editWidget)) {
            ui->menuEncoding->setEnabled(false);
            mRecent.setEditor(gdxViewer, this);
            mRecent.group = mProjectRepo.asGroup(gdxViewer->groupId());
            mStatusWidgets->setLineCount(-1);
            gdxViewer->reload();
        } else if (reference::ReferenceViewer* refViewer = FileMeta::toReferenceViewer(editWidget)) {
            ui->menuEncoding->setEnabled(false);
            mRecent.setEditor(refViewer, this);
            ProjectFileNode* fc = mProjectRepo.findFileNode(refViewer);
            if (fc) {
                mRecent.editFileId = fc->file()->id();
                mRecent.group = fc->parentNode();
                mStatusWidgets->setFileName(fc->location());
                mStatusWidgets->setEncoding(fc->file()->codecMib());
                mStatusWidgets->setLineCount(-1);
            }
        }
    } else {
        ui->menuEncoding->setEnabled(false);
        mStatusWidgets->setFileName("");
        mStatusWidgets->setEncoding(-1);
        mStatusWidgets->setLineCount(-1);
    }

    if (searchDialog()) searchDialog()->updateReplaceActionAvailability();

    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly()) ce->setOverwriteMode(mOverwriteMode);
    updateEditorMode();
}

void MainWindow::fileChanged(FileId fileId)
{
    mProjectRepo.fileChanged(fileId);
    FileMeta *fm = mFileMetaRepo.fileMeta(fileId);
    if (!fm) return;
    for (QWidget *edit: fm->editors()) {
        int index = ui->mainTab->indexOf(edit);
        if (index >= 0) {
            if (fm) ui->mainTab->setTabText(index, fm->name(NameModifier::editState));
        }
    }
}

void MainWindow::fileClosed(FileId fileId)
{
    Q_UNUSED(fileId)
    // TODO(JM) check if anything needs to be updated
}

void MainWindow::fileChangedExtern(FileId fileId)
{
    FileMeta *file = mFileMetaRepo.fileMeta(fileId);
    // file has not been loaded: nothing to do
    if (!file->isOpen()) return;
    if (file->kind() == FileKind::Log) return;

    int choice;

    if (file->isAutoReload() || file->isReadOnly()) {
        choice = 0;
    } else {
        if (!file->isModified()) {
            choice = QMessageBox::question(this, "File modified", file->location()+" has been modified externally.\n"
                                           + "Do you want to reload the file?",
                                           "Reload", "Cancel", QString(), 1, 1);
        } else {
            choice = QMessageBox::question(this, "File modified", file->location()+" has been modified externally.\n"
                                           + "Do you want to reload the file or keep your changes?",
                                           "Reload", "Keep changes", QString(), 1, 1);
        }
    }
    if (choice == 0) {
        file->load(file->codecMib());
    } else {
        file->document()->setModified();
    }
}

void MainWindow::fileDeletedExtern(FileId fileId)
{
    FileMeta *file = mFileMetaRepo.fileMeta(fileId);
    if (!file || !file->isOpen()) return;

    int ret = 0;
    if (!file->isReadOnly()) {
        // file is loaded: ASK, if it should be closed
        ret = QMessageBox::question(this, "File vanished", file->location()+" doesn't exist any more.\n"
                                    +"Keep file in editor?", "Keep", "Close", QString(), 1, 0);
    }
    if (ret == 1)
        closeFileEditors(fileId);
    else if (!file->isReadOnly())
        file->document()->setModified();
}

bool MainWindow::processIfRenamed(FileId fileId)
{
    // TODO(JM) Decide if we want this feature
    FileMeta *file = mFileMetaRepo.fileMeta(fileId);
    if (file->kind() == FileKind::Gdx) return false;
    QString newFileName;
    QDir dir = QFileInfo(file->location()).dir();
    if (!dir.exists()) return false;
    for (const QString &fileName : dir.entryList(QDir::Files)) {
         FileDifferences diff = file->compare(fileName);
         if (!diff.testFlag(FdTime) && !diff.testFlag(FdSize)) {
             newFileName = fileName;
             break;
         }
    }
    if (newFileName.isEmpty()) return false;

    QMessageBox msgBox;
    msgBox.setWindowTitle("File renamed");
    msgBox.setText(file->location()+" seems to be renamed to\n"+newFileName);
    msgBox.setInformativeText("Switch to the new filename?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::NoButton);
    int ret = msgBox.exec();

    if (ret == QMessageBox::No)
        file->document()->setModified();
    else {
        file->saveAs(newFileName);
        mSettings->saveSettings(this);
    }
    return true;
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
    else if (!fm->isOpen())
        fileChanged(e.fileId()); // Just update display kind
    else {
        // file handling with user-interaction are delayed

        FileEventData data = e.data();
        if (!mFileEvents.contains(data))
            mFileEvents << data;
//        for (ProjectFileNode* node : mProjectRepo.fileNodes(data.fileId))
//            mProjectRepo.update(node);
        mFileTimer.start();
    }
}

void MainWindow::processFileEvents()
{
    while (!mFileEvents.isEmpty()) {
        if (!isActiveWindow()) {
            mFileTimer.start();
            break;
        }
        FileEventData fileEvent = mFileEvents.takeFirst();
        FileMeta *fm = mFileMetaRepo.fileMeta(fileEvent.fileId);
        if (!fm) continue;
        switch (fileEvent.kind) {
        case FileEventKind::changedExtern:
            fileChangedExtern(fm->id());
            break;
        case FileEventKind::removedExtern:
            fileDeletedExtern(fm->id());
            break;
        default: break;
        }
    }
}

void MainWindow::appendSystemLog(const QString &text)
{
    mSyslog->appendLog(text, LogMsgType::Info);
}

void MainWindow::postGamsRun(NodeId origin)
{
    if (origin == -1) {
        mSyslog->appendLog("No fileId set to process", LogMsgType::Error);
        return;
    }
    // TODO(JM) Replace the FileId by NodeId in GamsProcess
    ProjectRunGroupNode* groupNode = mProjectRepo.findRunGroup(origin);
    if (!groupNode) {
        mSyslog->appendLog("No group attached to process", LogMsgType::Error);
        return;
    }
    FileMeta *runMeta = groupNode->runnableGms();
    if (!runMeta) {
        mSyslog->appendLog("Invalid runable attached to process", LogMsgType::Error);
        return;
    }
    if(groupNode && runMeta->exists(true)) {
        QString lstFile = groupNode->lstFile();
        bool doFocus = groupNode == mRecent.group;

        if (mSettings->jumpToError())
            groupNode->jumpToFirstError(doFocus);

        ProjectFileNode* lstNode = mProjectRepo.findOrCreateFileNode(lstFile, groupNode);

        if (lstNode) lstNode->enhanceMarksFromLst();

        if (mSettings->openLst())
            openFileNode(lstNode);
    }
    if (groupNode && groupNode->logNode())
        groupNode->logNode()->logDone();

    // add all created files to project explorer
    for (QString loc : groupNode->specialFiles().values())
        groupNode->findOrCreateFileNode(loc);
}

void MainWindow::postGamsLibRun()
{
    // TODO(AF) Are there models without a GMS file? How to handle them?"
    ProjectFileNode *node = mProjectRepo.findFile(mLibProcess->targetDir() + "/" + mLibProcess->inputFile());
    if (!node)
        node = addNode(mLibProcess->targetDir(), mLibProcess->inputFile());
    if (node && !node->file()->editors().isEmpty()) {
        if (node->file()->kind() != FileKind::Log)
            node->file()->load(node->file()->codecMib());
    }
    openFileNode(node); // this
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
    QWidget* widget = focusWidget();
    if (mGamsOptionWidget->isAnOptionWidgetFocused(widget)) {
        mHelpWidget->on_helpContentRequested(HelpWidget::GAMSCALL_CHAPTER, mGamsOptionWidget->getSelectedOptionName(widget));
    } else if ( (mRecent.editor() != nullptr) && (widget == mRecent.editor()) ) {
        CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
        if (ce) {
            QString word;
            int istate = 0;
            ce->wordInfo(ce->textCursor(), word, istate);

            if (istate == static_cast<int>(SyntaxState::Title)) {
                mHelpWidget->on_helpContentRequested(HelpWidget::DOLLARCONTROL_CHAPTER, "title");
            } else if (istate == static_cast<int>(SyntaxState::Directive)) {
                mHelpWidget->on_helpContentRequested(HelpWidget::DOLLARCONTROL_CHAPTER, word);
            } else {
                mHelpWidget->on_helpContentRequested(HelpWidget::INDEX_CHAPTER, word);
            }
        }
    }
    if (ui->dockHelpView->isHidden())
        ui->dockHelpView->show();
    if (tabifiedDockWidgets(ui->dockHelpView).count())
        ui->dockHelpView->raise();
}

QString MainWindow::studioInfo()
{
    QString ret = "Release: GAMS Studio " + QApplication::applicationVersion() + " ";
    ret += QString(sizeof(void*)==8 ? "64" : "32") + " bit<br/>";
    ret += "Build Date: " __DATE__ " " __TIME__ "<br/><br/>";

    return ret;
}

void MainWindow::on_actionAbout_triggered()
{
    QString about = "<b><big>GAMS Studio " + QApplication::applicationVersion() + "</big></b><br/><br/>";
    about += studioInfo();
    about += "Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com><br/>";
    about += "Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com><br/><br/>";
    about += "This program is free software: you can redistribute it and/or modify ";
    about += "it under the terms of the GNU General Public License as published by ";
    about += "the Free Software Foundation, either version 3 of the License, or ";
    about += "(at your option) any later version.<br/><br/>";
    about += "This program is distributed in the hope that it will be useful, ";
    about += "but WITHOUT ANY WARRANTY; without even the implied warranty of ";
    about += "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the ";
    about += "GNU General Public License for more details.<br/><br/>";
    about += "You should have received a copy of the GNU General Public License ";
    about += "along with this program. If not, see ";
    about += "<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>.<br/><br/>";
    about += "The source code of the program can be accessed at ";
    about += "<a href=\"https://github.com/GAMS-dev/studio\">https://github.com/GAMS-dev/studio/</a>.";
    about += "<br/><br/><b><big>GAMS Distribution ";
    about += CheckForUpdateWrapper::distribVersionString();
    about += "</big></b><br/><br/>";
    GamsProcess gproc;
    about += gproc.aboutGAMS().replace("\n", "<br/>");
    about += "<br/><br/>For further information about GAMS please visit ";
    about += "<a href=\"https://www.gams.com\">https://www.gams.com</a>.<br/>";

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setWindowTitle("About GAMS Studio");
    box.setText(about);
    box.setIconPixmap(QPixmap(":/img/gams-w24"));
    box.addButton("Close", QMessageBox::RejectRole);
    box.addButton("Copy product info", QMessageBox::AcceptRole);
    int answer = box.exec();

    if (answer) {
        QClipboard *clip = QGuiApplication::clipboard();
        clip->setText(studioInfo().replace("<br/>", "\n") + gproc.aboutGAMS());
    }
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::on_actionUpdate_triggered()
{
    UpdateDialog updateDialog(this);
    updateDialog.checkForUpdate();
    updateDialog.exec();
}

void MainWindow::on_mainTab_tabCloseRequested(int index)
{
    QWidget* widget = ui->mainTab->widget(index);
    FileMeta* fc = mFileMetaRepo.fileMeta(widget);
    if (!fc) {
        // assuming we are closing a welcome page here
        ui->mainTab->removeTab(index);
        mClosedTabs << "Wp Closed";
        mClosedTabsIndexes << index;
        return;
    }

    int ret = QMessageBox::Discard;
    if (fc->editors().size() == 1 && fc->isModified()) {
        // only ask, if this is the last editor of this file
        ret = showSaveChangesMsgBox(ui->mainTab->tabText(index)+" has been modified.");
    }

    if (ret == QMessageBox::Save) {
        mAutosaveHandler->clearAutosaveFiles(mOpenTabsList);
        fc->save();
        closeFileEditors(fc->id());
    } else if (ret == QMessageBox::Discard) {
        mAutosaveHandler->clearAutosaveFiles(mOpenTabsList);
        closeFileEditors(fc->id());
    }
    mClosedTabsIndexes << index;
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
    QWidget* edit = ui->logTabs->widget(index);
    if (edit) {
        FileMeta* log = mFileMetaRepo.fileMeta(edit);
        if (log) log->removeEditor(edit);
        ui->logTabs->removeTab(index);
        AbstractEdit* ed = FileMeta::toAbstractEdit(edit);
        if (ed) ed->setDocument(nullptr);

        // dont remove syslog
        if (edit != mSyslog)
            edit->deleteLater();
    }
}

void MainWindow::showWelcomePage()
{
    ui->mainTab->insertTab(0, mWp, QString("Welcome")); // always first position
    ui->mainTab->setCurrentIndex(0); // go to welcome page
}

bool MainWindow::isActiveTabRunnable()
{
    QWidget* editWidget = (ui->mainTab->currentIndex() < 0 ? nullptr : ui->mainTab->widget((ui->mainTab->currentIndex())) );
    if (editWidget) {
       FileMeta* fm = mFileMetaRepo.fileMeta(editWidget);
       if (!fm) { // assuming a welcome page here
           return false;
       } else {
           return true;
       }
    }
    return false;
}

bool MainWindow::isRecentGroupInRunningState()
{
    if (!mRecent.group) return false;
    ProjectRunGroupNode *runGroup = mRecent.group->assignedRunGroup();
    if (!runGroup) return false;
    return (runGroup->gamsProcessState() == QProcess::Running);
}

void MainWindow::on_actionShow_System_Log_triggered()
{
    int index = ui->logTabs->indexOf(mSyslog);
    if (index < 0)
        ui->logTabs->addTab(mSyslog, "System");
    else
        ui->logTabs->setCurrentIndex(index);
    mSyslog->raise();
    dockWidgetShow(ui->dockLogView, true);
}

void MainWindow::on_actionShow_Welcome_Page_triggered()
{
    showWelcomePage();
}

void MainWindow::triggerGamsLibFileCreation(LibraryItem *item)
{
    openModelFromLib(item->library()->glbFile(), item);
}

HistoryData *MainWindow::history()
{
    return mHistory;
}

void MainWindow::addToOpenedFiles(QString filePath)
{
    if (!QFileInfo(filePath).exists()) return;

    if (filePath.startsWith("[")) return; // invalid

    if (history()->lastOpenedFiles.size() >= mSettings->historySize())
        history()->lastOpenedFiles.removeLast();

    if (!history()->lastOpenedFiles.contains(filePath))
        history()->lastOpenedFiles.insert(0, filePath);
    else
        history()->lastOpenedFiles.move(history()->lastOpenedFiles.indexOf(filePath), 0);

    if(mWp) mWp->historyChanged(history());
}

void MainWindow::on_actionGAMS_Library_triggered()
{
    ModelDialog dialog(mSettings->userModelLibraryDir(), this);
    if(dialog.exec() == QDialog::Accepted)
    {
        QMessageBox msgBox;
        LibraryItem *item = dialog.selectedLibraryItem();
        QFileInfo fileInfo(item->files().first());
        QString gmsFileName = fileInfo.completeBaseName() + ".gms";
        QString gmsFilePath = mSettings->defaultWorkspace() + "/" + gmsFileName;
        QFile gmsFile(gmsFilePath);

        if (gmsFile.exists()) {
            FileMeta* fm = mFileMetaRepo.findOrCreateFileMeta(gmsFilePath);

            QMessageBox msgBox;
            msgBox.setWindowTitle("File already existing");

            msgBox.setText("The file you are trying to load already exists in your temporary working directory.");
            msgBox.setInformativeText("What do you want to do with the existing file?");
            msgBox.setStandardButtons(QMessageBox::Abort);
            msgBox.addButton("Open", QMessageBox::ActionRole);
            msgBox.addButton("Replace", QMessageBox::ActionRole);
            int answer = msgBox.exec();

            switch(answer) {
            case 0: // open
                openFileNode( addNode("", gmsFilePath) );
                break;
            case 1: // replace
                fm->renameToBackup();
                triggerGamsLibFileCreation(item);
                break;
            case QMessageBox::Abort:
                break;
            }
        } else {
            triggerGamsLibFileCreation(item);
        }
    }
}

void MainWindow::on_projectView_activated(const QModelIndex &index)
{
    ProjectAbstractNode* node = mProjectRepo.node(index);
    if ((node->type() == NodeType::group) || (node->type() == NodeType::runGroup)) {
        ProjectLogNode* logProc = mProjectRepo.logNode(node);
        openFileNode(logProc, true, logProc->file()->codecMib());
//        if (logProc->file()->editors().isEmpty()) {
//            ProcessLogEdit* logEdit = new ProcessLogEdit(this);
//            logEdit->setLineWrapMode(mSettings->lineWrapProcess() ? AbstractEdit::WidgetWidth : AbstractEdit::NoWrap);
//            FileMeta::initEditorType(logEdit);
//            int ind = ui->logTabs->addTab(logEdit, logProc->name(NameModifier::editState));
//            logProc->file()->addEditor(logEdit);
//            ui->logTabs->setCurrentIndex(ind);
//        }
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
    QString filesText = changedFiles.size()==1 ? changedFiles.first()->location() + " has been modified."
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
    mSettings->saveSettings(this);
    QVector<FileMeta*> oFiles = mFileMetaRepo.modifiedFiles();
    if (requestCloseChanged(oFiles)) {
        on_actionClose_All_triggered();
        closeHelpView();
        mTextMarkRepo.clear();
    } else {
        event->setAccepted(false);
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_0))
        updateFixedFonts(mSettings->fontFamily(), mSettings->fontSize());

    if (event->key() == Qt::Key_Escape) {
        mSearchDialog->hide();
        mSearchDialog->clearSearch();
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasUrls()) {
        e->setDropAction(Qt::CopyAction);
        e->accept();
    } else {
        e->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent* e)
{
    if (e->mimeData()->hasUrls()) {
        e->accept();
        QStringList pathList;
        for (QUrl url: e->mimeData()->urls()) {
            pathList << url.toLocalFile();
        }

        int answer;
        if(pathList.size() > 25) {
            QMessageBox msgBox;
            msgBox.setText("You are trying to open " + QString::number(pathList.size()) +
                           " files at once. Depending on the file sizes this may take a long time.");
            msgBox.setInformativeText("Do you want to continue?");
            msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Cancel);
            answer = msgBox.exec();

            if(answer != QMessageBox::Open) return;
        }
        openFiles(pathList);
    }
}

void MainWindow::openFiles(QStringList pathList)
{
    QStringList filesNotFound;
    for (QString fName: pathList) {
        QFileInfo fi(fName);
        if (fi.isFile())
            openFilePath(CommonPaths::absolutFilePath(fName));
        else
            filesNotFound.append(fName);
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
        Q_UNUSED(child);
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::customEvent(QEvent *event)
{
    QMainWindow::customEvent(event);
    if (event->type() == LineEditCompleteEvent::type())
        (static_cast<LineEditCompleteEvent*>(event))->complete();
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

OptionWidget *MainWindow::gamsOptionWidget() const
{
    return mGamsOptionWidget;
}

void MainWindow::execute(QString commandLineStr, ProjectFileNode* gmsFileNode)
{
    ProjectFileNode* fc = (gmsFileNode ? gmsFileNode : mProjectRepo.findFileNode(mRecent.editor()));
    ProjectRunGroupNode *runGroup = (fc ? fc->assignedRunGroup() : nullptr);
    if (!runGroup) {
        DEB() << "Nothing to be executed.";
        return;
    }

    runGroup->addRunParametersHistory( mGamsOptionWidget->getCurrentCommandLineData() );
    runGroup->clearLstErrorTexts();

    // gather open files and autosave or request to save
    QVector<FileMeta*> openFiles;
    for (ProjectFileNode *node: runGroup->listFiles(true)) {
        if (node->file()->isOpen() && !openFiles.contains(node->file()))
            openFiles << node->file();
    }
    bool doSave = !openFiles.isEmpty();

    if (doSave && !mSettings->autosaveOnRun()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        if (openFiles.size() > 1)
            msgBox.setText(openFiles.first()->location()+" has been modified.");
        else
            msgBox.setText(QString::number(openFiles.size())+" files have been modified.");
        msgBox.setInformativeText("Do you want to save your changes before running?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        QAbstractButton* discardButton = msgBox.addButton(tr("Discard Changes and Run"), QMessageBox::ResetRole);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Cancel) {
            return;
        } else if (msgBox.clickedButton() == discardButton) {
            for (FileMeta *file: openFiles)
                if (file->kind() != FileKind::Log)
                    file->load(file->codecMib());
            doSave = false;
        }
    }
    if (doSave) {
        for (FileMeta *file: openFiles) file->save();
    }

    // clear the TextMarks for this group
    QSet<TextMark::Type> markTypes;
    markTypes << TextMark::error << TextMark::link;
    for (ProjectFileNode *node: runGroup->listFiles(true))
        mTextMarkRepo.removeMarks(node->file()->id(), node->assignedRunGroup()->id(), markTypes);

    // prepare the log
    ProjectLogNode* logProc = mProjectRepo.logNode(runGroup);
    if (!logProc->file()->isOpen()) {
        QWidget *wid = logProc->file()->createEdit(ui->logTabs, logProc->assignedRunGroup(), QList<int>() << logProc->file()->codecMib());
        if (FileMeta::toCodeEdit(wid) || FileMeta::toLogEdit(wid))
            FileMeta::toAbstractEdit(wid)->setFont(QFont(mSettings->fontFamily(), mSettings->fontSize()));
        if (FileMeta::toAbstractEdit(wid))
            FileMeta::toAbstractEdit(wid)->setLineWrapMode(mSettings->lineWrapProcess() ? AbstractEdit::WidgetWidth
                                                                                        : AbstractEdit::NoWrap);
    }
    if (!mSettings->clearLog()) {
        logProc->markOld();
    } else {
        logProc->clearLog();
    }
    if (!ui->logTabs->children().contains(logProc->file()->editors().first())) {
        ui->logTabs->addTab(logProc->file()->editors().first(), logProc->name(NameModifier::editState));
    }
    ui->logTabs->setCurrentWidget(logProc->file()->editors().first());
    ui->dockLogView->setVisible(true);

    // select gms-file  and working dir to run
    QString gmsFilePath = (gmsFileNode ? gmsFileNode->location() : runGroup->specialFile(FileKind::Gms));
    if (gmsFilePath == "") {
        mSyslog->appendLog("No runnable GMS file found in group ["+runGroup->name()+"].", LogMsgType::Warning);
        ui->actionShow_System_Log->trigger(); // TODO: move this out of here, do on every append
        return;
    }
    if (gmsFileNode)
        logProc->file()->setCodecMib(fc->file()->codecMib());
    else {
        FileMeta *runMeta = mFileMetaRepo.fileMeta(gmsFilePath);
        ProjectFileNode *runNode = runGroup->findFile(runMeta);
        logProc->file()->setCodecMib(runNode ? runNode->file()->codecMib() : -1);
    }
    QString workDir = gmsFileNode ? QFileInfo(gmsFilePath).path() : runGroup->location();
    logProc->setJumpToLogEnd(true);

    // prepare the options and process and run it
    QList<OptionItem> itemList = mGamsOptionWidget->getGamsOptionTokenizer()->tokenize( commandLineStr );
    GamsProcess* process = runGroup->gamsProcess();
    process->setParameters(runGroup->analyzeParameters(gmsFilePath, itemList));
    process->setGroupId(runGroup->id());
    process->setWorkingDir(workDir);
    process->execute();

    connect(process, &GamsProcess::newStdChannelData, logProc, &ProjectLogNode::addProcessData, Qt::UniqueConnection);
    connect(process, &GamsProcess::finished, this, &MainWindow::postGamsRun, Qt::UniqueConnection);
    ui->dockLogView->raise();
}

void MainWindow::updateRunState()
{
    mGamsOptionWidget->updateRunState(isActiveTabRunnable(), isRecentGroupInRunningState());
}

HelpWidget *MainWindow::helpWidget() const
{
    return mHelpWidget;
}

void MainWindow::runGmsFile(ProjectFileNode *node)
{
    execute("", node);
}

void MainWindow::setMainGms(ProjectFileNode *node)
{
    ProjectRunGroupNode *runGroup = node->assignedRunGroup();
    if (runGroup) {
        runGroup->setRunnableGms(node->file());
        updateRunState();
    }
}

void MainWindow::commandLineHelpTriggered()
{
    mHelpWidget->on_helpContentRequested(HelpWidget::GAMSCALL_CHAPTER, "");
    if (ui->dockHelpView->isHidden())
        ui->dockHelpView->show();
    if (tabifiedDockWidgets(ui->dockHelpView).count())
        ui->dockHelpView->raise();
}

void MainWindow::optionRunChanged()
{
    if (isActiveTabRunnable() && !isRecentGroupInRunningState())
       on_actionRun_triggered();
}

void MainWindow::on_actionRun_triggered()
{
    execute( mGamsOptionWidget->on_runAction(RunActionState::Run) );
}

void MainWindow::on_actionRun_with_GDX_Creation_triggered()
{
    execute( mGamsOptionWidget->on_runAction(RunActionState::RunWithGDXCreation) );
}

void MainWindow::on_actionCompile_triggered()
{
    execute( mGamsOptionWidget->on_runAction(RunActionState::Compile) );
}

void MainWindow::on_actionCompile_with_GDX_Creation_triggered()
{
    execute( mGamsOptionWidget->on_runAction(RunActionState::CompileWithGDXCreation) );
}

void MainWindow::on_actionInterrupt_triggered()
{
    ProjectFileNode* node = mProjectRepo.findFileNode(mRecent.editor());
    ProjectRunGroupNode *group = (node ? node->assignedRunGroup() : nullptr);
    if (!group)
        return;
    mGamsOptionWidget->on_interruptAction();
    GamsProcess* process = group->gamsProcess();
    QtConcurrent::run(process, &GamsProcess::interrupt);
}

void MainWindow::on_actionStop_triggered()
{
    ProjectFileNode* node = mProjectRepo.findFileNode(mRecent.editor());
    ProjectRunGroupNode *group = (node ? node->assignedRunGroup() : nullptr);
    if (!group)
        return;
    mGamsOptionWidget->on_stopAction();
    GamsProcess* process = group->gamsProcess();
    QtConcurrent::run(process, &GamsProcess::stop);
}

void MainWindow::changeToLog(ProjectAbstractNode *node, bool createMissing)
{
    bool moveToEnd = false;
    ProjectLogNode* logNode = mProjectRepo.logNode(node);
    if (!logNode) return;
    if (createMissing) {
        moveToEnd = true;
        if (logNode->file()->isOpen()) {
            QWidget *wid = logNode->file()->createEdit(ui->logTabs, logNode->assignedRunGroup(), QList<int>() << logNode->file()->codecMib());
            wid->setFont(QFont(mSettings->fontFamily(), mSettings->fontSize()));
            if (FileMeta::toAbstractEdit(wid))
                FileMeta::toAbstractEdit(wid)->setLineWrapMode(mSettings->lineWrapProcess() ? AbstractEdit::WidgetWidth
                                                                                            : AbstractEdit::NoWrap);
        }
    }
    if (logNode->file()->isOpen()) {
        ProcessLogEdit* logEdit = FileMeta::toLogEdit(logNode->file()->editors().first());
        if (logEdit) {
            if (ui->logTabs->currentWidget() != logEdit) {
                if (ui->logTabs->currentWidget() != mResultsView)
                    ui->logTabs->setCurrentWidget(logEdit);
            }
            if (moveToEnd) {
                QTextCursor cursor = logEdit->textCursor();
                cursor.movePosition(QTextCursor::End);
                logEdit->setTextCursor(cursor);
            }
        }
    }
}

void MainWindow::storeTree()
{
    // TODO(JM) add settings methods to store each part separately
    mSettings->saveSettings(this);
}

void MainWindow::openFile(FileMeta* fileMeta, bool focus, ProjectRunGroupNode *runGroup, int codecMib)
{
    if (!fileMeta) return;
    QWidget* edit = nullptr;
    QTabWidget* tabWidget = fileMeta->kind() == FileKind::Log ? ui->logTabs : ui->mainTab;
    if (!fileMeta->editors().empty()) {
        edit = fileMeta->editors().first();
    }
    // open edit if existing or create one
    if (edit) {
        if (runGroup) {
            if (AbstractEdit *ae = FileMeta::toAbstractEdit(edit)) {
                ae->setGroupId(runGroup->id());
            }
            if (gdxviewer::GdxViewer *gv = FileMeta::toGdxViewer(edit)) {
                gv->setGroupId(runGroup->id());
            }
        }
        if (focus) {
            tabWidget->setCurrentWidget(edit);
            if (tabWidget == ui->mainTab) {
                on_mainTab_currentChanged(tabWidget->indexOf(edit));
            }
        }
    } else {
        QWidget *edit = fileMeta->createEdit(tabWidget, runGroup, QList<int>() << codecMib);
        if (!edit) {
            DEB() << "Error: could nor create editor for '" << fileMeta->location() << "'";
            return;
        }
        if (FileMeta::toCodeEdit(edit) || FileMeta::toLogEdit(edit))
            FileMeta::toAbstractEdit(edit)->setFont(QFont(mSettings->fontFamily(), mSettings->fontSize()));

        connect(fileMeta, &FileMeta::changed, this, &MainWindow::fileChanged, Qt::UniqueConnection);
        if (focus) {
            tabWidget->setCurrentWidget(edit);
            updateMenuToCodec(fileMeta->codecMib());
            if (tabWidget == ui->mainTab) {
                mRecent.setEditor(tabWidget->currentWidget(), this);
                mRecent.editFileId = fileMeta->id();
            }
        }
        if (fileMeta->kind() == FileKind::Ref) {
            reference::ReferenceViewer *refView = FileMeta::toReferenceViewer(edit);
            connect(refView, &reference::ReferenceViewer::jumpTo, this, &MainWindow::on_referenceJumpTo);
        }
    }
    // set keyboard focus to editor
    if (tabWidget->currentWidget())
        if (focus) {
            lxiviewer::LxiViewer* lxiViewer = FileMeta::toLxiViewer(edit);
            if (lxiViewer)
                lxiViewer->codeEdit()->setFocus();
            else
                tabWidget->currentWidget()->setFocus();
            if (runGroup) ui->projectView->expand(mProjectRepo.treeModel()->index(runGroup));
            mGamsOptionWidget->loadCommandLineOption( runGroup->getRunParametersHistory() );
        }
    if (tabWidget != ui->logTabs) {
        // if there is already a log -> show it
        ProjectFileNode* fileNode = mProjectRepo.findFileNode(edit);
        changeToLog(fileNode);
        mRecent.setEditor(tabWidget->currentWidget(), this);
        mRecent.editFileId = fileMeta->id();
        mRecent.path = fileMeta->location();
        mRecent.group = runGroup;
    }
    addToOpenedFiles(fileMeta->location());
}

void MainWindow::openFileNode(ProjectFileNode *node, bool focus, int codecMib)
{
    if (!node) return;
    openFile(node->file(), focus, node->assignedRunGroup(), codecMib);
}

void MainWindow::closeGroup(ProjectGroupNode* group)
{
    if (!group) return;
    ProjectRunGroupNode *runGroup = group->assignedRunGroup();
    QVector<FileMeta*> changedFiles;
    QVector<FileMeta*> openFiles;
    for (ProjectFileNode *node: group->listFiles(true)) {
        if (node->isModified()) changedFiles << node->file();
        if (node->file()->isOpen()) openFiles << node->file();
    }

    if (requestCloseChanged(changedFiles)) {
        // TODO(JM)  close if selected
        for (FileMeta *file: openFiles) {
            closeFileEditors(file->id());
        }
        ProjectLogNode* log = runGroup ? runGroup->logNode() : nullptr;
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
}

/// Asks user for confirmation if a file is modified before calling closeFile
/// \param file
///
void MainWindow::closeNodeConditionally(ProjectFileNode* node)
{
    QVector<ProjectFileNode*> fiNodes = mProjectRepo.fileNodes(node->file()->id());
    fiNodes.removeAll(node);
    // not the last OR not modified OR permitted
    if (!fiNodes.isEmpty() || !node->isModified() || requestCloseChanged(QVector<FileMeta*>() << node->file())) {
        closeFileEditors(node->file()->id());
        mProjectRepo.closeNode(node);
    }
}

/// Closes all open editors and tabs related to a file and remove option history
/// \param fileId
///
void MainWindow::closeFileEditors(FileId fileId)
{
    FileMeta* fm = mFileMetaRepo.fileMeta(fileId);
    if (!fm) return; // TODO(AF) add logging but no execption

    // add to recently closed tabs
    mClosedTabs << fm->location();

    // close all related editors, tabs and clean up
    while (!fm->editors().isEmpty()) {
        QWidget *edit = fm->editors().first();
        ui->mainTab->removeTab(ui->mainTab->indexOf(edit));
        fm->removeEditor(edit);
        edit->deleteLater();
    }
}

void MainWindow::openFilePath(const QString &filePath, bool focus, int codecMib)
{
    if (!QFileInfo(filePath).exists()) {
        EXCEPT() << "File not found: " << filePath;
    }
    ProjectFileNode *fileNode = mProjectRepo.findFile(filePath);

    if (!fileNode) {
        fileNode = mProjectRepo.findOrCreateFileNode(filePath);
        if (!fileNode) {
            EXCEPT() << "Could not create node for file: " << filePath;
        }
    }
    openFileNode(fileNode, focus, codecMib);
}

ProjectFileNode* MainWindow::addNode(const QString &path, const QString &fileName)
{
    ProjectFileNode *node = nullptr;
    if (!fileName.isEmpty()) {
        QFileInfo fInfo(path, fileName);

        FileType fType = FileType::from(fInfo.suffix());

        if (fType == FileKind::Gsp) {
            // TODO(JM) Read project and create all nodes for associated files
        } else {
            node = mProjectRepo.findOrCreateFileNode(fInfo.absoluteFilePath());
        }
    }
    return node;
}

void MainWindow::on_referenceJumpTo(reference::ReferenceItem item)
{
    QFileInfo fi(item.location);
    if (fi.isFile()) {
        openFilePath(fi.absoluteFilePath(), true);
        CodeEdit *codeEdit = FileMeta::toCodeEdit(mRecent.editor());
        if (codeEdit) {
            int line = (item.lineNumber > 0 ? item.lineNumber-1 : 0);
            int column = (item.columnNumber > 0 ? item.columnNumber-1 : 0);
            codeEdit->jumpTo(line, column);
        }
    }
}

void MainWindow::on_mainTab_currentChanged(int index)
{
    QWidget* edit = ui->mainTab->widget(index);
    if (!edit) return;

    mProjectRepo.editorActivated(edit);
    ProjectFileNode* fc = mProjectRepo.findFileNode(edit);
    if (fc && mRecent.group != fc->parentNode()) {
        mRecent.group = fc->parentNode();
        updateRunState();
    }
    changeToLog(fc);

    CodeEdit* ce = FileMeta::toCodeEdit(edit);
    if (ce) ce->updateExtraSelections();
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog sd(this);
    connect(&sd, &SettingsDialog::editorFontChanged, this, &MainWindow::updateFixedFonts);
    connect(&sd, &SettingsDialog::editorLineWrappingChanged, this, &MainWindow::updateEditorLineWrapping);
    sd.exec();
    sd.disconnect();
    mSettings->saveSettings(this);
}

void MainWindow::on_actionSearch_triggered()
{
    if (ui->dockHelpView->isAncestorOf(QApplication::focusWidget()) ||
        ui->dockHelpView->isAncestorOf(QApplication::activeWindow())) {
        mHelpWidget->on_searchHelp();
    } else {
       ProjectFileNode *fc = mProjectRepo.findFileNode(mRecent.editor());
       if (fc && fc->file()->kind() == FileKind::Gdx) {
           gdxviewer::GdxViewer *gdx = FileMeta::toGdxViewer(mRecent.editor());
           gdx->selectSearchField();
           return;
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
           QPoint p(0,0);
           QPoint newP(this->mapToGlobal(p));

           if (ui->mainTab->currentWidget())
               mSearchDialog->move(newP.x(), newP.y());

           mSearchDialog->show();
       }
    }
}

void MainWindow::showResults(SearchResultList &results)
{
    int index = ui->logTabs->indexOf(mResultsView); // did widget exist before?

    mResultsView = new ResultsView(results, this);
    QString title("Results: " + mSearchDialog->searchTerm() + " (" + QString::number(results.size()) + ")");

    ui->dockLogView->show();
    mResultsView->resizeColumnsToContent();

    if (index != -1) ui->logTabs->removeTab(index); // remove old result page

    ui->logTabs->addTab(mResultsView, title); // add new result page
    ui->logTabs->setCurrentWidget(mResultsView);
}

void MainWindow::closeResults()
{
    int index = ui->logTabs->indexOf(mResultsView);
    if (index != -1) ui->logTabs->removeTab(index);
}

void MainWindow::updateFixedFonts(const QString &fontFamily, int fontSize)
{
    QFont font(fontFamily, fontSize);
    for (QWidget* edit: openEditors()) {
        if (FileMeta::toCodeEdit(edit) || FileMeta::toLogEdit(edit))
            FileMeta::toAbstractEdit(edit)->setFont(font);
    }
    for (QWidget* log: openLogs())
        log->setFont(font);

    mSyslog->setFont(font);
}

void MainWindow::updateEditorLineWrapping()
{// TODO(AF) split logs and editors
    QPlainTextEdit::LineWrapMode wrapModeEditor = mSettings->lineWrapEditor() ? QPlainTextEdit::WidgetWidth
                                                                              : QPlainTextEdit::NoWrap;
    QPlainTextEdit::LineWrapMode wrapModeProcess = mSettings->lineWrapProcess() ? QPlainTextEdit::WidgetWidth
                                                                                  : QPlainTextEdit::NoWrap;
    QWidgetList editList = mFileMetaRepo.editors();
    for (int i = 0; i < editList.size(); i++) {
        AbstractEdit* ed = FileMeta::toAbstractEdit(editList.at(i));
        if (ed) {
            ed->blockCountChanged(0); // force redraw for line number area
            ed->setLineWrapMode(FileMeta::toLogEdit(ed) ? wrapModeProcess : wrapModeEditor);
        }
    }

}

void MainWindow::readTabs(const QJsonObject &json)
{
    if (json.contains("mainTabs") && json["mainTabs"].isArray()) {
        QJsonArray tabArray = json["mainTabs"].toArray();
        for (int i = 0; i < tabArray.size(); ++i) {
            QJsonObject tabObject = tabArray[i].toObject();
            if (tabObject.contains("location")) {
                QString location = tabObject["location"].toString();
                int mib = tabObject.contains("codecMib") ? tabObject["codecMib"].toInt() : -1;
                if (QFileInfo(location).exists()) {
                    openFilePath(location, true, mib);
                    mOpenTabsList << location;
                }
                QApplication::processEvents();
            }
        }
    }
    if (json.contains("mainTabRecent")) {
        QString location = json["mainTabRecent"].toString();
        if (QFileInfo(location).exists()) {
            openFilePath(location);
            mOpenTabsList << location;
        } else if (location == "WELCOME_PAGE") {
            showWelcomePage();
        }
    }
    QTimer::singleShot(0, this, SLOT(initAutoSave()));
}

void MainWindow::initAutoSave()
{
    mAutosaveHandler->recoverAutosaveFiles(mAutosaveHandler->checkForAutosaveFiles(mOpenTabsList));
}

void MainWindow::writeTabs(QJsonObject &json) const
{
    QJsonArray tabArray;
    for (int i = 0; i < ui->mainTab->count(); ++i) {
        QWidget *wid = ui->mainTab->widget(i);
        if (!wid || wid == mWp) continue;
        FileMeta *fm = mFileMetaRepo.fileMeta(wid);
        if (!fm) continue;
        QJsonObject tabObject;
        tabObject["location"] = fm->location();
        tabObject["codecMib"] = fm->codecMib();
        // TODO(JM) store current edit position
        tabArray.append(tabObject);
    }
    json["mainTabs"] = tabArray;

    FileMeta *fm = mRecent.editor() ? mFileMetaRepo.fileMeta(mRecent.editor()) : nullptr;
    if (fm)
        json["mainTabRecent"] = fm->location();
    else if (ui->mainTab->currentWidget() == mWp)
        json["mainTabRecent"] = "WELCOME_PAGE";
}

void MainWindow::on_actionGo_To_triggered()
{
    if ((ui->mainTab->currentWidget() == mWp) || (mRecent.editor() == nullptr))
        return;
    GoToDialog dialog(this);
    int result = dialog.exec();
    if (QDialog::Rejected == result)
        return;
    CodeEdit *codeEdit = FileMeta::toCodeEdit(mRecent.editor());
    if (codeEdit)
        codeEdit->jumpTo(dialog.lineNumber());
}

void MainWindow::on_actionRedo_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce) ce->extendedRedo();
}

void MainWindow::on_actionUndo_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce) ce->extendedUndo();
}

void MainWindow::on_actionPaste_triggered()
{
    CodeEdit *ce = FileMeta::toCodeEdit(focusWidget());
    if (!ce || ce->isReadOnly()) return;
    ce->pasteClipboard();
}

void MainWindow::on_actionCopy_triggered()
{
    if (!focusWidget()) return;

    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editor());
    if (!fm) return;

    if (fm->kind() == FileKind::Gdx) {
        gdxviewer::GdxViewer *gdx = FileMeta::toGdxViewer(mRecent.editor());
        gdx->copyAction();
    } else if (focusWidget() == mSyslog) {
        mSyslog->copy();
    } else {
        AbstractEdit *ae = FileMeta::toAbstractEdit(focusWidget());
        if (!ae) return;
        CodeEdit *ce = FileMeta::toCodeEdit(ae);
        if (ce) {
            ce->copySelection();
        } else {
            ae->copy();
        }
    }
}

void MainWindow::on_actionSelect_All_triggered()
{
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editor());
    if (!fm || !focusWidget()) return;

    if (fm->kind() == FileKind::Gdx) {
        gdxviewer::GdxViewer *gdx = FileMeta::toGdxViewer(mRecent.editor());
        gdx->selectAllAction();
    } else if (focusWidget() == mSyslog) {
        mSyslog->selectAll();
    } else {
        AbstractEdit *ae = FileMeta::toAbstractEdit(focusWidget());
        if (!ae) return;
        ae->selectAll();
    }
}

void MainWindow::on_actionCut_triggered()
{
    CodeEdit* ce= FileMeta::toCodeEdit(focusWidget());
    if (!ce || ce->isReadOnly()) return;
    ce->cutSelection();
}

void MainWindow::on_actionReset_Zoom_triggered()
{
    if (helpWidget()->isAncestorOf(QApplication::focusWidget()) ||
        helpWidget()->isAncestorOf(QApplication::activeWindow())) {
        helpWidget()->resetZoom(); // reset help view
    } else {
        updateFixedFonts(mSettings->fontFamily(), mSettings->fontSize()); // reset all editors
    }

}

void MainWindow::on_actionZoom_Out_triggered()
{
    if (helpWidget()->isAncestorOf(QApplication::focusWidget()) ||
        helpWidget()->isAncestorOf(QApplication::activeWindow())) {
        helpWidget()->zoomOut();
    } else {
        AbstractEdit *ae = FileMeta::toAbstractEdit(QApplication::focusWidget());
        if (ae) {
            int pix = ae->fontInfo().pixelSize();
            if (pix == ae->fontInfo().pixelSize()) ae->zoomOut();
        }
    }
}

void MainWindow::on_actionZoom_In_triggered()
{
    if (helpWidget()->isAncestorOf(QApplication::focusWidget()) ||
        helpWidget()->isAncestorOf(QApplication::activeWindow())) {
        helpWidget()->zoomIn();
    } else {
        AbstractEdit *ae = FileMeta::toAbstractEdit(QApplication::focusWidget());
        if (ae) {
            int pix = ae->fontInfo().pixelSize();
            ae->zoomIn();
            if (pix == ae->fontInfo().pixelSize() && ae->fontInfo().pointSize() > 1) ae->zoomIn();
        }
    }
}

void MainWindow::on_actionSet_to_Uppercase_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce= FileMeta::toCodeEdit(mRecent.editor());
    if (ce) ce->convertToUpper();
}

void MainWindow::on_actionSet_to_Lowercase_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce) ce->convertToLower();
}

void MainWindow::on_actionOverwrite_Mode_toggled(bool overwriteMode)
{
    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
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

    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (!ce || ce->isReadOnly()) return;
    QPoint pos(-1,-1); QPoint anc(-1,-1);
    ce->getPositionAndAnchor(pos, anc);
    ce->indent(mSettings->tabSize(), pos.y()-1, anc.y()-1);
}

void MainWindow::on_actionOutdent_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (!ce || ce->isReadOnly()) return;
    QPoint pos(-1,-1); QPoint anc(-1,-1);
    ce->getPositionAndAnchor(pos, anc);
    ce->indent(-mSettings->tabSize(), pos.y()-1, anc.y()-1);
}

void MainWindow::on_actionDuplicate_Line_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly())
        ce->duplicateLine();
}

void MainWindow::on_actionRemove_Line_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly())
        ce->removeLine();
}

void MainWindow::on_actionComment_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly())
        ce->commentLine();
}

void MainWindow::toggleDebugMode()
{
    mDebugMode = !mDebugMode;
    mProjectRepo.setDebugMode(mDebugMode);
}

void MainWindow::on_actionRestore_Recently_Closed_Tab_triggered()
{
    // TODO: remove duplicates?
    if (mClosedTabs.isEmpty())
        return;

    if (mClosedTabs.last()=="Wp Closed") {
        mClosedTabs.removeLast();
        mClosedTabsIndexes.removeLast();
        showWelcomePage();
        return;
    }
    QFile file(mClosedTabs.last());
    mClosedTabs.removeLast();
    if (file.exists()) {
        openFilePath(file.fileName());
        ui->mainTab->tabBar()->moveTab(ui->mainTab->currentIndex(), mClosedTabsIndexes.takeLast());
    } else
        on_actionRestore_Recently_Closed_Tab_triggered();
}

void MainWindow::on_actionSelect_encodings_triggered()
{
    SelectEncodings se(encodingMIBs(), this);
    se.exec();
    setEncodingMIBs(se.selectedMibs());
    mSettings->saveSettings(this);
}

QWidget *RecentData::editor() const
{
    return mEditor;
}

void RecentData::setEditor(QWidget *editor, MainWindow* window)
{
    AbstractEdit* edit = FileMeta::toAbstractEdit(mEditor);
    if (edit) {
        MainWindow::disconnect(edit, &AbstractEdit::cursorPositionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::disconnect(edit, &AbstractEdit::selectionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::disconnect(edit, &AbstractEdit::blockCountChanged, window, &MainWindow::updateEditorBlockCount);
        MainWindow::disconnect(edit->document(), &QTextDocument::contentsChange, window, &MainWindow::currentDocumentChanged);
    }
    window->searchDialog()->setActiveEditWidget(nullptr);
    mEditor = editor;
    edit = FileMeta::toAbstractEdit(mEditor);
    if (edit) {
        MainWindow::connect(edit, &AbstractEdit::cursorPositionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::connect(edit, &AbstractEdit::selectionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::connect(edit, &AbstractEdit::blockCountChanged, window, &MainWindow::updateEditorBlockCount);
        MainWindow::connect(edit->document(), &QTextDocument::contentsChange, window, &MainWindow::currentDocumentChanged);
        window->searchDialog()->setActiveEditWidget(edit);
    }
    window->searchDialog()->invalidateCache();
    window->updateEditorMode();
    window->updateEditorPos();
}

void MainWindow::on_actionReset_Views_triggered()
{
    resetViews();
}

void MainWindow::resetViews()
{
    setWindowState(Qt::WindowNoState);
    mSettings->resetViewSettings();
    mSettings->loadSettings(this);

    QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
    for (QDockWidget* dock: dockWidgets) {
        dock->setFloating(false);
        dock->setVisible(true);

        if (dock == ui->dockProjectView) {
            addDockWidget(Qt::LeftDockWidgetArea, dock);
            resizeDocks(QList<QDockWidget*>() << dock, {width()/6}, Qt::Horizontal);
        } else if (dock == ui->dockLogView) {
            addDockWidget(Qt::RightDockWidgetArea, dock);
            resizeDocks(QList<QDockWidget*>() << dock, {width()/3}, Qt::Horizontal);
        } else if (dock == ui->dockHelpView) {
            dock->setVisible(false);
            addDockWidget(Qt::RightDockWidgetArea, dock);
            resizeDocks(QList<QDockWidget*>() << dock, {width()/3}, Qt::Horizontal);
        } else if (dock == ui->dockOptionEditor) {
            addDockWidget(Qt::TopDockWidgetArea, dock);
        }
    }
}

void MainWindow::resizeOptionEditor(const QSize &size)
{
    mGamsOptionWidget->resize( size );
    this->resizeDocks({ui->dockOptionEditor}, {size.height()}, Qt::Vertical);
}


void MainWindow::setForeground()
{
#if defined (WIN32)
   HWND WinId= HWND(winId());
   if (this->windowState() == Qt::WindowMinimized) {
       this->setWindowState(Qt::WindowActive);
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
    if (mSettings->foregroundOnDemand())
        setForeground();
}
  
void MainWindow::on_actionNextTab_triggered()
{
    QWidget *wid = focusWidget();
    QTabWidget *tabs = nullptr;
    while (wid) {
        if (wid == ui->mainTab) {
           tabs = ui->mainTab;
           break;
        }
        if (wid == ui->logTabs) {
           tabs = ui->logTabs;
           break;
        }
        wid = wid->parentWidget();
    }
    if (tabs) tabs->setCurrentIndex((tabs->count() + tabs->currentIndex() + 1) % tabs->count());
}

void MainWindow::on_actionPreviousTab_triggered()
{
    QWidget *wid = focusWidget();
    QTabWidget *tabs = nullptr;
    while (wid) {
        if (wid == ui->mainTab) {
           tabs = ui->mainTab;
           break;
        }
        if (wid == ui->logTabs) {
           tabs = ui->logTabs;
           break;
        }
        wid = wid->parentWidget();
    }
    if (tabs) tabs->setCurrentIndex((tabs->count() + tabs->currentIndex() - 1) % tabs->count());
}

}
}


