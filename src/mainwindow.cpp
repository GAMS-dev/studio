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
#include "searchwidget.h"
#include "option/optionwidget.h"
//#include "option/commandlinehistory.h"
#include "option/lineeditcompleteevent.h"
#include "searchresultlist.h"
#include "resultsview.h"
#include "gotodialog.h"
#include "updatedialog.h"
#include "checkforupdatewrapper.h"
#include "autosavehandler.h"
#include "distributionvalidator.h"
#include "syntax/textmarkrepo.h"
#include "help/helpwidget.h"
#include "statuswidgets.h"

namespace gams {
namespace studio {

MainWindow::MainWindow(StudioSettings *settings, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      mFileMetaRepo(this, settings),
      mProjectRepo(this),
      mTextMarkRepo(&mFileMetaRepo, &mProjectRepo, this),
      mSettings(settings),
      mAutosaveHandler(new AutosaveHandler(this))
{
    mHistory = new HistoryData();
//    QFile css(":/data/style.css");
//    if (css.open(QFile::ReadOnly | QFile::Text)) {
//        this->setStyleSheet(css.readAll());
//    }

    ui->setupUi(this);

    mFileTimer.setSingleShot(true);
    connect(&mFileTimer, &QTimer::timeout, this, &MainWindow::processFileEvents);
    setAcceptDrops(true);
    mTimerID = startTimer(60000);
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
    ui->projectView->setContextMenuPolicy(Qt::CustomContextMenu);

//    mTextMarkRepo = new TextMarkRepo(&mProjectRepo, this);
    mProjectRepo.init(&mFileMetaRepo, &mTextMarkRepo);
    mFileMetaRepo.init(&mTextMarkRepo, &mProjectRepo);

    // TODO(JM) it is possible to put the QTabBar into the docks title:
    //          if we override the QTabWidget it should be possible to extend it over the old tab-bar-space
//    ui->dockLogView->setTitleBarWidget(ui->tabLog->tabBar());

    mHelpWidget = new HelpWidget(this);
    ui->dockHelpView->setWidget(mHelpWidget);
    ui->dockHelpView->show();

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
    connect(&mProjectContextMenu, &ProjectContextMenu::closeFile, this, &MainWindow::closeNodeConditionally);
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
    mSearchWidget = new SearchWidget(this);

    if (mSettings.get()->resetSettingsSwitch()) mSettings.get()->resetSettings();

    mSyslog = new SystemLogEdit(this);
    mSyslog->setFont(QFont(mSettings->fontFamily(), mSettings->fontSize()));
    ui->logTabs->addTab(mSyslog, "System");

    initTabs();
//    updateFixedFonts(mSettings->fontFamily(), mSettings->fontSize());

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F12), this, SLOT(toggleLogDebug()));


