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
#include "editors/codeeditor.h"
#include "welcomepage.h"
#include "modeldialog/modeldialog.h"
#include "exception.h"
#include "treeitemdelegate.h"
#include "commonpaths.h"
#include "gamsprocess.h"
#include "gamslibprocess.h"
#include "lxiviewer/lxiviewer.h"
#include "gdxviewer/gdxviewer.h"
#include "logger.h"
#include "studiosettings.h"
#include "settingsdialog.h"
#include "searchwidget.h"
#include "option/optioneditor.h"
#include "option/commandlinehistory.h"
#include "searchresultlist.h"
#include "resultsview.h"
#include "gotowidget.h"
#include "editors/logeditor.h"
#include "editors/abstracteditor.h"
#include "editors/selectencodings.h"
#include "updatedialog.h"
#include "checkforupdatewrapper.h"
#include "autosavehandler.h"
#include "distributionvalidator.h"
#include "syntax/textmarkrepo.h"
#include "helpview.h"
#include "statuswidgets.h"

namespace gams {
namespace studio {

MainWindow::MainWindow(StudioSettings *settings, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      mSettings(settings),
      mAutosaveHandler(new AutosaveHandler(this)),
      mProjectRepo(this),
      mFileMetaRepo(this, settings),
      mTextMarkRepo(&mFileMetaRepo, &mProjectRepo, this)
{
    mHistory = new HistoryData();
    QFile css(":/data/style.css");
    if (css.open(QFile::ReadOnly | QFile::Text)) {
//        this->setStyleSheet(css.readAll());
    }

    ui->setupUi(this);

    setAcceptDrops(true);
    TimerID = startTimer(60000);
    QFont font = ui->statusBar->font();
    font.setPointSizeF(font.pointSizeF()*0.9);
    ui->statusBar->setFont(font);
    mStatusWidgets = new StatusWidgets(this);
    int iconSize = fontInfo().pixelSize()*2-1;
    ui->projectView->setModel(mProjectRepo.treeModel());
    ui->projectView->setRootIndex(mProjectRepo.treeModel()->rootModelIndex());
//    mProjectRepo.setSuffixFilter(QStringList() << ".gms" << ".lst" << ".gdx");
    ui->projectView->setHeaderHidden(true);
    ui->projectView->setItemDelegate(new TreeItemDelegate(ui->projectView));
    ui->projectView->setIconSize(QSize(iconSize*0.8,iconSize*0.8));
    ui->systemLogView->setTextInteractionFlags(ui->systemLogView->textInteractionFlags() | Qt::TextSelectableByKeyboard);
    ui->projectView->setContextMenuPolicy(Qt::CustomContextMenu);

//    mTextMarkRepo = new TextMarkRepo(&mProjectRepo, this);
    mProjectRepo.init(&mFileMetaRepo, &mTextMarkRepo);
    mFileMetaRepo.init(&mTextMarkRepo, &mProjectRepo);

    // TODO(JM) it is possible to put the QTabBar into the docks title:
    //          if we override the QTabWidget it should be possible to extend it over the old tab-bar-space
//    ui->dockLogView->setTitleBarWidget(ui->tabLog->tabBar());

    mDockHelpView = new HelpView(this);
    this->addDockWidget(Qt::RightDockWidgetArea, mDockHelpView);
    connect(mDockHelpView, &HelpView::visibilityChanged, this, &MainWindow::helpViewVisibilityChanged);
    mDockHelpView->hide();

    createRunAndCommandLineWidgets();

    mCodecGroupReload = new QActionGroup(this);
    connect(mCodecGroupReload, &QActionGroup::triggered, this, &MainWindow::codecReload);
    mCodecGroupSwitch = new QActionGroup(this);
    connect(mCodecGroupSwitch, &QActionGroup::triggered, this, &MainWindow::codecChanged);
    connect(ui->mainTab, &QTabWidget::currentChanged, this, &MainWindow::activeTabChanged);


    // TODO(JM) mFileMetaRepo should signal just one FileEvent with the kind of change. That makes it easier to stack
    //          the events to be triggered later if Studio gains the focus again
    connect(&mFileMetaRepo, &FileMetaRepo::fileEvent, this, &MainWindow::fileEvent);
//    connect(&mFileMetaRepo, &FileMetaRepo::fileClosed, this, &MainWindow::fileClosed);
//    connect(&mProjectRepo, &ProjectRepo::fileChangedExtern, this, &MainWindow::fileChangedExtern);
//    connect(&mProjectRepo, &ProjectRepo::fileDeletedExtern, this, &MainWindow::fileDeletedExtern);


    connect(&mProjectRepo, &ProjectRepo::openFile, this, &MainWindow::openFile);
    connect(&mProjectRepo, &ProjectRepo::setNodeExpanded, this, &MainWindow::setProjectNodeExpanded);
    connect(&mProjectRepo, &ProjectRepo::isNodeExpanded, this, &MainWindow::isProjectNodeExpanded);
    connect(&mProjectRepo, &ProjectRepo::gamsProcessStateChanged, this, &MainWindow::gamsProcessStateChanged);
    connect(ui->projectView->selectionModel(), &QItemSelectionModel::currentChanged, &mProjectRepo, &ProjectRepo::setSelected);

    connect(ui->projectView, &QTreeView::customContextMenuRequested, this, &MainWindow::projectContextMenuRequested);
    connect(&mProjectContextMenu, &ProjectContextMenu::closeGroup, this, &MainWindow::closeGroup);
    connect(&mProjectContextMenu, &ProjectContextMenu::closeFile, this, &MainWindow::closeFile);
    connect(&mProjectContextMenu, &ProjectContextMenu::addExistingFile, this, &MainWindow::addToGroup);
    connect(&mProjectContextMenu, &ProjectContextMenu::getSourcePath, this, &MainWindow::sendSourcePath);
    connect(&mProjectContextMenu, &ProjectContextMenu::runFile, this, &MainWindow::on_runGmsFile);
    connect(&mProjectContextMenu, &ProjectContextMenu::setMainFile, this, &MainWindow::on_setMainGms);
    connect(ui->dockProjectView, &QDockWidget::visibilityChanged, this, &MainWindow::projectViewVisibiltyChanged);
    connect(ui->dockLogView, &QDockWidget::visibilityChanged, this, &MainWindow::outputViewVisibiltyChanged);

    setEncodingMIBs(encodingMIBs());
    ui->menuEncoding->setEnabled(false);
    mSettings->loadSettings(this);
    mRecent.path = mSettings->defaultWorkspace();
    mSearchWidget = new SearchWidget(this);
    mGoto= new GoToWidget(this);

    if (mSettings.get()->resetSettingsSwitch()) mSettings.get()->resetSettings();

    if (mSettings->lineWrapProcess()) // set wrapping for system log
        ui->systemLogView->setLineWrapMode(AbstractEditor::WidgetWidth);
    else
        ui->systemLogView->setLineWrapMode(AbstractEditor::NoWrap);

    initTabs();

    connectCommandLineWidgets();

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F12), this, SLOT(toggleLogDebug()));

}

void MainWindow::delayedFileRestoration()
{
    mSettings->restoreTabsAndProjects(this);
    mSettings.get()->restoreLastFilesUsed(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    killTimer(TimerID);
    // TODO(JM) The delete ui deletes all child instances in the tree. If you want to remove instances that may or
    //          may not be in the ui, delete and remove them from ui before the ui is deleted.
    delete mGamsOption;
    FileType::clear();
}

void MainWindow::initTabs()
{
    QPalette pal = ui->projectView->palette();
    pal.setColor(QPalette::Highlight, Qt::transparent);
    ui->projectView->setPalette(pal);

    if (!mSettings->skipWelcomePage())
        createWelcomePage();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    mAutosaveHandler->saveChangedFiles();
    mSettings->saveSettings(this);
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
    visibility = visibility || tabifiedDockWidgets(ui->dockLogView).count();
    ui->actionOutput_View->setChecked(visibility);
}

void MainWindow::setProjectViewVisibility(bool visibility)
{
    visibility = visibility || tabifiedDockWidgets(ui->dockProjectView).count();
    ui->actionProject_View->setChecked(visibility);
}

void MainWindow::setOptionEditorVisibility(bool visibility)
{
    visibility = visibility || tabifiedDockWidgets(mDockOptionView).count();
    ui->actionOption_View->setChecked(visibility);
}

void MainWindow::setHelpViewVisibility(bool visibility)
{
    visibility = visibility || tabifiedDockWidgets(mDockHelpView).count();
    if (!visibility)
        getDockHelpView()->clearSearchBar();
    else
        getDockHelpView()->setFocus();
    ui->actionHelp_View->setChecked(visibility);
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
    dockWidgetShow(mDockOptionView, checked);
    if(!checked) mDockOptionView->setFloating(false);
}

void MainWindow::on_actionHelp_View_triggered(bool checked)
{
    dockWidgetShow(mDockHelpView, checked);
}

void MainWindow::setCommandLineHistory(CommandLineHistory *opt)
{
    mCommandLineHistory = opt;
}

void MainWindow::checkOptionDefinition(bool checked)
{
    mShowOptionDefintionCheckBox->setChecked(checked);
    toggleOptionDefinition(checked);
}

bool MainWindow::isOptionDefinitionChecked()
{
    return mShowOptionDefintionCheckBox->isChecked();
}

CommandLineHistory *MainWindow::commandLineHistory()
{
    return mCommandLineHistory;
}

FileMetaRepo *MainWindow::fileRepo()
{
    return &mFileMetaRepo;
}

ProjectRepo *MainWindow::projectRepo()
{
    return &mProjectRepo;
}

QWidgetList MainWindow::openEditors()
{
    QWidgetList res;
    for (int i = 0; i < ui->mainTab->count(); ++i) {
        res << ui->mainTab->widget(i);
    }
    return res;
}

QList<AbstractEditor*> MainWindow::openLogs()
{
    QList<AbstractEditor*> resList;
    for (int i = 0; i < ui->logTabs->count(); i++) {
        AbstractEditor* ed = FileMeta::toAbstractEdit(ui->logTabs->widget(i));
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

void MainWindow::openModelFromLib(QString glbFile, QString model, QString gmsFileName)
{
    if (gmsFileName == "")
        gmsFileName = model.toLower() + ".gms";

    QDir gamsSysDir(CommonPaths::systemDir());
    mLibProcess = new GAMSLibProcess(this);
    mLibProcess->setGlbFile(gamsSysDir.filePath(glbFile));
    mLibProcess->setModelName(model);
    mLibProcess->setInputFile(gmsFileName);
    mLibProcess->setTargetDir(mSettings->defaultWorkspace());
    mLibProcess->execute();

    // This log is passed to the system-wide log
    connect(mLibProcess, &GamsProcess::newStdChannelData, this, &MainWindow::appendSystemLog);
    connect(mLibProcess, &GamsProcess::finished, this, &MainWindow::postGamsLibRun);
}

void MainWindow::receiveModLibLoad(QString model)
{
    openModelFromLib("gamslib_ml/gamslib.glb", model);
}

void MainWindow::receiveOpenDoc(QString doc, QString anchor)
{
    if (!getDockHelpView()->isVisible()) getDockHelpView()->show();

    QString link = CommonPaths::systemDir() + "/" + doc;
    QUrl result = QUrl::fromLocalFile(link);

    if (anchor != "")
        result = QUrl(result.toString() + "#" + anchor);

    getDockHelpView()->on_urlOpened(result);
}

SearchWidget* MainWindow::searchWidget() const
{
    return mSearchWidget;
}

QString MainWindow::encodingMIBsString()
{
    QStringList res;
    foreach (QAction *act, ui->menuEncoding->actions()) {
        if (!act->data().isNull()) res << act->data().toString();
    }
    return res.join(",");
}

QList<int> MainWindow::encodingMIBs()
{
    QList<int> res;
    foreach (QAction *act, ui->menuEncoding->actions())
        if (!act->data().isNull()) res << act->data().toInt();
    return res;
}

void MainWindow::setEncodingMIBs(QString mibList, int active)
{
    QList<int> mibs;
    QStringList strMibs = mibList.split(",");
    foreach (QString mib, strMibs) {
        if (mib.length()) mibs << mib.toInt();
    }
    setEncodingMIBs(mibs, active);
}

void MainWindow::setEncodingMIBs(QList<int> mibs, int active)
{
    while (mCodecGroupSwitch->actions().size()) {
        QAction *act = mCodecGroupSwitch->actions().last();
        if (ui->menuEncoding->actions().contains(act))
            ui->menuEncoding->removeAction(act);
        mCodecGroupSwitch->removeAction(act);
    }
    while (mCodecGroupReload->actions().size()) {
        QAction *act = mCodecGroupReload->actions().last();
        if (ui->menureload_with->actions().contains(act))
            ui->menureload_with->removeAction(act);
        mCodecGroupReload->removeAction(act);
    }
    foreach (int mib, mibs) {
        if (!QTextCodec::availableMibs().contains(mib)) continue;
        QAction *act = new QAction(QTextCodec::codecForMib(mib)->name(), mCodecGroupSwitch);
        act->setCheckable(true);
        act->setData(mib);
        act->setChecked(mib == active);

        act = new QAction(QTextCodec::codecForMib(mib)->name(), mCodecGroupReload);
        act->setCheckable(true);
        act->setData(mib);
        act->setChecked(mib == active);
    }
    ui->menuEncoding->addActions(mCodecGroupSwitch->actions());
    ui->menureload_with->addActions(mCodecGroupReload->actions());
}

void MainWindow::setActiveMIB(int active)
{
    foreach (QAction *act, ui->menuEncoding->actions())
        if (!act->data().isNull()) {
            act->setChecked(act->data().toInt() == active);
        }

    foreach (QAction *act, ui->menureload_with->actions())
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

void MainWindow::toggleOptionDefinition(bool checked)
{
    if (checked) {
        updateRunState();
        mCommandLineOption->lineEdit()->setEnabled( false );
        mOptionEditor->show();
        emit mOptionEditor->optionTableModelChanged(mCommandLineOption->lineEdit()->text());
    } else {
        updateRunState();
        mCommandLineOption->lineEdit()->setEnabled( true );
        mOptionEditor->hide();
        mDockOptionView->widget()->resize( mCommandWidget->size() );
        this->resizeDocks({mDockOptionView}, {mCommandWidget->size().height()}, Qt::Vertical);
    }
}

void MainWindow::closeHelpView()
{
    if (mDockHelpView)
        mDockHelpView->close();
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
    ui->actionOption_View->setChecked(visibility || tabifiedDockWidgets(mDockOptionView).count());
}

void MainWindow::helpViewVisibilityChanged(bool visibility)
{
    ui->actionHelp_View->setChecked(visibility || tabifiedDockWidgets(mDockHelpView).count());
}

void MainWindow::updateEditorPos()
{
    QPoint pos;
    QPoint anchor;
    AbstractEditor* edit = FileMeta::toAbstractEdit(mRecent.editor());
    CodeEditor *ce = FileMeta::toCodeEdit(edit);
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
    CodeEditor* edit = FileMeta::toCodeEdit(mRecent.editor());
    if (!edit || edit->isReadOnly()) {
        mStatusWidgets->setEditMode(EditMode::Readonly);
    } else {
        mStatusWidgets->setEditMode(edit->overwriteMode() ? EditMode::Overwrite : EditMode::Insert);
    }
}

void MainWindow::updateEditorBlockCount()
{
    AbstractEditor* edit = FileMeta::toAbstractEdit(mRecent.editor());
    if (edit) mStatusWidgets->setLineCount(edit->blockCount());
}

void MainWindow::on_currentDocumentChanged(int from, int charsRemoved, int charsAdded)
{
    searchWidget()->on_documentContentChanged(from, charsRemoved, charsAdded);
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
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString path = QFileInfo(mRecent.path).path();
    QStringList fNames = QFileDialog::getOpenFileNames(this, "Open file", path,
                                                       tr("GAMS code (*.gms *.inc *.gdx *.lst *.opt);;"
                                                          "Text files (*.txt);;"
                                                          "All files (*.*)"),
                                                       nullptr,
                                                       DONT_RESOLVE_SYMLINKS_ON_MACOS);

    foreach (QString item, fNames) {
        addNode("", item);
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
    if (mRecent.editFileId < 0) return;
    FileMeta *fileMeta = mFileMetaRepo.fileMeta(mRecent.editFileId);
    if (!fileMeta) return;
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

        fileMeta->saveAs(filePath);
        ProjectRunGroupNode* runGroup = mRecent.group ? mRecent.group->runParentNode() : nullptr;
        openFile(fileMeta, true, runGroup);
        mStatusWidgets->setFileName(fileMeta->location());
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
    for(int i = ui->mainTab->count(); i > 0; i--) {
        on_mainTab_tabCloseRequested(0);
    }
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
        if (fm->document() && !fm->isReadOnly()) fm->document()->setModified(true);
        updateMenuToCodec(action->data().toInt());
        mStatusWidgets->setEncoding(fm->codecMib());
    }
}

void MainWindow::codecReload(QAction *action)
{
    FileMeta *fm = mFileMetaRepo.fileMeta(mRecent.editFileId);
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
            fm->load(QList<int>() << action->data().toInt());
            updateMenuToCodec(action->data().toInt());
            mStatusWidgets->setEncoding(fm->codecMib());
        }
    }
}

void MainWindow::loadCommandLineOptions(ProjectFileNode* fc)
{
    ProjectRunGroupNode* group = fc->runParentNode();
    if (!group) return;

    mCommandLineOption->clear();
    QStringList option = mCommandLineHistory->getHistoryFor(group->runnableGms()->location());
    foreach(QString str, option) {
       mCommandLineOption->insertItem(0, str );
    }
    mCommandLineOption->setCurrentIndex(0);
    mCommandLineOption->setEnabled(true);
    mCommandLineOption->setCurrentContext(fc->location());
}

void MainWindow::activeTabChanged(int index)
{
    if (!mCommandLineOption->getCurrentContext().isEmpty()) {
        mCommandLineHistory->addIntoCurrentContextHistory(mCommandLineOption->getCurrentOption());
    }
    mCommandLineOption->setCurrentIndex(-1);
    mCommandLineOption->setCurrentContext("");
    mCommandWidget->setEnabled(false);
    mOptionEditor->setEnabled(false);

    // remove highlights from old tab
    if (mRecent.editFileId >= 0)
        mTextMarkRepo.removeMarks(mRecent.editFileId, QSet<TextMark::Type>() << TextMark::match);

    mRecent.setEditor(nullptr, this);
    QWidget *editWidget = (index < 0 ? nullptr : ui->mainTab->widget(index));
    AbstractEditor* edit = FileMeta::toAbstractEdit(editWidget);
    lxiviewer::LxiViewer* lxiViewer = FileMeta::toLxiViewer(editWidget);
    gdxviewer::GdxViewer *gdxViewer = FileMeta::toGdxViewer(editWidget);

    FileMeta* fm = mFileMetaRepo.fileMeta(edit ? edit->fileId() : gdxViewer ? gdxViewer->fileId() : -1);

    // TODO(JM) missing common base class for all viewers/editors
    if (fm) {
        mRecent.editFileId = fm->id();
        mStatusWidgets->setFileName(fm->location());
        mStatusWidgets->setEncoding(fm->codecMib());
        if (edit) {
            mRecent.setEditor(lxiViewer ? editWidget : edit, this);
            mRecent.group = mProjectRepo.asGroup(edit->groupId());
            if (!edit->isReadOnly()) {
                loadCommandLineOptions(fm);
                mCommandWidget->setEnabled(true);
                mOptionEditor->setEnabled(true);
                updateRunState();
                ui->menuEncoding->setEnabled(true);
            }
            updateMenuToCodec(fm->codecMib());
            mStatusWidgets->setLineCount(edit->blockCount());
            ui->menuEncoding->setEnabled(fm && !edit->isReadOnly());
        } else if (gdxViewer) {
            ui->menuEncoding->setEnabled(false);
            mRecent.setEditor(gdxViewer, this);
            mRecent.group = mProjectRepo.asGroup(gdxViewer->groupId());
            mStatusWidgets->setLineCount(-1);
            gdxViewer->reload();
        }
    } else {
        ui->menuEncoding->setEnabled(false);
        mStatusWidgets->setFileName("");
        mStatusWidgets->setEncoding(-1);
        mStatusWidgets->setLineCount(-1);
    }

    if (searchWidget()) searchWidget()->updateReplaceActionAvailability();

    CodeEditor* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly()) ce->setOverwriteMode(mOverwriteMode);
    updateEditorMode();
}

void MainWindow::fileChanged(FileId fileId)
{
    QWidgetList editors = mProjectRepo.editors(fileId);
    for (QWidget *edit: editors) {
        int index = ui->mainTab->indexOf(edit);
        if (index >= 0) {
            ProjectFileNode *fc = mProjectRepo.asFile(fileId);
            if (fc) ui->mainTab->setTabText(index, fc->name(NameModifier::editState));
        }
    }
}

void MainWindow::fileChangedExtern(FileId fileId)
{
    ProjectFileNode *fc = mProjectRepo.asFile(fileId);

    // file has not been loaded: nothing to do
    if (!fc->document()) return;

    int choice;

    // TODO(JM) Handle other file-types
    if (fc->metrics().fileType().autoReload()) {
        choice = QMessageBox::Yes;

    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("File modified");

        // file is loaded but unchanged: ASK, if it should be reloaded
        if (fc->document()) {
            msgBox.setText(fc->location()+" has been modified externally.");
            msgBox.setInformativeText("Reload?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        }
        msgBox.setDefaultButton(QMessageBox::NoButton);
        choice = msgBox.exec();
    }

    if (choice == QMessageBox::Yes || choice == QMessageBox::Discard) {
        fc->load(fc->codecMib(), true);
    } else {
        fc->document()->setModified();
    }
}

void MainWindow::fileDeletedExtern(FileId fileId)
{
    ProjectFileNode *fc = mProjectRepo.asFile(fileId);
    // file has not been loaded: nothing to do
    if (!fc->document()) return;

    QMessageBox msgBox;
    msgBox.setWindowTitle("File vanished");

    // file is loaded: ASK, if it should be closed
    msgBox.setText(fc->location()+" doesn't exist any more.");
    msgBox.setInformativeText("Keep file in editor?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::NoButton);
    int ret = msgBox.exec();

    if (ret == QMessageBox::No)
        fileClosed(fileId);
    else
        fc->document()->setModified();
}

void MainWindow::fileClosed(FileId fileId)
{
    ProjectFileNode* fc = mProjectRepo.asFile(fileId);
    mClosedTabs << fc->location();
    if (!fc)
        FATAL() << "FileId " << fileId << " is not of class FileNode.";
    while (!fc->editors().isEmpty()) {
        QWidget *edit = fc->editors().first();
        ui->mainTab->removeTab(ui->mainTab->indexOf(edit));
        fc->removeEditor(edit);
        edit->deleteLater();
    }

    // TODO(JM) Right now there are issues with the TextMarkList, so we leave it out for now
//    if (!QFileInfo(fc->location()).exists()) {
//        fc->marks()->unbind();
//        emit fc->parentEntry()->removeNode(fc);
//        fc = nullptr;
//    }

}

void MainWindow::fileEvent(FileMeta *fileMeta, const FileEvent &e)
{
    // TODO(JM) handle what has happened to the file
}

void MainWindow::appendSystemLog(const QString &text)
{
    QPlainTextEdit *outWin = ui->systemLogView;
    if (!text.isNull()) {
        outWin->moveCursor(QTextCursor::End);
        outWin->insertPlainText(text);
        outWin->moveCursor(QTextCursor::End);
        outWin->document()->setModified(false);
    }
}

void MainWindow::postGamsRun(AbstractProcess* process)
{
    ProjectRunGroupNode* runGroup = mProjectRepo.findGroup(process);
    QFileInfo fileInfo(process->inputFile());
    if(runGroup && fileInfo.exists()) {
        QString lstFile = runGroup->lstFileName();
//        appendErrData(fileInfo.path() + "/" + fileInfo.completeBaseName() + ".err");
        bool doFocus = runGroup->isActive();

        if (mSettings->jumpToError())
            runGroup->jumpToFirstError(doFocus);

        ProjectAbstractNode * node = mProjectRepo.findNode(lstFile, runGroup);
        ProjectFileNode* lstNode = node ? node->toFile() : nullptr;
        if (!lstNode) lstNode = mFileMetaRepo.findOrCreateFileMeta(lstFile);
        if (lstNode) lstNode->updateMarks();

        if (mSettings->openLst())
            openFile(lstNode, true, runGroup);

    }
    ui->dockLogView->raise();
//    setRunActionsEnabled(true);
}

void MainWindow::postGamsLibRun(AbstractProcess* process)
{
    // TODO(AF) Are there models without a GMS file? How to handle them?"
    Q_UNUSED(process);
    ProjectFileNode *fc = nullptr;
    mProjectRepo.findFile(mLibProcess->targetDir() + "/" + mLibProcess->inputFile(), &fc);
    if (!fc)
        fc = addNode(mLibProcess->targetDir(), mLibProcess->inputFile());
    if (fc && !fc->editors().isEmpty()) {
        fc->load(fc->codecMib());
    }
    openFile(fc);
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
    if ( (mRecent.editor() != nullptr) && (focusWidget() == mRecent.editor()) ) {
        CodeEditor* ce = FileMeta::toCodeEdit(mRecent.editor());
        QString word;
        int istate = 0;
        ce->wordInfo(ce->textCursor(), word, istate);

        if (istate == static_cast<int>(SyntaxState::Title)) {
            mDockHelpView->on_dollarControlHelpRequested("title");
        } else if (istate == static_cast<int>(SyntaxState::Directive)) {
            mDockHelpView->on_dollarControlHelpRequested(word);
        } else {
            mDockHelpView->on_keywordHelpRequested(word);
        }
    }
    if (mDockHelpView->isHidden())
        mDockHelpView->show();
    if (tabifiedDockWidgets(mDockHelpView).count())
        mDockHelpView->raise();
}

void MainWindow::on_actionAbout_triggered()
{
    QString about = "<b><big>GAMS Studio " + QApplication::applicationVersion() + "</big></b>";
    about += "<br/><br/>Release: GAMS Studio " + QApplication::applicationVersion() + " ";
    about += QString(sizeof(void*)==8 ? "64" : "32") + " bit<br/>";
    about += "Build Date: " __DATE__ " " __TIME__ "<br/><br/>";
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
    QMessageBox::about(this, "About GAMS Studio", about);
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
    QWidget* edit = ui->mainTab->widget(index);
    ProjectFileNode* fc = mProjectRepo.asFile(edit);
    if (!fc) {
        ui->mainTab->removeTab(index);
        // assuming we are closing a welcome page here
        mWp = nullptr;
        return;
    }

    int ret = QMessageBox::Discard;
    if (fc->editors().size() == 1 && fc->isModified()) {
        // only ask, if this is the last editor of this file
        QMessageBox msgBox;
        msgBox.setText(ui->mainTab->tabText(index)+" has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        ret = msgBox.exec();
    }
    if (ret == QMessageBox::Save)
        fc->save();

    if (ret != QMessageBox::Cancel) {
        for (const auto& file : mAutosaveHandler->checkForAutosaveFiles(mOpenTabsList))
            QFile::remove(file);
        if (fc->editors().size() == 1) {
            mProjectRepo.close(fc->id());
//            if (!QFileInfo(fc->location()).exists()) {
//                emit fc->parentEntry()->removeNode(fc);
//                fc = nullptr;
//            }
        } else {
            fc->removeEditor(edit);
            ui->mainTab->removeTab(ui->mainTab->indexOf(edit));
        }
    }
}

void MainWindow::on_logTabs_tabCloseRequested(int index)
{
    QWidget* edit = ui->logTabs->widget(index);
    if (edit) {
        ProjectLogNode* log = mProjectRepo.asLog(edit);
        if (log) log->removeEditor(edit);
        ui->logTabs->removeTab(index);
    }
}

void MainWindow::createWelcomePage()
{
    mWp = new WelcomePage(history(), this);
    ui->mainTab->insertTab(0, mWp, QString("Welcome")); // always first position
    connect(mWp, &WelcomePage::linkActivated, this, &MainWindow::openFilePath);
    ui->mainTab->setCurrentIndex(0); // go to welcome page
}

void MainWindow::createRunAndCommandLineWidgets()
{
    mGamsOption = new Option(CommonPaths::systemDir(), QString("optgams.def"));
    mCommandLineTokenizer = new CommandLineTokenizer(mGamsOption);
    mCommandLineOption = new CommandLineOption(true, this);
    mCommandLineHistory = new CommandLineHistory(this);

    mDockOptionView = new QDockWidget(this);
    mDockOptionView->setObjectName(QStringLiteral("mDockOptionView"));
    mDockOptionView->setEnabled(true);
    mDockOptionView->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    mDockOptionView->setWindowTitle("Option");
    mDockOptionView->setFloating(false);
    connect(mDockOptionView, &QDockWidget::visibilityChanged, this, &MainWindow::optionViewVisibiltyChanged);

    QWidget* optionWidget = new QWidget;
    QVBoxLayout* widgetVLayout = new QVBoxLayout;
    widgetVLayout->setContentsMargins(0,0,0,0);

    QHBoxLayout* commandHLayout = new QHBoxLayout;
    commandHLayout->setContentsMargins(-1,0,-1,0);

    QMenu* runMenu = new QMenu;
    runMenu->addAction(ui->actionRun);
    runMenu->addAction(ui->actionRun_with_GDX_Creation);
    runMenu->addSeparator();
    runMenu->addAction(ui->actionCompile);
    runMenu->addAction(ui->actionCompile_with_GDX_Creation);
    ui->actionRun->setShortcutVisibleInContextMenu(true);
    ui->actionRun_with_GDX_Creation->setShortcutVisibleInContextMenu(true);
    ui->actionCompile->setShortcutVisibleInContextMenu(true);
    ui->actionCompile_with_GDX_Creation->setShortcutVisibleInContextMenu(true);

    mRunToolButton = new QToolButton(this);
    mRunToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    mRunToolButton->setMenu(runMenu);
    mRunToolButton->setDefaultAction(ui->actionRun);
    QSizePolicy buttonSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    buttonSizePolicy.setVerticalStretch(0);
    mRunToolButton->setSizePolicy(buttonSizePolicy);

    commandHLayout->addWidget(mRunToolButton);

    interruptToolButton = new QToolButton(this);
    interruptToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu* interruptMenu = new QMenu();
    QIcon interruptIcon(":/img/interrupt");
    QIcon stopIcon(":/img/stop");
    QAction* interruptAction = interruptMenu->addAction(interruptIcon, "Interrupt");
    QAction* stopAction = interruptMenu->addAction(stopIcon, "Stop");
    connect(interruptAction, &QAction::triggered, this, &MainWindow::interruptTriggered);
    connect(stopAction, &QAction::triggered, this, &MainWindow::stopTriggered);
    interruptToolButton->setMenu(interruptMenu);
    interruptToolButton->setDefaultAction(interruptAction);
    interruptToolButton->setSizePolicy(buttonSizePolicy);

    commandHLayout->addWidget(interruptToolButton);

    commandHLayout->addWidget(mCommandLineOption);

    QPushButton* helpButton = new QPushButton(this);
    QIcon helpButtonIcon(":/img/question");
    helpButton->setIcon(helpButtonIcon);
    helpButton->setToolTip(QStringLiteral("Help on The GAMS Call and Command Line Parameters"));
    helpButton->setSizePolicy(buttonSizePolicy);
    commandHLayout->addWidget(helpButton);

    mShowOptionDefintionCheckBox = new QCheckBox(this);
    mShowOptionDefintionCheckBox->setObjectName(QStringLiteral("showOptionDefintionCheckBox"));
    mShowOptionDefintionCheckBox->setEnabled(true);
    mShowOptionDefintionCheckBox->setText(QApplication::translate("OptionEditor", "Option Editor", nullptr));
    mShowOptionDefintionCheckBox->setSizePolicy(buttonSizePolicy);
    commandHLayout->addWidget(mShowOptionDefintionCheckBox);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy.setVerticalStretch(0);
    optionWidget->setSizePolicy(sizePolicy);

    mOptionEditor = new OptionEditor(mCommandLineOption, mCommandLineTokenizer, mDockOptionView);
    mOptionEditor->hide();

    mCommandWidget = new QWidget;
    mCommandWidget->setLayout( commandHLayout );
    QSizePolicy commandSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    commandSizePolicy.setVerticalStretch(0);
    mCommandWidget->setSizePolicy(commandSizePolicy);

    widgetVLayout->addWidget( mCommandWidget );
    widgetVLayout->addWidget( mOptionEditor );
    optionWidget->setLayout( widgetVLayout );

    mDockOptionView->setWidget( optionWidget );

    mDockOptionView->show();
    ui->actionOption_View->setChecked(true);

    this->addDockWidget(Qt::TopDockWidgetArea, mDockOptionView);
    mDockOptionView->widget()->resize( mCommandWidget->size() );
    this->resizeDocks({mDockOptionView}, {mCommandWidget->size().height()}, Qt::Vertical);

    connect(mShowOptionDefintionCheckBox, &QCheckBox::clicked, this, &MainWindow::toggleOptionDefinition);
    connect(helpButton, &QPushButton::clicked, this, &MainWindow::on_commandLineHelpTriggered);
}

void MainWindow::connectCommandLineWidgets()
{
    connect(mCommandLineOption, &CommandLineOption::optionRunChanged,
            this, &MainWindow::on_runWithChangedOptions);

    connect(mCommandLineOption, &CommandLineOption::commandLineOptionChanged,
            mCommandLineTokenizer, &CommandLineTokenizer::formatTextLineEdit);
    connect(mCommandLineOption, &CommandLineOption::commandLineOptionChanged,
            mOptionEditor, &OptionEditor::updateTableModel );

    connect(mCommandLineOption, &QComboBox::editTextChanged,  mCommandLineOption, &CommandLineOption::validateChangedOption );
}

void MainWindow::setRunActionsEnabled(bool enable)
{
    ui->actionRun->setEnabled(enable);
    ui->actionRun_with_GDX_Creation->setEnabled(enable);
    ui->actionCompile->setEnabled(enable);
    ui->actionCompile_with_GDX_Creation->setEnabled(enable);
}

bool MainWindow::isActiveTabEditable()
{
    QWidget *editWidget = (ui->mainTab->currentIndex() < 0 ? nullptr : ui->mainTab->widget((ui->mainTab->currentIndex())) );
    AbstractEditor* edit = FileMeta::toAbstractEdit( editWidget );
    if (edit) {
        ProjectFileNode* fc = mProjectRepo.asFile(edit);
        return (fc && !edit->isReadOnly());
    }
    return false;
}

QString MainWindow::getCommandLineStrFrom(const QList<OptionItem> optionItems, const QList<OptionItem> forcedOptionItems)
{
    QString commandLineStr;
    QStringList keyList;
    for(OptionItem item: optionItems) {
        if (item.disabled)
            continue;

        commandLineStr.append(item.key);
        commandLineStr.append("=");
        commandLineStr.append(item.value);
        commandLineStr.append(" ");
        keyList << item.key;
    }
    QString message;
    for(OptionItem item: forcedOptionItems) {
        if (item.disabled)
            continue;

//        if ( keyList.contains(item.key, Qt::CaseInsensitive) ||
//             keyList.contains(gamsOption->getSynonym(item.key)) ) {
//            message.append(QString("\n   '%1' with '%1=%2'").arg(item.key).arg(item.value));
//        }

        commandLineStr.append(item.key);
        commandLineStr.append("=");
        commandLineStr.append(item.value);
        commandLineStr.append(" ");
    }
    if (!message.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText(QString("This action will override the following command line options: %1").arg(message));
        msgBox.setInformativeText("Do you want to continue ?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.exec();
    }
    return commandLineStr.simplified();
}

void MainWindow::on_actionShow_System_Log_triggered()
{
    int index = ui->logTabs->indexOf(ui->systemLog);
    ui->logTabs->setCurrentIndex(index);
}

void MainWindow::on_actionShow_Welcome_Page_triggered()
{
    if(mWp == nullptr)
        createWelcomePage();
    else
        ui->mainTab->setCurrentIndex(ui->mainTab->indexOf(mWp));
}

void MainWindow::renameToBackup(QFile *file)
{
    const int MAX_BACKUPS = 3;
    ProjectAbstractNode *fsc = mProjectRepo.findNode(file->fileName());
    if (fsc) {
        ProjectFileNode *fc = mProjectRepo.asFile(fsc->id());
        if (fc) fc->unwatch();
    }

    QString filename = file->fileName();

    // find oldest backup file
    int last = 1;
    while (QFile(filename + "." + QString::number(last) + ".bak").exists()) {
        if (last == MAX_BACKUPS) break; // dont exceed MAX_BACKUPS
        last++;
    }
    if (last == MAX_BACKUPS) { // delete if maximum reached
        QFile(filename + "." + QString::number(last) + ".bak").remove();
        last--; // last is now one less
    }

    // move up all by 1, starting last
    for (int i = last; i > 0; i--) {
        QFile(filename + "." + QString::number(i) + ".bak") // from
                .rename(filename + "." + QString::number(i + 1) + ".bak"); // to
    }
    //rename to 1
    file->rename(filename + ".1.bak");
}

void MainWindow::triggerGamsLibFileCreation(LibraryItem *item, QString gmsFileName)
{
    openModelFromLib(item->library()->glbFile(), item->name(), gmsFileName);
}

QStringList MainWindow::openedFiles()
{
    return history()->lastOpenedFiles;
}

void MainWindow::openFilePath(const QString &filePath)
{
    FileMeta* fm = mFileMetaRepo.findOrCreateFileMeta(filePath);
    // TODO(JM) Do we need to find a ProjectRunGroupNode as context?
    openFile(fm, true, nullptr, -1);
}

HistoryData *MainWindow::history()
{
    return mHistory;
}

void MainWindow::addToOpenedFiles(QString filePath)
{
    if (!QFileInfo(filePath).exists()) return;

    if (filePath.startsWith('[')) return; // invalid

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
                addNode("", gmsFilePath);
                break;
            case 1: // replace
                renameToBackup(&gmsFile);
                triggerGamsLibFileCreation(item, gmsFileName);
                break;
            case QMessageBox::Abort:
                break;
            }
        } else {
            triggerGamsLibFileCreation(item, gmsFileName);
        }
    }
}

void MainWindow::on_projectView_activated(const QModelIndex &index)
{
    ProjectAbstractNode* node = mProjectRepo.node(index);
    if (node->toRunGroup()) {
        ProjectLogNode* logNode = node->toRunGroup()->getOrCreateLogNode(mTextMarkRepo, mFileMetaRepo);
        if (logNode->editors().isEmpty()) {
            logNode->setDebugLog(mLogDebugLines);
            LogEditor* logEdit = new LogEditor(mSettings.get(), this);
            ProjectAbstractNode::initEditorType(logEdit);
            logEdit->setLineWrapMode(mSettings->lineWrapProcess() ? AbstractEditor::WidgetWidth
                                                                  : AbstractEditor::NoWrap);
            int ind = ui->logTabs->addTab(logEdit, logNode->name(NameModifier::editState));
            logNode->addEditor(logEdit);
            ui->logTabs->setCurrentIndex(ind);
        }
    } else if (node->toFile()) {
        openNode(index);
    }
}

bool MainWindow::requestCloseChanged(QList<ProjectFileNode*> changedFiles)
{
    // TODO: make clear that this saves/discrads all modified files?
    if (changedFiles.size() > 0) {
        int ret = QMessageBox::Discard;
        QMessageBox msgBox;
        QString filesText = changedFiles.size()==1 ? changedFiles.first()->location() + " has been modified."
                                             : QString::number(changedFiles.size())+" files have been modified";
        msgBox.setText(filesText);
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        ret = msgBox.exec();
        if (ret == QMessageBox::Save) {
            for (ProjectFileNode* fc: changedFiles) {
                if (fc->isModified()) {
                    fc->save();
                }
            }
        }
        if (ret == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

StudioSettings *MainWindow::settings() const
{
    return mSettings.get();
}

RecentData *MainWindow::recent()
{
    return &mRecent;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QList<ProjectFileNode*> oFiles = mProjectRepo.modifiedFiles();
    if (!requestCloseChanged(oFiles)) {
        event->setAccepted(false);
    } else {
        mSettings->saveSettings(this);
    }
    on_actionClose_All_triggered();
    closeHelpView();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_0)){
            updateFixedFonts(mSettings->fontFamily(), mSettings->fontSize());
    }
    if (focusWidget() == ui->projectView && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
        openNode(ui->projectView->currentIndex());
        event->accept();
    } else {
        QMainWindow::keyPressEvent(event);
    }

    if (event->key() == Qt::Key_Escape) {
        mSearchWidget->hide();
        mSearchWidget->clearResults();
    }
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
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            answer = msgBox.exec();

            if(answer != QMessageBox::Ok) return;
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
        ((LineEditCompleteEvent*)event)->complete();
}

void MainWindow::parseFilesFromCommandLine(ProjectGroupNode* fgc)
{
    QList<OptionItem> items = mCommandLineTokenizer->tokenize(mCommandLineOption->getCurrentOption());

    // set default lst file name in case output option changed back to default
    if (!fgc->runnableGms().isEmpty())
        fgc->setLstFileName(QFileInfo(fgc->runnableGms()).baseName() + ".lst");

    foreach (OptionItem item, items) {
        // output (o) found, case-insensitive
        if (QString::compare(item.key, "o", Qt::CaseInsensitive) == 0
                || QString::compare(item.key, "output", Qt::CaseInsensitive) == 0) {

            fgc->setLstFileName(item.value);
        }
    }
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

void MainWindow::execute(QString commandLineStr, ProjectFileNode* gmsFileNode)
{
    mPerformanceTime.start();
    ProjectFileNode* fc = (gmsFileNode ? gmsFileNode : mProjectRepo.asFile(mRecent.editor()));
    ProjectGroupNode *group = (fc ? fc->parentNode() : nullptr);
    if (!group) return;

    parseFilesFromCommandLine(group);

    group->clearLstErrorTexts();

    if (mSettings->autosaveOnRun())
        group->saveGroup();

    if (fc->editors().size() > 0 && fc->isModified()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(fc->location()+" has been modified.");
        msgBox.setInformativeText("Do you want to save your changes before running?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        QAbstractButton* discardButton = msgBox.addButton(tr("Discard Changes and Run"), QMessageBox::ResetRole);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Cancel) {
            return;
        } else if (ret == QMessageBox::Save) {
            fc->save();
        } else if (msgBox.clickedButton() == discardButton) {
            fc->load(fc->codecMib());
        }
    }

    setRunActionsEnabled(false);

    mProjectRepo.removeMarks(group);
    ProjectLogNode* logProc = mProjectRepo.asLog(group);

    if (logProc->editors().isEmpty()) {
        logProc->setDebugLog(mLogDebugLines);
        LogEditor* logEdit = new LogEditor(mSettings.get(), this);
        ProjectAbstractNode::initEditorType(logEdit);

        ui->logTabs->addTab(logEdit, logProc->name(NameModifier::editState));
        logProc->addEditor(logEdit);
        updateFixedFonts(mSettings->fontFamily(), mSettings->fontSize());
    }

    if (!mSettings->clearLog()) {
        logProc->markOld();
    } else {
        logProc->clearLog();
    }
    if (!ui->logTabs->children().contains(logProc->editors().first())) {
        ui->logTabs->addTab(logProc->editors().first(), logProc->name(NameModifier::editState));
    }
    ui->logTabs->setCurrentWidget(logProc->editors().first());

    ui->dockLogView->setVisible(true);
    QString gmsFilePath = (gmsFileNode ? gmsFileNode->location() : group->runnableGms());

    if (gmsFilePath == "")
        appendSystemLog("No runnable GMS file found.");

    QFileInfo gmsFileInfo(gmsFilePath);

    logProc->setJumpToLogEnd(true);
    GamsProcess* process = group->gamsProcess();
    QString lstFileName = group->lstFileName();
    if (gmsFileNode) {
        QFileInfo fi(gmsFilePath);
        lstFileName = fi.path() + "/" + fi.completeBaseName() + ".lst";
    }
    process->setWorkingDir(gmsFileInfo.path());
    process->setInputFile(gmsFilePath);
    process->setCommandLineStr(commandLineStr);
    process->execute();

    connect(process, &GamsProcess::newStdChannelData, logProc, &ProjectLogNode::addProcessData, Qt::UniqueConnection);
    connect(process, &GamsProcess::finished, this, &MainWindow::postGamsRun, Qt::UniqueConnection);
}

void MainWindow::interruptTriggered()
{
    ProjectFileNode* fc = mProjectRepo.asFile(mRecent.editor());
    ProjectGroupNode *group = (fc ? fc->parentNode() : nullptr);
    if (!group)
        return;
    GamsProcess* process = group->gamsProcess();
    QtConcurrent::run(process, &GamsProcess::interrupt);
}

void MainWindow::stopTriggered()
{
    ProjectFileNode* fc = mProjectRepo.asFile(mRecent.editor());
    ProjectGroupNode *group = (fc ? fc->parentNode() : nullptr);
    if (!group)
        return;
    GamsProcess* process = group->gamsProcess();
    QtConcurrent::run(process, &GamsProcess::stop);
}

void MainWindow::updateRunState()
{
    QProcess::ProcessState state = mRecent.group ? mRecent.group->gamsProcessState() : QProcess::NotRunning;
    setRunActionsEnabled(state != QProcess::Running);
    interruptToolButton->setEnabled(state == QProcess::Running);
    interruptToolButton->menu()->setEnabled(state == QProcess::Running);
    mCommandLineOption->lineEdit()->setReadOnly(state == QProcess::Running);
    mShowOptionDefintionCheckBox->setEnabled(state != QProcess::Running);
    mOptionEditor->setEnabled(state != QProcess::Running);
}

void MainWindow::on_runWithChangedOptions()
{
    mCommandLineHistory->addIntoCurrentContextHistory( mCommandLineOption->getCurrentOption() );
    execute( mCommandLineOption->getCurrentOption() );
}

void MainWindow::on_runWithParamAndChangedOptions(const QList<OptionItem> forcedOptionItems)
{
    mCommandLineHistory->addIntoCurrentContextHistory( mCommandLineOption->getCurrentOption() );
    execute( getCommandLineStrFrom(mCommandLineTokenizer->tokenize(mCommandLineOption->getCurrentOption()),
                                   forcedOptionItems) );
}

void MainWindow::on_runGmsFile(ProjectFileNode *fc)
{
    execute("", fc);
}

void MainWindow::on_setMainGms(ProjectFileNode *fc)
{
    fc->parentNode()->setRunnableGms(fc);
    loadCommandLineOptions(fc);
}

void MainWindow::on_commandLineHelpTriggered()
{
    mDockHelpView->on_commandLineHelpRequested();
    if (mDockHelpView->isHidden())
        mDockHelpView->show();
    if (tabifiedDockWidgets(mDockHelpView).count())
        mDockHelpView->raise();
}

void MainWindow::on_actionRun_triggered()
{
    if (isActiveTabEditable()) {
       emit mCommandLineOption->optionRunChanged();
       mRunToolButton->setDefaultAction( ui->actionRun );
    }
}

void MainWindow::on_actionRun_with_GDX_Creation_triggered()
{
    if (isActiveTabEditable()) {
        QList<OptionItem> forcedOptionItems;
        forcedOptionItems.append( OptionItem("GDX", "default", -1, -1, false) );
        on_runWithParamAndChangedOptions(forcedOptionItems);
        mRunToolButton->setDefaultAction( ui->actionRun_with_GDX_Creation );
    }
}

void MainWindow::on_actionCompile_triggered()
{
    if (isActiveTabEditable()) {
        QList<OptionItem> forcedOptionItems;
        forcedOptionItems.append( OptionItem("ACTION", "C", -1, -1, false) );
        on_runWithParamAndChangedOptions(forcedOptionItems);
        mRunToolButton->setDefaultAction( ui->actionCompile );
    }
}

void MainWindow::on_actionCompile_with_GDX_Creation_triggered()
{
    if (isActiveTabEditable()) {
        QList<OptionItem> forcedOptionItems;
        forcedOptionItems.append( OptionItem("ACTION", "C", -1, -1, false) );
        forcedOptionItems.append( OptionItem("GDX", "default", -1, -1, false) );
        on_runWithParamAndChangedOptions(forcedOptionItems);
        mRunToolButton->setDefaultAction( ui->actionCompile_with_GDX_Creation );
    }
}

void MainWindow::changeToLog(ProjectFileNode* fileNode)
{
    ProjectLogNode* logNode = mProjectRepo.asLog(fileNode);
    if (logNode && !logNode->file()->editors().isEmpty()) {
        logNode->setDebugLog(mLogDebugLines);
        AbstractEditor* logEdit = FileMeta::toAbstractEdit(logNode->file()->editors().first());
        if (logEdit && ui->logTabs->currentWidget() != logEdit) {
            if (ui->logTabs->currentWidget() != mResultsView)
                ui->logTabs->setCurrentWidget(logEdit);
        }
    }
}

TextMarkRepo *MainWindow::textMarkRepo() const
{
    return &mTextMarkRepo;
}

void MainWindow::openFile(FileMeta *fileMeta, bool focus, ProjectRunGroupNode *runGroup, int codecMib)
{
    if (!fileMeta) return;
    QWidget* edit = nullptr;
    QTabWidget* tabWidget = fileMeta->kind() == FileKind::Log ? ui->logTabs : ui->mainTab;
    if (!fileMeta->editors().empty()) {
        edit = fileMeta->editors().first();
    }
    if (edit) {
        if (focus) tabWidget->setCurrentWidget(edit);
    } else {
        edit = fileMeta->createEdit(tabWidget, runGroup, (codecMib == -1) ? encodingMIBs() : QList<int>() << codecMib);
        CodeEditor* codeEdit = FileMeta::toCodeEdit(edit);
        if (codeEdit) {
            connect(codeEdit, &CodeEditor::searchFindNextPressed, mSearchWidget, &SearchWidget::on_searchNext);
            connect(codeEdit, &CodeEditor::searchFindPrevPressed, mSearchWidget, &SearchWidget::on_searchPrev);
            if (!codeEdit->isReadOnly()) {
                connect(codeEdit, &CodeEditor::requestAdvancedActions, this, &MainWindow::getAdvancedActions);
            }
        }

        int tabIndex = tabWidget->addTab(edit, fileMeta->name());
        tabWidget->setTabToolTip(tabIndex, fileMeta->location());
        if (focus) tabWidget->setCurrentIndex(tabIndex);
        connect(fileMeta, &FileMeta::changed, this, &MainWindow::fileChanged, Qt::UniqueConnection);
        if (focus) {
            updateMenuToCodec(fileMeta->codecMib());
            mRecent.setEditor(tabWidget->currentWidget(), this);
            mRecent.editFileId = fileMeta->id();
        }
        fileMeta->load();
    }
    if (tabWidget->currentWidget())
        if (focus) {
            lxiviewer::LxiViewer* lxiViewer = FileMeta::toLxiViewer(edit);
            if (lxiViewer)
                lxiViewer->codeEditor()->setFocus();
            else
                tabWidget->currentWidget()->setFocus();
        }
    if (tabWidget != ui->logTabs) {
        // if there is already a log -> show it
        changeToLog(fileMeta);
    }
    addToOpenedFiles(fileMeta->location());
}

void MainWindow::closeGroup(ProjectGroupNode* group)
{
    if (!group) return;
    QList<ProjectFileNode*> changedFiles;
    QList<ProjectFileNode*> openFiles;
    for (int i = 0; i < group->childCount(); ++i) {
        ProjectAbstractNode* fsc = group->childNode(i);
        if (fsc->type() == NodeType::file) {
            ProjectFileNode* file = static_cast<ProjectFileNode*>(fsc);
            openFiles << file;
            if (file->isModified())
                changedFiles << file;
        }
    }
    if (requestCloseChanged(changedFiles)) {
        // TODO(JM)  close if selected
        for (ProjectFileNode *file: openFiles) {
            fileClosed(file->id());
        }
        ProjectRunGroupNode* runGroup = group->toRunGroup();
        ProjectLogNode* log = runGroup ? runGroup->logNode() : nullptr;
        if (log) {
            QWidget* edit = log->file()->editors().isEmpty() ? nullptr : log->file()->editors().first();
            if (edit) {
                log->file()->removeEditor(edit);
                int index = ui->logTabs->indexOf(edit);
                if (index >= 0) ui->logTabs->removeTab(index);
            }
        }

        mProjectRepo.removeGroup(group);
        mSettings->saveSettings(this);
    }

}

void MainWindow::closeFile(ProjectFileNode* file)
{
    if (!file->isModified() || requestCloseChanged(QList<ProjectFileNode*>() << file)) {
        ui->projectView->setCurrentIndex(QModelIndex());

        ProjectGroupNode *parent = file->parentNode();

        if (parent->runnableGms() == file->location())
            parent->removeRunnableGms();

        if (parent->logNode())
            parent->logNode()->fileClosed(file);

        fileClosed(file->id());
        mProjectRepo.removeFile(file);

        if (parent->childCount() == 0)
            closeGroup(parent);

        mSettings->saveSettings(this);
    }
}

ProjectFileNode* MainWindow::addNode(const QString &path, const QString &fileName)
{
    ProjectFileNode *fc = nullptr;
    if (!fileName.isEmpty()) {
        QFileInfo fInfo(path, fileName);

        FileType fType = FileType::from(fInfo.suffix());

        if (fType.kind() == FileKind::Gsp) {
            // TODO(JM) Read project and create all nodes for associated files
        } else {
            openFilePath(fInfo.filePath()); // open all sorts of files
        }
    }
    return fc;
}

void MainWindow::openNode(const QModelIndex& index)
{
    ProjectFileNode *file = mProjectRepo.asFile(index);
    if (file) openFilePath(file);
}

void MainWindow::on_mainTab_currentChanged(int index)
{
    QWidget* edit = ui->mainTab->widget(index);
    if (!edit) return;

    mProjectRepo.editorActivated(edit);
    ProjectFileNode* fc = mProjectRepo.asFile(edit);
    if (fc && mRecent.group != fc->parentNode()) {
        mRecent.group = fc->parentNode();
        updateRunState();
    }
    changeToLog(fc);
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog sd(mSettings.get(), this);
    connect(&sd, &SettingsDialog::editorFontChanged, this, &MainWindow::updateFixedFonts);
    connect(&sd, &SettingsDialog::editorLineWrappingChanged, this, &MainWindow::updateEditorLineWrapping);
    sd.exec();
    sd.disconnect();
    mSettings->saveSettings(this);
}

void MainWindow::on_actionSearch_triggered()
{
    if (getDockHelpView()->isAncestorOf(QApplication::focusWidget()) ||
        getDockHelpView()->isAncestorOf(QApplication::activeWindow())) {
        getDockHelpView()->on_searchHelp();
    } else {
       // toggle visibility
       if (mSearchWidget->isVisible()) {
           mSearchWidget->activateWindow();
           mSearchWidget->focusSearchField();
       } else {
           QPoint p(0,0);
           QPoint newP(this->mapToGlobal(p));

           if (ui->mainTab->currentWidget()) {
               int sbs;
               if (mRecent.editor() && FileMeta::toAbstractEdit(mRecent.editor())
                       && FileMeta::toAbstractEdit(mRecent.editor())->verticalScrollBar()->isVisible())
                   sbs = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2;
               else
                   sbs = 2;

               int offset = (this->width() - mSearchWidget->width() - sbs);
               mSearchWidget->move(newP.x() + offset, newP.y());
           }
           mSearchWidget->show();
       }
    }
}

void MainWindow::showResults(SearchResultList &results)
{
    int index = ui->logTabs->indexOf(mResultsView); // did widget exist before?

    mResultsView = new ResultsView(results, this);
    QString title("Results: " + mSearchWidget->searchTerm());

    ui->dockLogView->show();
    mResultsView->resizeColumnsToContent();

    if (index != -1) ui->logTabs->removeTab(index); // remove old result page

    ui->logTabs->addTab(mResultsView, title); // add new result page
    ui->logTabs->setCurrentWidget(mResultsView);
}

void MainWindow::updateFixedFonts(const QString &fontFamily, int fontSize)
{
    QFont font(fontFamily, fontSize);
    foreach (QWidget* edit, openEditors()) {
        if (!FileMeta::toGdxViewer(edit))
            FileMeta::toAbstractEdit(edit)->setFont(font);
    }
    foreach (QWidget* log, openLogs()) {
        log->setFont(font);
    }
    ui->systemLogView->setFont(font);
}

void MainWindow::updateEditorLineWrapping()
{// TODO(AF) split logs and editors
    QPlainTextEdit::LineWrapMode wrapModeEditor;
    if(mSettings->lineWrapEditor())
        wrapModeEditor = QPlainTextEdit::WidgetWidth;
    else
        wrapModeEditor = QPlainTextEdit::NoWrap;

    QWidgetList editList = mProjectRepo.editors();
    for (int i = 0; i < editList.size(); i++) {
        AbstractEditor* ed = FileMeta::toAbstractEdit(editList.at(i));
        if (ed) {
            ed->blockCountChanged(0); // force redraw for line number area
            ed->setLineWrapMode(wrapModeEditor);
        }
    }

    QPlainTextEdit::LineWrapMode wrapModeProcess;
    if(mSettings->lineWrapProcess())
        wrapModeProcess = QPlainTextEdit::WidgetWidth;
    else
        wrapModeProcess = QPlainTextEdit::NoWrap;

    QList<AbstractEditor*> logList = openLogs();
    for (int i = 0; i < logList.size(); i++) {
        if (logList.at(i))
            logList.at(i)->setLineWrapMode(wrapModeProcess);
    }
}

HelpView *MainWindow::getDockHelpView() const
{
    return mDockHelpView;
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
                    openFile(mFileMetaRepo.findOrCreateFileMeta(location), true, nullptr, mib);
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
        }
    }
    QTimer::singleShot(0,this,SLOT(initAutoSave()));
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
        ProjectFileNode *fc = mProjectRepo.asFile(wid);
        if (!fc) continue;
        QJsonObject tabObject;
        tabObject["location"] = fc->location();
        tabObject["codecMib"] = fc->codecMib();
        // TODO(JM) store current edit position
        tabArray.append(tabObject);
    }
    json["mainTabs"] = tabArray;
    ProjectFileNode *fc = mRecent.editor() ? mProjectRepo.asFile(mRecent.editor()) : nullptr;
    if (fc) json["mainTabRecent"] = fc->location();
}

QWidget *MainWindow::welcomePage() const
{
    return mWp;
}

void MainWindow::on_actionGo_To_triggered()
{
    int width = mGoto->frameGeometry().width();
    int height = mGoto->frameGeometry().height();
    QDesktopWidget wid;
    int thisscreen = QApplication::desktop()->screenNumber(this);
    int screenWidth = wid.screen(thisscreen)->width();
    int screenHeight = wid.screen(thisscreen)->height();
    int x = wid.availableGeometry(thisscreen).x();
    int y = wid.availableGeometry(thisscreen).y();
    if (mGoto->isVisible()) {
        mGoto->hide();
    } else {
        mGoto->setGeometry(x+(screenWidth/2)-(width/2),y+(screenHeight/2)-(height/2),width,height);
        if ((ui->mainTab->currentWidget() == mWp) || (mRecent.editor() == nullptr))
            return;
        mGoto->show();
        mGoto->focusTextBox();
    }
}


void MainWindow::on_actionRedo_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEditor* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce) ce->redo();
}

void MainWindow::on_actionUndo_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEditor* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce) ce->undo();
}

