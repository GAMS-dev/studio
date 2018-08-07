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
#include "treeitemdelegate.h"
#include "commonpaths.h"
#include "gamsprocess.h"
#include "gamslibprocess.h"
#include "lxiviewer/lxiviewer.h"
#include "gdxviewer/gdxviewer.h"
#include "logger.h"
#include "studiosettings.h"
#include "settingsdialog.h"
#include "searchdialog.h"
#include "searchresultlist.h"
#include "resultsview.h"
#include "gotodialog.h"
#include "updatedialog.h"
#include "checkforupdatewrapper.h"
#include "autosavehandler.h"
#include "distributionvalidator.h"

namespace gams {
namespace studio {

MainWindow::MainWindow(StudioSettings *settings, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      mSettings(settings),
      mAutosaveHandler(new AutosaveHandler(this))
{
    mHistory = new HistoryData();
//    QFile css(":/data/style.css");
//    if (css.open(QFile::ReadOnly | QFile::Text)) {
//        this->setStyleSheet(css.readAll());
//    }

    ui->setupUi(this);

    setAcceptDrops(true);
    mTimerID = startTimer(60000);
    QList<QKeySequence> redoShortcuts;
    redoShortcuts << ui->actionRedo->shortcut() << QKeySequence("Ctrl+Shift+Z");
    ui->actionRedo->setShortcuts(redoShortcuts);

    QFont font = ui->statusBar->font();
    font.setPointSizeF(font.pointSizeF()*0.9);
    ui->statusBar->setFont(font);
    mStatusWidgets = new StatusWidgets(this);
    int iconSize = fontInfo().pixelSize()*2-1;
    ui->projectView->setModel(mProjectRepo.treeModel());
    ui->projectView->setRootIndex(mProjectRepo.treeModel()->rootModelIndex());
    mProjectRepo.setSuffixFilter(QStringList() << ".gms" << ".lst" << ".gdx");
    ui->projectView->setHeaderHidden(true);
    ui->projectView->setItemDelegate(new TreeItemDelegate(ui->projectView));
    ui->projectView->setIconSize(QSize(qRound(iconSize*0.8), qRound(iconSize*0.8)));
    ui->projectView->setContextMenuPolicy(Qt::CustomContextMenu);

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
    connect(&mProjectRepo, &ProjectRepo::fileChangedExtern, this, &MainWindow::fileChangedExtern);
    connect(&mProjectRepo, &ProjectRepo::fileDeletedExtern, this, &MainWindow::fileDeletedExtern);
    connect(&mProjectRepo, &ProjectRepo::openFile, this, &MainWindow::openFileNode);
    connect(&mProjectRepo, &ProjectRepo::setNodeExpanded, this, &MainWindow::setProjectNodeExpanded);
    connect(&mProjectRepo, &ProjectRepo::gamsProcessStateChanged, this, &MainWindow::gamsProcessStateChanged);
    connect(ui->projectView->selectionModel(), &QItemSelectionModel::currentChanged, &mProjectRepo, &ProjectRepo::setSelected);
    connect(ui->projectView, &QTreeView::customContextMenuRequested, this, &MainWindow::projectContextMenuRequested);
    connect(&mProjectContextMenu, &ProjectContextMenu::closeGroup, this, &MainWindow::closeGroup);
    connect(&mProjectContextMenu, &ProjectContextMenu::closeFile, this, &MainWindow::closeFileConditionally);
    connect(&mProjectContextMenu, &ProjectContextMenu::addExistingFile, this, &MainWindow::addToGroup);
    connect(&mProjectContextMenu, &ProjectContextMenu::getSourcePath, this, &MainWindow::sendSourcePath);
    connect(&mProjectContextMenu, &ProjectContextMenu::runFile, this, &MainWindow::on_runGmsFile);
    connect(&mProjectContextMenu, &ProjectContextMenu::setMainFile, this, &MainWindow::on_setMainGms);
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

    if (mSettings.get()->resetSettingsSwitch()) mSettings.get()->resetSettings();

    // stack help under output
    tabifyDockWidget(ui->dockHelpView, ui->dockLogView);

    mSyslog = new SystemLogEdit(this);
    mSyslog->setFont(QFont(mSettings->fontFamily(), mSettings->fontSize()));
    ui->logTabs->addTab(mSyslog, "System");

    initTabs();
//    updateFixedFonts(mSettings->fontFamily(), mSettings->fontSize());

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F12), this, SLOT(toggleLogDebug()));
}

void MainWindow::delayedFileRestoration()
{
    mSettings->restoreTabsAndProjects(this);
    mSettings.get()->restoreLastFilesUsed(this);
}

MainWindow::~MainWindow()
{
    killTimer(mTimerID);
    delete mWp;
    delete ui;
}

void MainWindow::initTabs()
{
    QPalette pal = ui->projectView->palette();
    pal.setColor(QPalette::Highlight, Qt::transparent);
    ui->projectView->setPalette(pal);

    mWp = new WelcomePage(history(), this);
    connect(mWp, &WelcomePage::linkActivated, this, &MainWindow::openFile);
    if (mSettings->skipWelcomePage())
        mWp->hide();
    else
        showWelcomePage();

}

void MainWindow::createEdit(QTabWidget *tabWidget, bool focus, int id, int codecMip)
{
    ProjectFileNode *fc = mProjectRepo.fileNode(id);
    if (fc) {
        int tabIndex;
        if (fc->metrics().fileType() == FileType::Gdx) {

            gdxviewer::GdxViewer* gdxView = new gdxviewer::GdxViewer(fc->location(), CommonPaths::systemDir(), this);
            ProjectAbstractNode::initEditorType(gdxView);
            fc->addEditor(gdxView);
            tabIndex = tabWidget->addTab(gdxView, fc->caption());
            fc->addFileWatcherForGdx();
        } else if (fc->metrics().fileType() == FileType::Ref) {
            qDebug() << "open ref:" << fc->location();
            ReferenceViewer* refView = new ReferenceViewer(fc->location(), this);
            ProjectAbstractNode::initEditorType(refView);
            fc->addEditor(refView);
            tabIndex = tabWidget->addTab(refView, fc->caption());
        } else {
            CodeEdit *codeEdit = new CodeEdit(this);
            codeEdit->setSettings(mSettings.get());
            codeEdit->setLineWrapMode(mSettings->lineWrapEditor() ? AbstractEdit::WidgetWidth : AbstractEdit::NoWrap);
            codeEdit->setTabChangesFocus(false);
            ProjectAbstractNode::initEditorType(codeEdit);
            codeEdit->setFont(QFont(mSettings->fontFamily(), mSettings->fontSize()));
            QFontMetrics metric(codeEdit->font());
            codeEdit->setTabStopDistance(8*metric.width(' '));
            if (fc->metrics().fileType() == FileType::Lst) {
                lxiviewer::LxiViewer* lxiViewer = new lxiviewer::LxiViewer(codeEdit, fc->location(), this);
                ProjectAbstractNode::initEditorType(lxiViewer);
                fc->addEditor(lxiViewer);
                connect(fc->parentEntry(), &ProjectGroupNode::gamsProcessStateChanged,
                        lxiViewer, &lxiviewer::LxiViewer::loadLxiFile);
                connect(fc->parentEntry(), &ProjectGroupNode::gamsProcessStateChanged,
                        lxiViewer, &lxiviewer::LxiViewer::loadLstFile);
                connect(lxiViewer->codeEdit(), &CodeEdit::searchFindNextPressed,
                        mSearchDialog, &SearchDialog::on_searchNext);
                connect(lxiViewer->codeEdit(), &CodeEdit::searchFindPrevPressed,
                        mSearchDialog, &SearchDialog::on_searchPrev);
                tabIndex = tabWidget->addTab(lxiViewer, fc->caption());
            } else {
                fc->addEditor(codeEdit);
                connect(codeEdit, &CodeEdit::searchFindNextPressed, mSearchDialog, &SearchDialog::on_searchNext);
                connect(codeEdit, &CodeEdit::searchFindPrevPressed, mSearchDialog, &SearchDialog::on_searchPrev);
                connect(codeEdit, &CodeEdit::requestAdvancedActions, this, &MainWindow::getAdvancedActions);
                tabIndex = tabWidget->addTab(codeEdit, fc->caption());
            }

            if (codecMip == -1)
                fc->load(encodingMIBs(), true);
            else
                fc->load(codecMip, true);

            if (fc->metrics().fileType() == FileType::Log ||
                    fc->metrics().fileType() == FileType::Lst ||
                    fc->metrics().fileType() == FileType::Ref) {

                codeEdit->setReadOnly(true);
                codeEdit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
            } else {
                connect(fc, &ProjectFileNode::changed, this, &MainWindow::fileChanged);
            }
            if (focus) updateMenuToCodec(fc->codecMib());

        }
        tabWidget->setTabToolTip(tabIndex, fc->location());
        if (focus) {
            tabWidget->setCurrentIndex(tabIndex);
            mRecent.setEditor(tabWidget->currentWidget(), this);
            mRecent.editFileId = fc->id();
        }
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    mAutosaveHandler->saveChangedFiles();
    mSettings->saveSettings(this);
}

void MainWindow::addToGroup(ProjectGroupNode* group, const QString& filepath)
{
    group->attachFile(filepath);
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

ProjectRepo *MainWindow::projectRepo()
{
    return &mProjectRepo;
}

QWidgetList MainWindow::openEditors()
{
    return mProjectRepo.editors();
}

QList<AbstractEdit*> MainWindow::openLogs()
{
    QList<AbstractEdit*> resList;
    for (int i = 0; i < ui->logTabs->count(); i++) {
        AbstractEdit* ed = ProjectFileNode::toAbstractEdit(ui->logTabs->widget(i));
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

    getHelpWidget()->on_urlOpened(result);

    on_actionHelp_View_triggered(true);
}

SearchDialog* MainWindow::searchDialog() const
{
    return mSearchDialog;
}

QString MainWindow::encodingMIBsString()
{
    QStringList res;
    foreach (QAction *act, ui->menuconvert_to->actions()) {
        if (!act->data().isNull()) res << act->data().toString();
    }
    return res.join(",");
}

QList<int> MainWindow::encodingMIBs()
{
    QList<int> res;
    foreach (QAction *act, mCodecGroupReload->actions())
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
    foreach (int mib, mibs) {
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

void MainWindow::updateEditorPos()
{
    QPoint pos;
    QPoint anchor;
    AbstractEdit* edit = ProjectFileNode::toAbstractEdit(mRecent.editor());
    CodeEdit *ce = ProjectFileNode::toCodeEdit(edit);
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
    CodeEdit* edit = ProjectAbstractNode::toCodeEdit(mRecent.editor());
    if (!edit || edit->isReadOnly()) {
        mStatusWidgets->setEditMode(EditMode::Readonly);
    } else {
        mStatusWidgets->setEditMode(edit->overwriteMode() ? EditMode::Overwrite : EditMode::Insert);
    }
}

void MainWindow::updateEditorBlockCount()
{
    AbstractEdit* edit = ProjectFileNode::toAbstractEdit(mRecent.editor());
    if (edit) mStatusWidgets->setLineCount(edit->blockCount());
}

void MainWindow::on_currentDocumentChanged(int from, int charsRemoved, int charsAdded)
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
        ProjectFileNode *fc = mProjectRepo.fileNode(mRecent.editFileId);
        if (fc) path = QFileInfo(fc->location()).path();
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
        fc->save();
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
    ProjectFileNode* fc = mProjectRepo.fileNode(mRecent.editFileId);
    if (!fc) return;
    if (fc->type() == ProjectAbstractNode::Log) {
        on_actionSave_As_triggered();
    } else if (fc->isModified()) {
        fc->save();
    }
}

void MainWindow::on_actionSave_As_triggered()
{
    QString path = mRecent.path;
    ProjectFileNode *formerFc;
    if (mRecent.editFileId >= 0) {
        formerFc = mProjectRepo.fileNode(mRecent.editFileId);
        if (formerFc) path = QFileInfo(formerFc->location()).path();
    }
    auto filePath = QFileDialog::getSaveFileName(this,
                                                 "Save file as...",
                                                 path,
                                                 tr("GAMS code (*.gms *.inc);;"
                                                    "Text files (*.txt);;"
                                                    "All files (*.*)"));
    if (!filePath.isEmpty()) {
        mRecent.path = QFileInfo(filePath).path();
        ProjectFileNode* fc = mProjectRepo.fileNode(mRecent.editFileId);
        if (!fc) return;

        if(fc->location().endsWith(".gms") && !filePath.endsWith(".gms")) {
            filePath = filePath + ".gms";
        } else if (fc->location().endsWith(".gdx") && !filePath.endsWith(".gdx")) {
            filePath = filePath + ".gdx";
        } else if (fc->location().endsWith(".lst") && !filePath.endsWith(".lst")) {
            filePath = filePath + ".lst";
        } // TODO: check if there are others to add


        // perform copy when file is a gdx file
        if (fc->metrics().fileType().kind() == FileType::Gdx) {
            if (QFile::exists(filePath))
                QFile::remove(filePath);
            QFile::copy(fc->location(), filePath);
        } else {
            fc->save(filePath);
            openFilePath(filePath, fc->parentEntry(), true);
            mStatusWidgets->setFileName(fc->location());
        }
    }
}

void MainWindow::on_actionSave_All_triggered()
{
    mProjectRepo.saveAll();
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
    ProjectFileNode *fc = mProjectRepo.fileNode(focusWidget());
    if (fc) {
        if (fc->document() && !fc->isReadOnly()) {
            fc->setCodecMib(action->data().toInt());
        }
        updateMenuToCodec(action->data().toInt());
        mStatusWidgets->setEncoding(fc->codecMib());
    }
}

void MainWindow::codecReload(QAction *action)
{
    if (!focusWidget()) return;
    ProjectFileNode *fc = mProjectRepo.fileNode(focusWidget());
    if (fc && fc->codecMib() != action->data().toInt()) {
        bool reload = true;
        if (fc->isModified()) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(fc->location()+" has been modified.");
            msgBox.setInformativeText("Do you want to discard your changes and reload it with Character Set "
                                      + action->text() + "?");
            msgBox.addButton(tr("Discard and Reload"), QMessageBox::ResetRole);
            msgBox.setStandardButtons(QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            reload = msgBox.exec();
        }
        if (reload) {
            fc->load(action->data().toInt(), true);
            updateMenuToCodec(action->data().toInt());
            mStatusWidgets->setEncoding(fc->codecMib());
        }
    }
}

void MainWindow::loadCommandLineOptions(ProjectFileNode* oldfn, ProjectFileNode* fn)
{
    if (oldfn) { // switch from a non-welcome page
        ProjectGroupNode* oldgroup = oldfn->parentEntry();
        if (!oldgroup) return;
        oldgroup->addRunParametersHistory( mGamsOptionWidget->getCurrentCommandLineData() );

        if (!fn) { // switch to a welcome page
            QStringList runParametersHistory;
            mGamsOptionWidget->loadCommandLineOption(runParametersHistory);
            return;
        }

        ProjectGroupNode* group = fn->parentEntry();
        if (!group) return;
        if (group == oldgroup) return;

        mGamsOptionWidget->loadCommandLineOption( group->getRunParametersHistory() );

    } else { // switch from a welcome page
        if (!fn) { // switch to a welcome page
            QStringList runParametersHistory;
            mGamsOptionWidget->loadCommandLineOption(runParametersHistory);
            return;
        }

        ProjectGroupNode* group = fn->parentEntry();
        if (!group) return;

        mGamsOptionWidget->loadCommandLineOption( group->getRunParametersHistory() );
    }
}

void MainWindow::activeTabChanged(int index)
{
    // remove highlights from old tab
    ProjectFileNode* oldTab = mProjectRepo.fileNode(mRecent.editor());
    if (oldTab)
        oldTab->removeTextMarks(TextMark::match, true);

    mRecent.setEditor(nullptr, this);
    QWidget *editWidget = (index < 0 ? nullptr : ui->mainTab->widget(index));
    AbstractEdit* edit = ProjectFileNode::toAbstractEdit(editWidget);
    lxiviewer::LxiViewer* lxiViewer = ProjectFileNode::toLxiViewer(editWidget);

    loadCommandLineOptions(oldTab, mProjectRepo.fileNode(editWidget));
    updateRunState();

    if (edit) {
        ProjectFileNode* fc = mProjectRepo.fileNode(lxiViewer ? editWidget : edit);

        if (fc) {
            mRecent.editFileId = fc->id();
            mRecent.setEditor(lxiViewer ? editWidget : edit, this);
            mRecent.group = fc->parentEntry();
            if (!edit->isReadOnly()) {
                ui->menuEncoding->setEnabled(true);
            }
            updateMenuToCodec(fc->codecMib());
            mStatusWidgets->setFileName(fc->location());
            mStatusWidgets->setEncoding(fc->codecMib());
            mStatusWidgets->setLineCount(edit->blockCount());
        } else {
            mStatusWidgets->setFileName("");
            mStatusWidgets->setEncoding(-1);
            mStatusWidgets->setLineCount(-1);
        }
        ui->menuEncoding->setEnabled(fc && !edit->isReadOnly());
    } else if (ProjectFileNode::toGdxViewer(editWidget)) {
        ui->menuEncoding->setEnabled(false);
        gdxviewer::GdxViewer* gdxViewer = ProjectFileNode::toGdxViewer(editWidget);
        mRecent.setEditor(gdxViewer, this);
        ProjectFileNode* fc = mProjectRepo.fileNode(gdxViewer);
        if (fc) {
            mRecent.editFileId = fc->id();
            mRecent.group = fc->parentEntry();
            mStatusWidgets->setFileName(fc->location());
            mStatusWidgets->setEncoding(fc->codecMib());
            mStatusWidgets->setLineCount(-1);
            gdxViewer->reload();
        }
    } else {
        ui->menuEncoding->setEnabled(false);
        mStatusWidgets->setFileName("");
        mStatusWidgets->setEncoding(-1);
        mStatusWidgets->setLineCount(-1);
    }

    if (searchDialog()) searchDialog()->updateReplaceActionAvailability();

    CodeEdit* ce = ProjectAbstractNode::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly()) ce->setOverwriteMode(mOverwriteMode);
    updateEditorMode();
}

void MainWindow::fileChanged(FileId fileId)
{
    QWidgetList editors = mProjectRepo.editors(fileId);
    for (QWidget *edit: editors) {
        int index = ui->mainTab->indexOf(edit);
        if (index >= 0) {
            ProjectFileNode *fc = mProjectRepo.fileNode(fileId);
            if (fc) ui->mainTab->setTabText(index, fc->caption());
        }
    }
}

void MainWindow::fileChangedExtern(FileId fileId)
{
    ProjectFileNode *fc = mProjectRepo.fileNode(fileId);

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
    ProjectFileNode *fc = mProjectRepo.fileNode(fileId);
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
        closeFile(fc);
    else
        fc->document()->setModified();
}

void MainWindow::appendSystemLog(const QString &text)
{
    mSyslog->appendLog(text, LogMsgType::Info);
}

void MainWindow::postGamsRun(AbstractProcess* process)
{
    ProjectGroupNode* groupNode = mProjectRepo.groupNode(process->groupId());
    QFileInfo fileInfo(process->inputFile());
    if(groupNode && fileInfo.exists()) {
        QString lstFile = groupNode->lstFileName();
//        appendErrData(fileInfo.path() + "/" + fileInfo.completeBaseName() + ".err");
        bool doFocus = groupNode == mRecent.group;

        if (mSettings->jumpToError())
            groupNode->jumpToFirstError(doFocus);

        ProjectFileNode* lstNode = nullptr;
        mProjectRepo.findOrCreateFileNode(lstFile, lstNode, groupNode);

        if (lstNode) lstNode->updateMarks();

        if (mSettings->openLst())
            openFileNode(lstNode, true);
    }
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
    openFileNode(fc);
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
        CodeEdit* ce = ProjectFileNode::toCodeEdit(mRecent.editor());
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
    ProjectFileNode* fc = mProjectRepo.fileNode(widget);
    if (!fc) {
        // assuming we are closing a welcome page here
        ui->mainTab->removeTab(index);
        mClosedTabs << "Wp Closed";
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
        ProjectLogNode* log = mProjectRepo.logNode(edit);
        if (log) log->removeEditor(edit);
        ui->logTabs->removeTab(index);
        AbstractEdit* ed = ProjectAbstractNode::toAbstractEdit(edit);
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
       ProjectFileNode* fc = mProjectRepo.fileNode(editWidget);
       if (!fc) { // assuming a welcome page here
           return false;
       } else {
           return true;
       }
    }
    return false;
}

bool MainWindow::isRecentGroupInRunningState()
{
    QProcess::ProcessState state = mRecent.group ? mRecent.group->gamsProcessState() : QProcess::NotRunning;
    return (state == QProcess::Running);
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

void MainWindow::renameToBackup(QFile *file)
{
    const int MAX_BACKUPS = 3;
    ProjectAbstractNode *fsc = mProjectRepo.findNode(file->fileName());
    if (fsc) {
        ProjectFileNode *fc = mProjectRepo.fileNode(fsc->id());
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

void MainWindow::triggerGamsLibFileCreation(LibraryItem *item)
{
    openModelFromLib(item->library()->glbFile(), item);
}

void MainWindow::openFile(const QString &filePath)
{
    openFilePath(filePath, nullptr, true, -1);
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
    ProjectAbstractNode* fsc = mProjectRepo.node(index);
    if (fsc->type() == ProjectAbstractNode::FileGroup) {
        ProjectLogNode* logProc = mProjectRepo.logNode(fsc);
        if (logProc->editors().isEmpty()) {
            logProc->setDebugLog(mLogDebugLines);
            ProcessLogEdit* logEdit = new ProcessLogEdit(this);
            logEdit->setLineWrapMode(mSettings->lineWrapProcess() ? AbstractEdit::WidgetWidth : AbstractEdit::NoWrap);
            ProjectAbstractNode::initEditorType(logEdit);
            int ind = ui->logTabs->addTab(logEdit, logProc->caption());
            logProc->addEditor(logEdit);
            ui->logTabs->setCurrentIndex(ind);
        }
    } else {
        openNode(index);
    }
}

bool MainWindow::requestCloseChanged(QList<ProjectFileNode*> changedFiles)
{
    if (changedFiles.size() <= 0) return true;

    int ret = QMessageBox::Discard;
    QMessageBox msgBox;
    QString filesText = changedFiles.size()==1 ? changedFiles.first()->location() + " has been modified."
                                         : QString::number(changedFiles.size())+" files have been modified";
    ret = showSaveChangesMsgBox(filesText);
    if (ret == QMessageBox::Save) {
        mAutosaveHandler->clearAutosaveFiles(mOpenTabsList);
        for (ProjectFileNode* fc : changedFiles) {
            if (fc->isModified()) {
                fc->save();
            }
        }
    } else if (ret == QMessageBox::Cancel) {
        return false;
    } else { // Discard
        mAutosaveHandler->clearAutosaveFiles(mOpenTabsList);
        for (ProjectFileNode* fc : changedFiles) {
            if (fc->isModified()) {
                closeFile(fc);
            }
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
    mSettings->saveSettings(this);
    QList<ProjectFileNode*> oFiles = mProjectRepo.modifiedFiles();
    if (requestCloseChanged(oFiles)) {
        on_actionClose_All_triggered();
        closeHelpView();
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
            openFilePath(CommonPaths::absolutFilePath(fName), nullptr, true);
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

QStringList MainWindow::parseFilesFromCommandLine(const QString &commandLineStr, ProjectGroupNode* fgc)
{
    QList<OptionItem> items = mGamsOptionWidget->getGamsOptionTokenizer()->tokenize( commandLineStr );
    QStringList commandLineArgs;
    // set default lst file name in case output option changed back to default
    if (!fgc->runnableGms().isEmpty())
        fgc->setLstFileName(QFileInfo(fgc->runnableGms()).completeBaseName() + ".lst");

    foreach (OptionItem item, items) {
        commandLineArgs << QString("%1=%2").arg(item.key).arg(item.value);
        // output (o) found, case-insensitive
        if (QString::compare(item.key, "o", Qt::CaseInsensitive) == 0
                || QString::compare(item.key, "output", Qt::CaseInsensitive) == 0) {

            fgc->setLstFileName(item.value);
        }
    }
    return commandLineArgs;
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

OptionWidget *MainWindow::getGamsOptionWidget() const
{
    return mGamsOptionWidget;
}

void MainWindow::ensureLogEditor(ProjectLogNode* logProc)
{
    if (!logProc->editors().isEmpty()) return;
    logProc->setDebugLog(mLogDebugLines);
    ProcessLogEdit* logEdit = new ProcessLogEdit(this);
    logEdit->setLineWrapMode(mSettings->lineWrapProcess() ? AbstractEdit::WidgetWidth : AbstractEdit::NoWrap);
    ProjectAbstractNode::initEditorType(logEdit);

    ui->logTabs->addTab(logEdit, logProc->caption());
    logProc->addEditor(logEdit);
    logEdit->setFont(QFont(mSettings->fontFamily(), mSettings->fontSize()));
}

void MainWindow::execute(QString commandLineStr, ProjectFileNode* gmsFileNode)
{
    ProjectFileNode* fc = (gmsFileNode ? gmsFileNode : mProjectRepo.fileNode(mRecent.editor()));
    ProjectGroupNode *group = (fc ? fc->parentEntry() : nullptr);
    if (!group) return;

    QStringList commandLineArgs = parseFilesFromCommandLine(commandLineStr, group);

    group->addRunParametersHistory( mGamsOptionWidget->getCurrentCommandLineData() );
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

    mProjectRepo.removeMarks(group);
    ProjectLogNode* logProc = mProjectRepo.logNode(group);

    ensureLogEditor(logProc);

    if (!mSettings->clearLog()) {
        logProc->markOld();
    } else {
        logProc->clearLog();
    }
    if (!ui->logTabs->children().contains(logProc->editors().first())) {
        ui->logTabs->addTab(logProc->editors().first(), logProc->caption());
    }
    ui->logTabs->setCurrentWidget(logProc->editors().first());

    ui->dockLogView->setVisible(true);
    QString gmsFilePath = (gmsFileNode ? gmsFileNode->location() : group->runnableGms());
    if (gmsFilePath == "") {
        mSyslog->appendLog("No runnable GMS file found in group ["+group->name()+"].", LogMsgType::Warning);
        ui->actionShow_System_Log->trigger();
        return;
    }
    QString workDir = gmsFileNode ? QFileInfo(gmsFilePath).path() : group->location();
    logProc->setJumpToLogEnd(true);

    GamsProcess* process = group->gamsProcess();
    process->setGroupId(group->id());
    process->setWorkingDir(workDir);
    process->setInputFile(gmsFilePath);
    process->setArguments( commandLineArgs );
    process->execute();

    connect(process, &GamsProcess::newStdChannelData, logProc, &ProjectLogNode::addProcessData, Qt::UniqueConnection);
    connect(process, &GamsProcess::finished, this, &MainWindow::postGamsRun, Qt::UniqueConnection);

    ui->dockLogView->raise();
}

void MainWindow::updateRunState()
{
    mGamsOptionWidget->updateRunState(isActiveTabRunnable(), isRecentGroupInRunningState());
}

HelpWidget *MainWindow::getHelpWidget() const
{
    return mHelpWidget;
}

void MainWindow::on_runGmsFile(ProjectFileNode *fc)
{
    execute("", fc);
}

void MainWindow::on_setMainGms(ProjectFileNode *fc)
{
    fc->parentEntry()->setRunnableGms(fc);
    updateRunState();
}

void MainWindow::on_commandLineHelpTriggered()
{
    mHelpWidget->on_helpContentRequested(HelpWidget::GAMSCALL_CHAPTER, "");
    if (ui->dockHelpView->isHidden())
        ui->dockHelpView->show();
    if (tabifiedDockWidgets(ui->dockHelpView).count())
        ui->dockHelpView->raise();
}

void MainWindow::on_optionRunChanged()
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
    ProjectFileNode* fc = mProjectRepo.fileNode(mRecent.editor());
    ProjectGroupNode *group = (fc ? fc->parentEntry() : nullptr);
    if (!group)
        return;
    mGamsOptionWidget->on_interruptAction();
    GamsProcess* process = group->gamsProcess();
    QtConcurrent::run(process, &GamsProcess::interrupt);
}

void MainWindow::on_actionStop_triggered()
{
    ProjectFileNode* fc = mProjectRepo.fileNode(mRecent.editor());
    ProjectGroupNode *group = (fc ? fc->parentEntry() : nullptr);
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
        ensureLogEditor(logNode);
    }
    if (!logNode->editors().isEmpty()) {
        logNode->setDebugLog(mLogDebugLines);
        AbstractEdit* logEdit = ProjectFileNode::toAbstractEdit(logNode->editors().first());
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

void MainWindow::openFileNode(ProjectFileNode* fileNode, bool focus, int codecMib)
{
    if (!fileNode) return;
    QWidget* edit = nullptr;
    QTabWidget* tabWidget = fileNode->type() == ProjectAbstractNode::Log ? ui->logTabs : ui->mainTab;
    if (!fileNode->editors().empty()) {
        edit = fileNode->editors().first();
    }
    // open edit if existing or create one
    if (edit) {
        if (focus) tabWidget->setCurrentWidget(edit);
    } else {
        createEdit(tabWidget, focus, fileNode->id(), codecMib);
    }
    // set keyboard focus to editor
    if (tabWidget->currentWidget())
        if (focus) {
            lxiviewer::LxiViewer* lxiViewer = ProjectAbstractNode::toLxiViewer(edit);
            if (lxiViewer)
                lxiViewer->codeEdit()->setFocus();
            else
                tabWidget->currentWidget()->setFocus();
        }
    if (tabWidget != ui->logTabs) {
        // if there is already a log -> show it
        changeToLog(fileNode);
    }
    addToOpenedFiles(fileNode->location());
}

void MainWindow::closeGroup(ProjectGroupNode* group)
{
    if (!group) return;
    QList<ProjectFileNode*> changedFiles;
    QList<ProjectFileNode*> openFiles;
    for (int i = 0; i < group->childCount(); ++i) {
        ProjectAbstractNode* fsc = group->childEntry(i);
        if (fsc->type() == ProjectAbstractNode::File) {
            ProjectFileNode* file = static_cast<ProjectFileNode*>(fsc);
            openFiles << file;
            if (file->isModified())
                changedFiles << file;
        }
    }
    if (requestCloseChanged(changedFiles)) {
        // TODO(JM)  close if selected
        for (ProjectFileNode *file: openFiles) {
            closeFileEditors(file->id());
        }
        ProjectLogNode* log = group->logNode();
        if (log) {
            QWidget* edit = log->editors().isEmpty() ? nullptr : log->editors().first();
            if (edit) {
                log->removeEditor(edit);
                int index = ui->logTabs->indexOf(edit);
                if (index >= 0) ui->logTabs->removeTab(index);
            }
        }

        mProjectRepo.removeGroup(group);
    }
}

/// Asks user for confirmation if a file is modified before calling closeFile
/// \param file
///
void MainWindow::closeFileConditionally(ProjectFileNode* file) {
    if (!file->isModified() || requestCloseChanged(QList<ProjectFileNode*>() << file))
        closeFile(file);
}

/// Removes file from repository. And calls closeFileEditors to clean everything up.
/// \param file
///
void MainWindow::closeFile(ProjectFileNode* file)
{
    ui->projectView->setCurrentIndex(QModelIndex());

    ProjectGroupNode *parentGroup = file->parentEntry();

    // if this is a lst file referenced in a log
    if (parentGroup->logNode() && parentGroup->logNode()->lstNode() == file)
        parentGroup->logNode()->setLstNode(nullptr);

    // close actual file and remove repo node
    closeFileEditors(file->id());
    QString filePath = file->location();
    mProjectRepo.removeFile(file);

    // if this file is marked as runnable remove reference
    if (parentGroup->runnableGms() == filePath) {
        parentGroup->removeRunnableGms();
        for (int i = 0; i < parentGroup->childCount(); i++) {
            // choose next as main gms file
            QFileInfo fi(parentGroup->childEntry(i)->location());
            if (FileType::from(fi.suffix()) == FileType::Gms) {
                parentGroup->setRunnableGms(static_cast<ProjectFileNode*>(parentGroup->childEntry(i)));
                break;
            }
        }
    }

    // close group if empty now
    if (parentGroup->childCount() == 0)
        closeGroup(parentGroup);
}

/// Closes all open editors and tabs related to a file and remove option history
/// \param fileId
///
void MainWindow::closeFileEditors(FileId fileId)
{
    ProjectFileNode* fc = mProjectRepo.fileNode(fileId);
    if (!fc) return; // TODO(AF) add logging but no execption

    // add to recently closed tabs
    mClosedTabs << fc->location();

    // close all related editors, tabs and clean up
    while (!fc->editors().isEmpty()) {
        QWidget *edit = fc->editors().first();
        ui->mainTab->removeTab(ui->mainTab->indexOf(edit));
        fc->removeEditor(edit);
        edit->deleteLater();
    }
}

void MainWindow::openFilePath(QString filePath, ProjectGroupNode *parent, bool focus, int codecMip)
{
    if (!QFileInfo(filePath).exists()) {
        EXCEPT() << "File not found: " << filePath;
    }
    ProjectAbstractNode *fsc = mProjectRepo.findNode(filePath, parent);
    ProjectFileNode *fileNode = (fsc && fsc->type() == ProjectAbstractNode::File) ? static_cast<ProjectFileNode*>(fsc) : nullptr;

    if (!fileNode) { // not yet opened by user, open file in new tab
        ProjectGroupNode* group = mProjectRepo.ensureGroup(CommonPaths::absolutFilePath(filePath));
        mProjectRepo.findOrCreateFileNode(filePath, fileNode, group);
        if (!fileNode) {
            EXCEPT() << "File not found: " << filePath;
        }
        QTabWidget* tabWidget = (fileNode->type() == ProjectAbstractNode::Log) ? ui->logTabs : ui->mainTab;
        createEdit(tabWidget, focus, fileNode->id(), codecMip);
        if (tabWidget->currentWidget())
            if (focus) tabWidget->currentWidget()->setFocus();
        ui->projectView->expand(mProjectRepo.treeModel()->index(group));
        addToOpenedFiles(filePath);
        mGamsOptionWidget->loadCommandLineOption( group->getRunParametersHistory() );
    } else {
        openFileNode(fileNode, focus, codecMip);
    }
    if (!fileNode) {
        EXCEPT() << "invalid pointer found: FileNode expected.";
    }
    mRecent.path = filePath;
    mRecent.group = fileNode->parentEntry();
}

ProjectFileNode* MainWindow::addNode(const QString &path, const QString &fileName)
{
    ProjectFileNode *fc = nullptr;
    if (!fileName.isEmpty()) {
        QFileInfo fInfo(path, fileName);

        FileType fType = FileType::from(fInfo.suffix());

        if (fType == FileType::Gsp) {
            // TODO(JM) Read project and create all nodes for associated files
        } else {
            openFilePath(fInfo.filePath(), nullptr, true); // open all sorts of files
        }
    }
    return fc;
}

void MainWindow::openNode(const QModelIndex& index)
{
    ProjectFileNode *file = mProjectRepo.fileNode(index);
    if (file) openFileNode(file);
}

void MainWindow::on_mainTab_currentChanged(int index)
{
    QWidget* edit = ui->mainTab->widget(index);
    if (!edit) return;

    mProjectRepo.editorActivated(edit);
    ProjectFileNode* fc = mProjectRepo.fileNode(edit);
    if (fc && mRecent.group != fc->parentEntry()) {
        mRecent.group = fc->parentEntry();
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
    if (ui->dockHelpView->isAncestorOf(QApplication::focusWidget()) ||
        ui->dockHelpView->isAncestorOf(QApplication::activeWindow())) {
        mHelpWidget->on_searchHelp();
    } else {
       ProjectFileNode *fc = mProjectRepo.fileNode(mRecent.editor());
       if (fc && fc->metrics().fileType() == FileType::Gdx) {
           gdxviewer::GdxViewer *gdx = ProjectFileNode::toGdxViewer(mRecent.editor());
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

           if (ui->mainTab->currentWidget()) {
               int sbs;
               if (mRecent.editor() && ProjectFileNode::toAbstractEdit(mRecent.editor())
                       && ProjectFileNode::toAbstractEdit(mRecent.editor())->verticalScrollBar()->isVisible())
                   sbs = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2;
               else
                   sbs = 2;

               int offset = (this->width() - mSearchDialog->width() - sbs);
               mSearchDialog->move(newP.x() + offset, newP.y());
           }
           mSearchDialog->show();
       }
    }
}

void MainWindow::showResults(SearchResultList &results)
{
    int index = ui->logTabs->indexOf(mResultsView); // did widget exist before?

    mResultsView = new ResultsView(results, this);
    QString title("Results: " + mSearchDialog->searchTerm());

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
    foreach (QWidget* edit, openEditors()) {
        if (!ProjectFileNode::toGdxViewer(edit))
            ProjectFileNode::toAbstractEdit(edit)->setFont(font);
    }
    foreach (QWidget* log, openLogs())
        log->setFont(font);

    mSyslog->setFont(font);
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
        AbstractEdit* ed = ProjectFileNode::toAbstractEdit(editList.at(i));
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

    QList<AbstractEdit*> logList = openLogs();
    for (int i = 0; i < logList.size(); i++) {
        if (logList.at(i))
            logList.at(i)->setLineWrapMode(wrapModeProcess);
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
                    openFilePath(location, nullptr, true, mib);
                    mOpenTabsList << location;
                }
                QApplication::processEvents();
            }
        }
    }
    if (json.contains("mainTabRecent")) {
        QString location = json["mainTabRecent"].toString();
        if (QFileInfo(location).exists()) {
            openFilePath(location, nullptr, true);
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
        ProjectFileNode *fc = mProjectRepo.fileNode(wid);
        if (!fc) continue;
        QJsonObject tabObject;
        tabObject["location"] = fc->location();
        tabObject["codecMib"] = fc->codecMib();
        // TODO(JM) store current edit position
        tabArray.append(tabObject);
    }
    json["mainTabs"] = tabArray;

    ProjectFileNode *fc = mRecent.editor() ? mProjectRepo.fileNode(mRecent.editor()) : nullptr;
    if (fc)
        json["mainTabRecent"] = fc->location();
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
    CodeEdit *codeEdit = ProjectFileNode::toCodeEdit(mRecent.editor());
    if (codeEdit)
        codeEdit->jumpTo(QTextCursor(), dialog.lineNumber());
}

void MainWindow::on_actionRedo_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce = ProjectFileNode::toCodeEdit(mRecent.editor());
    if (ce) ce->extendedRedo();
}

void MainWindow::on_actionUndo_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce = ProjectFileNode::toCodeEdit(mRecent.editor());
    if (ce) ce->extendedUndo();
}

void MainWindow::on_actionPaste_triggered()
{
    CodeEdit *ce = ProjectFileNode::toCodeEdit(focusWidget());
    if (!ce || ce->isReadOnly()) return;
    ce->pasteClipboard();
}

void MainWindow::on_actionCopy_triggered()
{
    if (!focusWidget()) return;

    ProjectFileNode *fc = mProjectRepo.fileNode(mRecent.editor());
    if (!fc) return;

    if (fc->metrics().fileType() == FileType::Gdx) {
        gdxviewer::GdxViewer *gdx = ProjectFileNode::toGdxViewer(mRecent.editor());
        gdx->copyAction();
    } else if (focusWidget() == mSyslog) {
        mSyslog->copy();
    } else {
        AbstractEdit *ae = ProjectFileNode::toAbstractEdit(focusWidget());
        if (!ae) return;
        CodeEdit *ce = ProjectFileNode::toCodeEdit(ae);
        if (ce && ce->blockEdit()) {
            ce->blockEdit()->selectionToClipboard();
        } else {
            ae->copy();
        }
    }
}

void MainWindow::on_actionSelect_All_triggered()
{
    ProjectFileNode *fc = mProjectRepo.fileNode(mRecent.editor());
    if (!fc || !focusWidget()) return;

    if (fc->metrics().fileType() == FileType::Gdx) {
        gdxviewer::GdxViewer *gdx = ProjectFileNode::toGdxViewer(mRecent.editor());
        gdx->selectAllAction();
    } else if (focusWidget() == mSyslog) {
        mSyslog->selectAll();
    } else {
        AbstractEdit *ae = ProjectFileNode::toAbstractEdit(focusWidget());
        if (!ae) return;
        ae->selectAll();
    }
}

void MainWindow::on_actionCut_triggered()
{
    CodeEdit* ce= ProjectFileNode::toCodeEdit(focusWidget());
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
    if (getHelpWidget()->isAncestorOf(QApplication::focusWidget()) ||
        getHelpWidget()->isAncestorOf(QApplication::activeWindow())) {
        getHelpWidget()->resetZoom(); // reset help view
    } else {
        updateFixedFonts(mSettings->fontFamily(), mSettings->fontSize()); // reset all editors
    }

}

void MainWindow::on_actionZoom_Out_triggered()
{
    if (getHelpWidget()->isAncestorOf(QApplication::focusWidget()) ||
        getHelpWidget()->isAncestorOf(QApplication::activeWindow())) {
        getHelpWidget()->zoomOut();
    } else {
        AbstractEdit *ae = ProjectFileNode::toAbstractEdit(QApplication::focusWidget());
        if (ae) {
            int pix = ae->fontInfo().pixelSize();
            if (pix == ae->fontInfo().pixelSize()) ae->zoomOut();
        }
    }
}

void MainWindow::on_actionZoom_In_triggered()
{
    if (getHelpWidget()->isAncestorOf(QApplication::focusWidget()) ||
        getHelpWidget()->isAncestorOf(QApplication::activeWindow())) {
        getHelpWidget()->zoomIn();
    } else {
        AbstractEdit *ae = ProjectFileNode::toAbstractEdit(QApplication::focusWidget());
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
    CodeEdit* ce= ProjectFileNode::toCodeEdit(mRecent.editor());
    if (ce) ce->convertToUpper();
}

void MainWindow::on_actionSet_to_Lowercase_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce = ProjectFileNode::toCodeEdit(mRecent.editor());
    if (ce) ce->convertToLower();
}

void MainWindow::on_actionOverwrite_Mode_toggled(bool overwriteMode)
{
    CodeEdit* ce = ProjectFileNode::toCodeEdit(mRecent.editor());
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

    CodeEdit* ce = ProjectFileNode::toCodeEdit(mRecent.editor());
    if (!ce || ce->isReadOnly()) return;
    QPoint pos(-1,-1); QPoint anc(-1,-1);
    ce->getPositionAndAnchor(pos, anc);
    ce->indent(mSettings->tabSize(), pos.y()-1, anc.y()-1);
}

void MainWindow::on_actionOutdent_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEdit* ce = ProjectFileNode::toCodeEdit(mRecent.editor());
    if (!ce || ce->isReadOnly()) return;
    QPoint pos(-1,-1); QPoint anc(-1,-1);
    ce->getPositionAndAnchor(pos, anc);
    ce->indent(-mSettings->tabSize(), pos.y()-1, anc.y()-1);
}

void MainWindow::on_actionDuplicate_Line_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEdit* ce = ProjectFileNode::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly())
        ce->duplicateLine();
}

void MainWindow::on_actionRemove_Line_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEdit* ce = ProjectFileNode::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly())
        ce->removeLine();
}

void MainWindow::on_actionComment_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;

    CodeEdit* ce = ProjectFileNode::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly())
        ce->commentLine();
}

void MainWindow::toggleLogDebug()
{
    mLogDebugLines = !mLogDebugLines;
    ProjectGroupNode* root = mProjectRepo.treeModel()->rootNode();
    for (int i = 0; i < root->childCount(); ++i) {
        ProjectAbstractNode *fsc = root->childEntry(i);
        if (fsc->type() == ProjectAbstractNode::FileGroup) {
            ProjectGroupNode* group = static_cast<ProjectGroupNode*>(fsc);
            ProjectLogNode* log = group->logNode();
            if (log) log->setDebugLog(mLogDebugLines);
        }
    }
}

void MainWindow::on_actionRestore_Recently_Closed_Tab_triggered()
{
    // TODO: remove duplicates?
    if (mClosedTabs.isEmpty())
        return;

    if (mClosedTabs.last()=="Wp Closed") {
        mClosedTabs.removeLast();
        showWelcomePage();
        return;
    }
    QFile file(mClosedTabs.last());
    mClosedTabs.removeLast();
    if (file.exists())
        openFile(file.fileName());
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
    AbstractEdit* edit = ProjectFileNode::toAbstractEdit(mEditor);
    if (edit) {
        MainWindow::disconnect(edit, &AbstractEdit::cursorPositionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::disconnect(edit, &AbstractEdit::selectionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::disconnect(edit, &AbstractEdit::blockCountChanged, window, &MainWindow::updateEditorBlockCount);
        MainWindow::disconnect(edit->document(), &QTextDocument::contentsChange, window, &MainWindow::on_currentDocumentChanged);
    }
    mEditor = editor;
    edit = ProjectFileNode::toAbstractEdit(mEditor);
    if (edit) {
        MainWindow::connect(edit, &AbstractEdit::cursorPositionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::connect(edit, &AbstractEdit::selectionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::connect(edit, &AbstractEdit::blockCountChanged, window, &MainWindow::updateEditorBlockCount);
        MainWindow::connect(edit->document(), &QTextDocument::contentsChange, window, &MainWindow::on_currentDocumentChanged);
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
    foreach (QDockWidget* dock, dockWidgets) {
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


}
}