    mSyslog->appendLog("Link: https://gams.com", LogMsgType::Info);
    mSyslog->appendLog("This is a placehoder message. say hi", LogMsgType::Error);
    mSyslog->appendLog("This is another message of high importance", LogMsgType::Warning);
    mSyslog->appendLog("And this one is very long. And this one is very long. https://doc.qt.io/qt-5/qsyntaxhighlighter.html And this one is very long. And this one is very long. And this one is very long. And this one is very long. And this one is very long. And this one is very long. And this one is very long. And this one is very long. And this one is very long. And this one is very long. And this one is very long. And this one is very long. And this one is very long. And this one is very long.", LogMsgType::Info);
}

void MainWindow::delayedFileRestoration()
{
    mSettings->restoreTabsAndProjects(this);
    mSettings.get()->restoreLastFilesUsed(this);
}

MainWindow::~MainWindow()
{
    killTimer(mTimerID);
    delete ui;
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
    visibility = visibility || tabifiedDockWidgets(ui->dockOptionEditor).count();
    ui->actionOption_View->setChecked(visibility);
}

void MainWindow::setHelpViewVisibility(bool visibility)
{
    visibility = visibility || tabifiedDockWidgets(ui->dockHelpView).count();
    if (!visibility)
        mHelpWidget->clearStatusBar();
    else
        mHelpWidget->setFocus();
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

void MainWindow::openModelFromLib(QString glbFile, QString model, QString gmsFileName)
{
    if (gmsFileName.isEmpty())
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
    QString link = CommonPaths::systemDir() + "/" + doc;
    QUrl result = QUrl::fromLocalFile(link);

    if (!anchor.isEmpty())
        result = QUrl(result.toString() + "#" + anchor);

    getHelpWidget()->on_urlOpened(result);

    on_actionHelp_View_triggered(true);
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

        // perform copy when file is a gdx file
        if (fileMeta->kind() == FileKind::Gdx) {
            if (QFile::exists(filePath))
                QFile::remove(filePath);
            QFile::copy(fileMeta->location(), filePath);
        } else {
           fileMeta->saveAs(filePath);
           ProjectRunGroupNode* runGroup = mRecent.group ? mRecent.group->runParentNode() : nullptr;
           openFile(fileMeta, true, runGroup);
           mStatusWidgets->setFileName(fileMeta->location());
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
            fm->load(action->data().toInt());
            updateMenuToCodec(action->data().toInt());
            mStatusWidgets->setEncoding(fm->codecMib());
        }
    }
}

void MainWindow::loadCommandLineOptions(ProjectAbstractNode* node)
{
    if (!node) return;
    FileMeta *file = nullptr;
    ProjectRunGroupNode *runGroup = node->toRunGroup();
    if (runGroup) file = runGroup->runnableGms();
    if (!file && node->toFile()) file = node->toFile()->file();

    if (file) emit mGamsOptionWidget->loadCommandLineOption(file->location());
}

void MainWindow::activeTabChanged(int index)
{
    emit mGamsOptionWidget->optionEditorDisabled();

    // remove highlights from old tab
    if (mRecent.editFileId >= 0)
        mTextMarkRepo.removeMarks(mRecent.editFileId, QSet<TextMark::Type>() << TextMark::match);

    mRecent.setEditor(nullptr, this);
    QWidget *editWidget = (index < 0 ? nullptr : ui->mainTab->widget(index));

    AbstractEdit* edit = FileMeta::toAbstractEdit(editWidget);
    gdxviewer::GdxViewer *gdxViewer = FileMeta::toGdxViewer(editWidget);
    ProjectFileNode *node = mProjectRepo.findFileNode(editWidget);

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
                loadCommandLineOptions(node);
                updateRunState();
                ui->menuEncoding->setEnabled(true);
            }
            updateMenuToCodec(node->file()->codecMib());
            mStatusWidgets->setLineCount(edit->blockCount());
            ui->menuEncoding->setEnabled(node && !edit->isReadOnly());
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

    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly()) ce->setOverwriteMode(mOverwriteMode);
    updateEditorMode();
}