void MainWindow::on_actionPaste_triggered()
{
    CodeEditor *ce = FileMeta::toCodeEdit(focusWidget());
    if (!ce || ce->isReadOnly()) return;
    ce->pasteClipboard();
}

void MainWindow::on_actionCopy_triggered()
{
    if (!focusWidget()) return;

    ProjectFileNode *fc = mProjectRepo.asFile(mRecent.editor());
    if (!fc) return;

    if (fc->metrics().fileType() == FileType::Gdx) {
        gdxviewer::GdxViewer *gdx = FileMeta::toGdxViewer(mRecent.editor());
        gdx->copyAction();
    } else {
        AbstractEditor *ae = FileMeta::toAbstractEdit(focusWidget());
        if (!ae) return;
        CodeEditor *ce = FileMeta::toCodeEdit(ae);
        if (ce && ce->blockEdit()) {
            ce->blockEdit()->selectionToClipboard();
        } else {
            ae->copy();
        }
    }
}

void MainWindow::on_actionSelect_All_triggered()
{
    ProjectFileNode *fc = mProjectRepo.asFile(mRecent.editor());
    if (!fc || !focusWidget()) return;

    if (fc->metrics().fileType() == FileType::Gdx) {
        gdxviewer::GdxViewer *gdx = FileMeta::toGdxViewer(mRecent.editor());
        gdx->selectAllAction();
    } else {
        CodeEditor* ce = FileMeta::toCodeEdit(focusWidget());
        if (ce) ce->selectAll();
    }
}