void MainWindow::fileChanged(FileId fileId)
{
    QWidgetList editors = mFileMetaRepo.fileMeta(fileId)->editors();
    for (QWidget *edit: editors) {
        int index = ui->mainTab->indexOf(edit);
        if (index >= 0) {
            FileMeta *fm = mFileMetaRepo.fileMeta(fileId);
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

    int choice;

    // TODO(JM) Handle other file-types
    if (file->isAutoReload()) {
        choice = QMessageBox::Yes;
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("File modified");
        if (!file->isModified()) {
            // file is loaded but unchanged: ASK, if it should be reloaded
            msgBox.setText(file->location()+" has been modified externally.");
            msgBox.setInformativeText("Reload?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        }
        msgBox.setDefaultButton(QMessageBox::NoButton);
        choice = msgBox.exec();
    }

    if (choice == QMessageBox::Yes || choice == QMessageBox::Discard) {
        file->load(file->codecMib());
    } else {
        file->document()->setModified();
    }
}

void MainWindow::fileDeletedExtern(FileId fileId)
{
    FileMeta *file = mFileMetaRepo.fileMeta(fileId);
    // file has not been loaded: nothing to do
    if (!file->isOpen()) return;

    // file is loaded: ASK, if it should be closed
    QMessageBox msgBox;
    msgBox.setWindowTitle("File vanished");
    msgBox.setText(file->location()+" doesn't exist any more.");
    msgBox.setInformativeText("Keep file in editor?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::NoButton);
    int ret = msgBox.exec();

    if (ret == QMessageBox::No)
        closeFileEditors(fileId);
    else
        file->document()->setModified();
}

void MainWindow::fileEvent(const FileEvent &e)
{
    FileMeta *fm = mFileMetaRepo.fileMeta(e.fileId());
    if (!fm) return;
    if (e.kind() == FileEvent::Kind::changed)
        fileChanged(e.fileId()); // Just update display kind
    else if (e.kind() == FileEvent::Kind::created)
        fileChanged(e.fileId()); // Just update display kind
    else if (e.kind() == FileEvent::Kind::closed)
        fileClosed(e.fileId());
    else if (!fm->isOpen())
        fileChanged(e.fileId()); // Just update display kind
    else {
        // file handling with user-interaction are delayed
        QPair<FileId, FileEvent::Kind> pair = QPair<FileId, FileEvent::Kind>(e.fileId(), e.kind());
        if (!mFileEvents.contains(pair))
            mFileEvents << pair;
        mFileTimer.start(100);
    }
}

void MainWindow::processFileEvents()
{
    while (!mFileEvents.isEmpty()) {
        if (!isActiveWindow()) break;
        QPair<FileId, FileEvent::Kind> fileEvent = mFileEvents.takeFirst();
        FileMeta *fm = mFileMetaRepo.fileMeta(fileEvent.first);
        if (!fm) continue;
        // Todo

        switch (fileEvent.second) {
//        case FileEvent::Kind::renamedExtern:
//            fileRenamedExtern(fm->id());
//            break;
        case FileEvent::Kind::changedExtern:
            fileChangedExtern(fm->id());
            break;
        case FileEvent::Kind::removedExtern:
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

void MainWindow::postGamsRun(AbstractProcess* process)
{
    ProjectRunGroupNode* runGroup = mProjectRepo.findRunGroup(process);
    QFileInfo fileInfo(process->inputFile());
    if(runGroup && fileInfo.exists()) {

        QString lstFile = runGroup->lstFileName(); // TODO(JM) detect options-dependant LST-filename

//        appendErrData(fileInfo.path() + "/" + fileInfo.completeBaseName() + ".err");
        bool doFocus = runGroup->isActive();

        if (mSettings->jumpToError())
            runGroup->jumpToFirstError(doFocus);

        ProjectAbstractNode * node = mProjectRepo.findNode(lstFile, runGroup);
        ProjectFileNode* lstNode = node ? node->toFile() : nullptr;
        if (!lstNode) lstNode = mProjectRepo.findOrCreateFileNode(lstFile, runGroup);
        if (lstNode) mTextMarkRepo.removeMarks(lstNode->file()->id());

        if (mSettings->openLst() && lstNode)
            openFile(lstNode->file(), true, runGroup);
    }
}

void MainWindow::postGamsLibRun(AbstractProcess* process)
{
    // TODO(AF) Are there models without a GMS file? How to handle them?"
    Q_UNUSED(process);
    FileMeta *file = mFileMetaRepo.fileMeta(mLibProcess->targetDir() + "/" + mLibProcess->inputFile());
    ProjectFileNode *node = nullptr;
    if (!file)
        node = addNode(mLibProcess->targetDir(), mLibProcess->inputFile());
    else {
        QVector<ProjectFileNode*> nodes = mProjectRepo.fileNodes(file->id(), file->id());
        if (!nodes.isEmpty()) node = nodes.first();
    }
    if (!node)
        EXCEPT() << "Error creating file '" << mLibProcess->inputFile() << "'";

    if (!file->editors().isEmpty()) {
        file->load(file->codecMib());
    }
    openFile(file, true, node->runParentNode(), file->codecMib());
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
    QWidget* edit = ui->mainTab->widget(index);
    FileMeta* file = mFileMetaRepo.fileMeta(edit);
    if (!file) {
        ui->mainTab->removeTab(index);
        // assuming we are closing a welcome page here
        mWp = nullptr;
        return;
    }

    int ret = QMessageBox::Discard;
    if (file->editors().size() == 1 && file->isModified()) {
        // only ask, if this is the last editor of this file
        QMessageBox msgBox;
        msgBox.setText(ui->mainTab->tabText(index)+" has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        ret = msgBox.exec();
    }
    if (ret == QMessageBox::Save)
        file->save();

    if (ret != QMessageBox::Cancel) {
        for (const auto& file : mAutosaveHandler->checkForAutosaveFiles(mOpenTabsList))
            QFile::remove(file);
        mClosedTabs << file->location();
        ui->mainTab->removeTab(ui->mainTab->indexOf(edit));
        file->removeEditor(edit);
        edit->deleteLater();
    }
}

void MainWindow::on_logTabs_tabCloseRequested(int index)
{
    QWidget* edit = ui->logTabs->widget(index);
    FileMeta* logMeta = mFileMetaRepo.fileMeta(ui->logTabs->widget(index));
    if (logMeta) {
        // TODO(JM) check validity
        ui->logTabs->removeTab(index);
        logMeta->removeEditor(edit);
        // dont remove pointers in ui, otherwise re-adding syslog will crash
        if (edit != mSyslog)
            edit->deleteLater();
    }
}

void MainWindow::createWelcomePage()
{
    mWp = new WelcomePage(history(), this);
    ui->mainTab->insertTab(0, mWp, QString("Welcome")); // always first position
    connect(mWp, &WelcomePage::linkActivated, this, &MainWindow::openFilePath);
    ui->mainTab->setCurrentIndex(0); // go to welcome page
}

bool MainWindow::isActiveTabRunnable()
{
    QWidget *editWidget = (ui->mainTab->currentIndex() < 0 ? nullptr : ui->mainTab->widget((ui->mainTab->currentIndex())) );

    AbstractEdit* edit = FileMeta::toAbstractEdit(editWidget);
    if (edit) {
        ProjectFileNode* node = mProjectRepo.findFileNode(edit);
        return (node && !edit->isReadOnly());
    }
    return false;
}

bool MainWindow::isActiveTabSetAsMain()
{
    QWidget *editWidget = (ui->mainTab->currentIndex() < 0 ? nullptr : ui->mainTab->widget((ui->mainTab->currentIndex())) );
    AbstractEdit* edit = FileMeta::toAbstractEdit(editWidget);
    if (edit) {
        ProjectFileNode* node = mProjectRepo.findFileNode(edit);
        if (node) {
           ProjectRunGroupNode* runGroup = node->runParentNode();
           if (runGroup)
               return (node->file() == runGroup->runnableGms());
        }
    }
    return false;
}

bool MainWindow::isRecentGroupInRunningState()
{
    ProjectRunGroupNode *runGroup = mRecent.group ? mRecent.group->toRunGroup() : nullptr;
    QProcess::ProcessState state = runGroup ? runGroup->gamsProcessState() : QProcess::NotRunning;
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
    if(mWp == nullptr)
        createWelcomePage();
    else
        ui->mainTab->setCurrentIndex(ui->mainTab->indexOf(mWp));
}

void MainWindow::renameToBackup(QFile *file)
{
    const int MAX_BACKUPS = 3;
    FileMeta *fileMeta = mFileMetaRepo.fileMeta(file->fileName());
    if (fileMeta) mFileMetaRepo.unwatch(fileMeta->location());

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

void MainWindow::openFilePath(const QString &filePath)
{
    FileMeta* fm = mFileMetaRepo.findOrCreateFileMeta(filePath);

    // We need to find a ProjectRunGroupNode
    ProjectRunGroupNode * runGroup = nullptr;
    ProjectFileNode * node = nullptr;
    // search for node in active group
    if (mRecent.group) {
        runGroup = mRecent.group->runParentNode();
        node = runGroup->findFile(fm);
    }
    // search for node
    if (!node) {
        node = mProjectRepo.treeModel()->rootNode()->findFile(fm);
        if (node) runGroup = node->runParentNode();
    }
    // no node exists
    if (!node) {
        QFileInfo fi(fm->location());
        ProjectGroupNode *group = mProjectRepo.findGroup(fm->location());
        if (!group)
            group = mProjectRepo.createGroup(fm->name(), fi.absolutePath(), fm->location());
        group->findOrCreateFileNode(fm->location());
        runGroup = group->runParentNode();
    }
    openFile(fm, true, runGroup, -1);
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
        ProjectLogNode* logNode = node->toRunGroup()->getOrCreateLogNode(&mFileMetaRepo);
        if (!logNode->file()->isOpen()) {
            logNode->setDebugLog(mLogDebugLines);
            ProcessLogEdit* logEdit = new ProcessLogEdit(this);
            logEdit->setLineWrapMode(mSettings->lineWrapProcess() ? AbstractEdit::WidgetWidth : AbstractEdit::NoWrap);
            FileMeta::initEditorType(logEdit);
            int ind = ui->logTabs->addTab(logEdit, logNode->name(NameModifier::editState));
            logNode->file()->addEditor(logEdit);
            ui->logTabs->setCurrentIndex(ind);
        }
    } else if (node->toFile()) {
        openNode(index);
    }
}

bool MainWindow::requestCloseChanged(QVector<FileMeta*> changedFiles)
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
            for (FileMeta* file: changedFiles) {
                if (file->isModified()) {
                    file->save();
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
    QVector<FileMeta*> changedFiles = mFileMetaRepo.modifiedFiles();
    if (!requestCloseChanged(changedFiles)) {
        event->setAccepted(false);
    } else {
        mSettings->saveSettings(this);
    }
    on_actionClose_All_triggered();
    closeHelpView();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_0))
        updateFixedFonts(mSettings->fontFamily(), mSettings->fontSize());

    if (event->key() == Qt::Key_Escape) {
        mSearchWidget->hide();
        mSearchWidget->clearResults();
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

void MainWindow::parseFilesFromCommandLine(const QString &commandLineStr, ProjectRunGroupNode* runnGroup)
{
    QList<OptionItem> items = mGamsOptionWidget->getGamsOptionTokenizer()->tokenize( commandLineStr );

    // set default lst file name in case output option changed back to default
    if (runnGroup->runnableGms())
        runnGroup->setLstFileName(QFileInfo(runnGroup->runnableGms()->location()).baseName() + ".lst");

    foreach (OptionItem item, items) {
        // output (o) found, case-insensitive
        if (QString::compare(item.key, "o", Qt::CaseInsensitive) == 0
                || QString::compare(item.key, "output", Qt::CaseInsensitive) == 0) {

            runnGroup->setLstFileName(item.value);
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

OptionWidget *MainWindow::getGamsOptionWidget() const
{
    return mGamsOptionWidget;
}

void MainWindow::ensureLogEditor(ProjectLogNode* logProc)
{
    if (!logProc->file()->isOpen()) return;
    logProc->setDebugLog(mLogDebugLines);

    ProcessLogEdit* logEdit = new ProcessLogEdit(this);
    logEdit->setLineWrapMode(mSettings->lineWrapProcess() ? AbstractEdit::WidgetWidth : AbstractEdit::NoWrap);
    FileMeta::initEditorType(logEdit);

    ui->logTabs->addTab(logEdit, logProc->name(NameModifier::editState));
    logProc->file()->addEditor(logEdit);
    updateFixedFonts(mSettings->fontFamily(), mSettings->fontSize());
}

void MainWindow::execute(QString commandLineStr, ProjectFileNode* gmsFileNode)
{
    ProjectFileNode* fc = (gmsFileNode ? gmsFileNode : mProjectRepo.findFileNode(mRecent.editor()));
    ProjectRunGroupNode *runGroup = (fc ? fc->runParentNode() : nullptr);
    if (!runGroup) return;

    parseFilesFromCommandLine(commandLineStr, runGroup);

    runGroup->clearLstErrorTexts();

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
            msgBox.setText(openFiles.size()+" files have been modified.");
        msgBox.setInformativeText("Do you want to save your changes before running?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        QAbstractButton* discardButton = msgBox.addButton(tr("Discard Changes and Run"), QMessageBox::ResetRole);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Cancel) {
            return;
        } else if (msgBox.clickedButton() == discardButton) {
            for (FileMeta *file: openFiles)
                file->load(file->codecMib());
            doSave = false;
        }
    }

    if (doSave)
        for (FileMeta *file: openFiles) {
            file->save();
        }

    // TODO(JM) for each fileNode in runGroup
    QSet<TextMark::Type> markTypes;
    markTypes << TextMark::error << TextMark::link;
    for (ProjectFileNode *node: runGroup->listFiles(true)) {
        mTextMarkRepo.removeMarks(node->file()->id(), markTypes);
    }
    //mProjectRepo.removeMarks(runGroup);

    ProjectLogNode* logProc = mProjectRepo.asLogNode(runGroup);

    ensureLogEditor(logProc);

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
    QString gmsFilePath = (gmsFileNode ? gmsFileNode->location() : runGroup->runnableGms()->location());

    if (gmsFilePath == "")
        mSyslog->appendLog("No runnable GMS file found.", LogMsgType::Warning);

    QFileInfo gmsFileInfo(gmsFilePath);

    logProc->setJumpToLogEnd(true);
    GamsProcess* process = runGroup->gamsProcess();
    QString lstFileName = runGroup->lstFileName();
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

    ui->dockLogView->raise();
}

void MainWindow::updateRunState()
{
    mGamsOptionWidget->updateRunState(isActiveTabRunnable(), isActiveTabSetAsMain(), isRecentGroupInRunningState());
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
    ProjectRunGroupNode *runGroup = fc->runParentNode();
    if (runGroup)
        runGroup->setRunnableGms(fc->file());
    // loadCommandLineOptions(fc);
    // TODO As an activated tab should synchronize with the shown option,
    // also activate Tab in addition to loadCommandLineOptions(fc).
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
    if (isActiveTabSetAsMain() && !isRecentGroupInRunningState())
       on_actionRun_triggered();
}

void MainWindow::on_actionRun_triggered()
{
    if (isActiveTabRunnable()) {
        execute( mGamsOptionWidget->on_runAction(RunActionState::Run) );
    }
}

void MainWindow::on_actionRun_with_GDX_Creation_triggered()
{
    if (isActiveTabRunnable()) {
        execute( mGamsOptionWidget->on_runAction(RunActionState::RunWithGDXCreation) );
    }
}

void MainWindow::on_actionCompile_triggered()
{
    if (isActiveTabRunnable()) {
        execute( mGamsOptionWidget->on_runAction(RunActionState::Compile) );
    }
}

void MainWindow::on_actionCompile_with_GDX_Creation_triggered()
{
    if (isActiveTabRunnable()) {
        execute( mGamsOptionWidget->on_runAction(RunActionState::CompileWithGDXCreation) );
    }
}

void MainWindow::on_actionInterrupt_triggered()
{
    ProjectFileNode* fc = mProjectRepo.findFileNode(mRecent.editor());
    ProjectRunGroupNode *runGroup = (fc ? fc->runParentNode() : nullptr);
    if (!runGroup) return;

    mGamsOptionWidget->on_interruptAction();
    GamsProcess* process = runGroup->gamsProcess();
    QtConcurrent::run(process, &GamsProcess::interrupt);
}

void MainWindow::on_actionStop_triggered()
{
    ProjectFileNode* fc = mProjectRepo.findFileNode(mRecent.editor());
    ProjectRunGroupNode *runGroup = (fc ? fc->runParentNode() : nullptr);
    if (!runGroup) return;

    mGamsOptionWidget->on_stopAction();
    GamsProcess* process = runGroup->gamsProcess();
    QtConcurrent::run(process, &GamsProcess::stop);
}

void MainWindow::changeToLog(ProjectAbstractNode *node, bool createMissing)
{
    if (!node) return;
    ProjectRunGroupNode *runGroup = node->runParentNode();
    if (!runGroup) return;
    ProjectLogNode* logNode = runGroup->logNode();
    if (!logNode) return;

    if (createMissing) ensureLogEditor(logNode);
    if (!logNode->file()->isOpen()) return;

    logNode->setDebugLog(mLogDebugLines);
    AbstractEdit* logEdit = FileMeta::toAbstractEdit(logNode->file()->editors().first());
    if (logEdit) {
        if (ui->logTabs->currentWidget() != logEdit) {
            if (ui->logTabs->currentWidget() != mResultsView)
                ui->logTabs->setCurrentWidget(logEdit);
        }
        if (createMissing) { // move to end
            QTextCursor cursor = logEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            logEdit->setTextCursor(cursor);
        }
    }
}

void MainWindow::storeTree()
{
    // TODO(JM) add settings methods to store each part separately
    mSettings->saveSettings(this);
}

TextMarkRepo *MainWindow::textMarkRepo()
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
    // open edit if existing or create one
    if (edit) {
        if (focus) tabWidget->setCurrentWidget(edit);
    } else {
        edit = fileMeta->createEdit(tabWidget, runGroup, (codecMib == -1) ? encodingMIBs() : QList<int>() << codecMib);
        CodeEdit* codeEdit = FileMeta::toCodeEdit(edit);
        if (codeEdit) {
            connect(codeEdit, &CodeEdit::searchFindNextPressed, mSearchWidget, &SearchWidget::on_searchNext);
            connect(codeEdit, &CodeEdit::searchFindPrevPressed, mSearchWidget, &SearchWidget::on_searchPrev);
            if (!codeEdit->isReadOnly()) {
                connect(codeEdit, &CodeEdit::requestAdvancedActions, this, &MainWindow::getAdvancedActions);
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
    // set keyboard focus to editor
    if (tabWidget->currentWidget())
        if (focus) {
            lxiviewer::LxiViewer* lxiViewer = FileMeta::toLxiViewer(edit);
            if (lxiViewer)
                lxiViewer->codeEdit()->setFocus();
            else
                tabWidget->currentWidget()->setFocus();
        }
    if (tabWidget != ui->logTabs) {
        // if there is already a log -> show it
        changeToLog(mProjectRepo.findFileNode(edit));
    }
    addToOpenedFiles(fileMeta->location());
}

void MainWindow::closeGroup(ProjectGroupNode* group)
{

    // TODO(JM) what to do here? Don't need to close a file that is linked from a node of another group
    // 1. gather nodes
    // 2. gather open files
    // 3. request
    // 4. close node via ProjectRepo

    if (!group) return;
    QVector<FileMeta*> changedFiles;
    QVector<FileMeta*> openFiles;
    QVector<FileMeta*> unboundFiles;


    for (ProjectFileNode *node: group->listFiles(true)) {
        QVector<ProjectRunGroupNode*> otherRunGroups = mProjectRepo.runGroups(node->file()->id());
        otherRunGroups.removeOne(node->runParentNode());

        if (otherRunGroups.size() == 0) { // all nodes are in the same runGroup -> fileMeta can be removed later
            unboundFiles << node->file();
            if (node->file()->isOpen()) {
                openFiles << node->file();
                if (node->file()->isModified())
                    changedFiles << node->file();
            }
        }
    }

    if (requestCloseChanged(changedFiles)) {
        for (FileMeta *file: openFiles) {
            closeFileEditors(file->id());
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

        mProjectRepo.closeGroup(group);
        while (!unboundFiles.isEmpty()) {
            FileMeta* file = unboundFiles.takeFirst();
            mFileMetaRepo.removedFile(file);
            delete file;
        }
        mSettings->saveSettings(this);
    }
}

/// Asks user for confirmation if a file is modified before calling closeFile
/// \param file
///
void MainWindow::closeNodeConditionally(ProjectFileNode* node) {
    QVector<ProjectRunGroupNode*> otherRunGroups = mProjectRepo.runGroups(node->file()->id());
    otherRunGroups.removeOne(node->runParentNode());
    if (!node->isModified() || !otherRunGroups.isEmpty() || requestCloseChanged(QVector<FileMeta*>() << node->file())) {
        mProjectRepo.closeNode(node);
    }
}

/// Closes all open editors and tabs related to a file and remove option history
/// \param fileId
///
void MainWindow::closeFileEditors(FileId fileId)
{
    FileMeta* file = mFileMetaRepo.fileMeta(fileId);
    if (!file)
        FATAL() << "FileId " << fileId << " is not of class FileContext.";

    // add to recently closed tabs
    mClosedTabs << file->location();

    // close all related editors, tabs and clean up
    while (!file->editors().isEmpty()) {
        QWidget *edit = file->editors().first();
        ui->mainTab->removeTab(ui->mainTab->indexOf(edit));
        file->removeEditor(edit);
        edit->deleteLater();
    }
    // purge history
    getGamsOptionWidget()->removeFromHistory(file->location());
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
    ProjectFileNode *file = mProjectRepo.asFileNode(index);
    if (file) openFilePath(file->location());
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
        gdxviewer::GdxViewer *gdx = FileMeta::toGdxViewer(mRecent.editor());
        if (gdx) {
            gdx->selectSearchField();
            return;
        }
       // toggle visibility
       if (mSearchWidget->isVisible()) {
           mSearchWidget->activateWindow();
           mSearchWidget->autofillSearchField();
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

    QWidgetList editList = openEditors();
    for (AbstractEdit* ed: openLogs()) editList << ed;
    for (int i = 0; i < editList.size(); i++) {
        AbstractEdit* ed = FileMeta::toAbstractEdit(editList.at(i));
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
        QWidget *widget = ui->mainTab->widget(i);
        if (!widget || widget == mWp) continue;
        ProjectFileNode *fc = mProjectRepo.findFileNode(widget);
        if (!fc) continue;
        QJsonObject tabObject;
        tabObject["location"] = fc->location();
        tabObject["codecMib"] = fc->file()->codecMib();
        // TODO(JM) store current edit position
        tabArray.append(tabObject);
    }
    json["mainTabs"] = tabArray;
    ProjectFileNode *fc = mRecent.editor() ? mProjectRepo.findFileNode(mRecent.editor()) : nullptr;
    if (fc) json["mainTabRecent"] = fc->location();
}

void MainWindow::on_actionGo_To_triggered()
{
    if ((ui->mainTab->currentWidget() == mWp) || (mRecent.editor() == nullptr))
        return;
    GoToDialog dialog(this);
    dialog.exec();
}


void MainWindow::on_actionRedo_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce) ce->redo();
}

void MainWindow::on_actionUndo_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce) ce->undo();
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

    ProjectFileNode *fc = mProjectRepo.findFileNode(mRecent.editor());
    if (!fc) return;

    if (fc->file()->kind() == FileKind::Gdx) {
        gdxviewer::GdxViewer *gdx = FileMeta::toGdxViewer(mRecent.editor());
        gdx->copyAction();
    } else if (focusWidget() == mSyslog) {
        mSyslog->copy();
    } else {
        AbstractEdit *ae = FileMeta::toAbstractEdit(focusWidget());
        if (!ae) return;
        CodeEdit *ce = FileMeta::toCodeEdit(ae);

        if (ce && ce->blockEdit()) {
            ce->blockEdit()->selectionToClipboard();
        } else {
            ae->copy();
        }
    }
}

void MainWindow::on_actionSelect_All_triggered()
{
    ProjectFileNode *fc = mProjectRepo.findFileNode(mRecent.editor());
    if (!fc || !focusWidget()) return;

    if (fc->file()->kind() == FileKind::Gdx) {
        gdxviewer::GdxViewer *gdx = FileMeta::toGdxViewer(mRecent.editor());
        gdx->selectAllAction();
    } else if (focusWidget() == mSyslog) {
        mSyslog->selectAll();
    } else {
        CodeEdit* ce = FileMeta::toCodeEdit(focusWidget());
        if (ce) ce->selectAll();
    }
}

void MainWindow::on_actionCut_triggered()
{
    CodeEdit* ce= FileMeta::toCodeEdit(focusWidget());
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
        AbstractEdit *ae = FileMeta::toAbstractEdit(QApplication::focusWidget());
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
    if (ce && !ce->isReadOnly()) {
        QTextCursor c = ce->textCursor();
        c.insertText(c.selectedText().toUpper());
    }
}

void MainWindow::on_actionSet_to_Lowercase_triggered()
{
    if ( !mRecent.editor() || (focusWidget() != mRecent.editor()) )
        return;
    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
    if (ce && !ce->isReadOnly()) {
        QTextCursor c = ce->textCursor();
        c.insertText(c.selectedText().toLower());
    }
}

void MainWindow::on_actionOverwrite_Mode_toggled(bool overwriteMode)
{
    CodeEdit* ce = FileMeta::toCodeEdit(mRecent.editor());
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

void MainWindow::toggleLogDebug()
{
    mLogDebugLines = !mLogDebugLines;
    ProjectGroupNode* root = mProjectRepo.treeModel()->rootNode();
    for (int i = 0; i < root->childCount(); ++i) {
        ProjectRunGroupNode *runGroup = root->childNode(i)->toRunGroup();
        if (runGroup) {
            ProjectLogNode* log = runGroup->logNode();
            if (log) log->setDebugLog(mLogDebugLines);
        }
    }
}

void MainWindow::on_actionRestore_Recently_Closed_Tab_triggered()
{
    // TODO: remove duplicates?
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
    AbstractEdit* edit = FileMeta::toAbstractEdit(mEditor);
    if (edit) {
        MainWindow::disconnect(edit, &AbstractEdit::cursorPositionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::disconnect(edit, &AbstractEdit::selectionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::disconnect(edit, &AbstractEdit::blockCountChanged, window, &MainWindow::updateEditorBlockCount);
        MainWindow::disconnect(edit->document(), &QTextDocument::contentsChange, window, &MainWindow::on_currentDocumentChanged);
    }
    mEditor = editor;
    edit = FileMeta::toAbstractEdit(mEditor);
    if (edit) {
        MainWindow::connect(edit, &AbstractEdit::cursorPositionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::connect(edit, &AbstractEdit::selectionChanged, window, &MainWindow::updateEditorPos);
        MainWindow::connect(edit, &AbstractEdit::blockCountChanged, window, &MainWindow::updateEditorBlockCount);
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
        } else if (dock == ui->dockHelpView) {
            dock->setVisible(false);
            addDockWidget(Qt::RightDockWidgetArea, dock);
            resizeDocks(QList<QDockWidget*>() << dock, {width()/3}, Qt::Horizontal);
            stackedSecond = dock;
        } else if (dock == ui->dockOptionEditor) {
            addDockWidget(Qt::TopDockWidgetArea, dock);
        }
    }
    // stack help over output
    tabifyDockWidget(stackedFirst, stackedSecond);
}

void MainWindow::resizeOptionEditor(const QSize &size)
{
    mGamsOptionWidget->resize( size );
    this->resizeDocks({ui->dockOptionEditor}, {size.height()}, Qt::Vertical);
}

}
}