void MainWindow::on_actionCut_triggered()
{
    CodeEditor* ce= FileMeta::toCodeEdit(focusWidget());
    if (!ce || ce->isReadOnly()) return;

    if (ce->blockEdit()) {
        ce->blockEdit()->selectionToClipboard();
        ce->blockEdit()->replaceBlockText("");
        return;
    } else {
        ce->cut();
    }
}

void MainWindow::on_actionReset_Zoom_triggered()
{
    if (getDockHelpView()->isAncestorOf(QApplication::focusWidget()) ||
        getDockHelpView()->isAncestorOf(QApplication::activeWindow())) {
        getDockHelpView()->resetZoom(); // reset help view
    } else {
        updateFixedFonts(mSettings->fontFamily(), mSettings->fontSize()); // reset all editors
    }

}

void MainWindow::on_actionZoom_Out_triggered()
{
    if (getDockHelpView()->isAncestorOf(QApplication::focusWidget()) ||
        getDockHelpView()->isAncestorOf(QApplication::activeWindow())) {
        getDockHelpView()->zoomOut();
    } else {
        AbstractEditor *ae = FileMeta::toAbstractEdit(QApplication::focusWidget());
        if (ae) {
            int pix = ae->fontInfo().pixelSize();
            if (pix == ae->fontInfo().pixelSize()) ae->zoomOut();
        }
    }
}

void MainWindow::on_actionZoom_In_triggered()
{
    if (getDockHelpView()->isAncestorOf(QApplication::focusWidget()) ||
        getDockHelpView()->isAncestorOf(QApplication::activeWindow())) {
        getDockHelpView()->zoomIn();
    } else {
        AbstractEditor *ae = FileMeta::toAbstractEdit(QApplication::focusWidget());
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
    CodeEditor* ce= FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly()) {
        QTextCursor c = ce->textCursor();
        c.insertText(c.selectedText().toUpper());
    }
}

void MainWindow::on_actionSet_to_Lowercase_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEditor* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly()) {
        QTextCursor c = ce->textCursor();
        c.insertText(c.selectedText().toLower());
    }
}

void MainWindow::on_actionOverwrite_Mode_toggled(bool overwriteMode)
{
    CodeEditor* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly()) {
        mOverwriteMode = overwriteMode;
        ce->setOverwriteMode(overwriteMode);
        updateEditorMode();
    }
}

void MainWindow::on_actionIndent_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEditor* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (!ce || ce->isReadOnly()) return;
    QPoint pos(-1,-1); QPoint anc(-1,-1);
    ce->getPositionAndAnchor(pos, anc);
    ce->indent(mSettings->tabSize(), pos.y()-1, anc.y()-1);
}

void MainWindow::on_actionOutdent_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEditor* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (!ce || ce->isReadOnly()) return;
    QPoint pos(-1,-1); QPoint anc(-1,-1);
    ce->getPositionAndAnchor(pos, anc);
    ce->indent(-mSettings->tabSize(), pos.y()-1, anc.y()-1);
}

void MainWindow::on_actionDuplicate_Line_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEditor* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly())
        ce->duplicateLine();
}

void MainWindow::on_actionRemove_Line_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEditor* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly())
        ce->removeLine();
}

void MainWindow::on_actionComment_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEditor* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly())
        ce->commentLine();
}

void MainWindow::toggleLogDebug()
{
    mLogDebugLines = !mLogDebugLines;
    ProjectGroupNode* root = mProjectRepo.treeModel()->rootNode();
    for (int i = 0; i < root->childCount(); ++i) {
        ProjectAbstractNode *fsc = root->childNode(i);
        if (fsc->type() == NodeType::group) {
            ProjectGroupNode* group = static_cast<ProjectGroupNode*>(fsc);
            ProjectLogNode* log = group->logNode();
            if (log) log->setDebugLog(mLogDebugLines);
        }
    }
}

void MainWindow::on_actionRestore_Recently_Closed_Tab_triggered()
{
    if (mClosedTabs.isEmpty())
        return;
    QFile file(mClosedTabs.last());
    mClosedTabs.removeLast();
    if (file.exists())
        openFilePath(file.fileName());
    else
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
    AbstractEditor* edit = FileMeta::toAbstractEdit(mEditor);
    if (edit) {
        MainWindow::disconnect(edit, &AbstractEditor::cursorPositionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::disconnect(edit, &AbstractEditor::selectionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::disconnect(edit, &AbstractEditor::blockCountChanged, window, &MainWindow::updateEditorBlockCount);
        MainWindow::disconnect(edit->document(), &QTextDocument::contentsChange, window, &MainWindow::on_currentDocumentChanged);
    }
    mEditor = editor;
    edit = FileMeta::toAbstractEdit(mEditor);
    if (edit) {
        MainWindow::connect(edit, &AbstractEditor::cursorPositionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::connect(edit, &AbstractEditor::selectionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::connect(edit, &AbstractEditor::blockCountChanged, window, &MainWindow::updateEditorBlockCount);
        MainWindow::connect(edit->document(), &QTextDocument::contentsChange, window, &MainWindow::on_currentDocumentChanged);
    }
    window->searchWidget()->invalidateCache();
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

    QDockWidget* stackedFirst;
    QDockWidget* stackedSecond;

    QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
    foreach (QDockWidget* dock, dockWidgets) {
        dock->setFloating(false);
        dock->setVisible(true);

        if (dock == ui->dockProjectView) {
            addDockWidget(Qt::LeftDockWidgetArea, dock);
            resizeDocks(QList<QDockWidget*>() << dock, {width()/6}, Qt::Horizontal);
        } else if (dock == ui->dockLogView) {
            addDockWidget(Qt::RightDockWidgetArea, dock);
            resizeDocks(QList<QDockWidget*>() << dock, {width()/3}, Qt::Horizontal);
            stackedFirst = dock;
        } else if (dock == mDockHelpView) {
            dock->setVisible(false);
            addDockWidget(Qt::RightDockWidgetArea, dock);
            resizeDocks(QList<QDockWidget*>() << dock, {width()/3}, Qt::Horizontal);
            stackedSecond = dock;
        } else if (dock == mDockOptionView) {
            addDockWidget(Qt::TopDockWidgetArea, dock);
        }
    }
    // stack help over output
    tabifyDockWidget(stackedFirst, stackedSecond);
}

}
}
