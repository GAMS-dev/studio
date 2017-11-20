/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "codeeditor.h"
#include "welcomepage.h"
#include "modeldialog/modeldialog.h"
#include "exception.h"
#include "treeitemdelegate.h"
#include "gamspaths.h"
#include "newdialog.h"
#include "gamsprocess.h"
#include "gamslibprocess.h"
#include "gdxviewer/gdxviewer.h"
#include "logger.h"

namespace gams {
namespace studio {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    history = new HistoryData();
    loadSettings();
    ui->setupUi(this);
    setAcceptDrops(true);

    ui->projectView->setModel(mFileRepo.treeModel());
    ui->projectView->setRootIndex(mFileRepo.treeModel()->rootModelIndex());
    mFileRepo.setSuffixFilter(QStringList() << ".gms" << ".inc" << ".log" << ".lst" << ".txt" << ".gdx");
    mFileRepo.setDefaultActions(QList<QAction*>() << ui->actionNew << ui->actionOpen);
    ui->projectView->setHeaderHidden(true);
    ui->projectView->setItemDelegate(new TreeItemDelegate(ui->projectView));
    ui->projectView->setIconSize(QSize(16,16));
    ui->mainToolBar->setIconSize(QSize(21,21));
    ui->logView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    ui->logView->setTextInteractionFlags(ui->logView->textInteractionFlags() | Qt::TextSelectableByKeyboard);

    // TODO(JM) it is possible to put the QTabBar into the docks title:
    //          if we override the QTabWidget it should be possible to extend it over the old tab-bar-space
//    ui->dockLogView->setTitleBarWidget(ui->tabLog->tabBar());

    initTabs();

    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(ui->actionRun);
    mCommandLineOption = new CommandLineOption(this);
    ui->mainToolBar->addWidget(mCommandLineOption);
    connect(mCommandLineOption, &CommandLineOption::runWithChangedOption, this, &MainWindow::on_runWithCommandLineOption);

    mCodecGroup = new QActionGroup(this);
    connect(mCodecGroup, &QActionGroup::triggered, this, &MainWindow::codecChanged);
    connect(ui->mainTab, &QTabWidget::currentChanged, this, &MainWindow::activeTabChanged);
    connect(&mFileRepo, &FileRepository::fileClosed, this, &MainWindow::fileClosed);
    connect(&mFileRepo, &FileRepository::fileChangedExtern, this, &MainWindow::fileChangedExtern);
    connect(&mFileRepo, &FileRepository::fileDeletedExtern, this, &MainWindow::fileDeletedExtern);
    connect(&mFileRepo, &FileRepository::openOrShowContext, this, &MainWindow::openOrShowContext);
    connect(ui->dockLogView, &QDockWidget::visibilityChanged, this, &MainWindow::setOutputViewVisibility);
    connect(ui->dockProjectView, &QDockWidget::visibilityChanged, this, &MainWindow::setProjectViewVisibility);
    connect(ui->projectView, &QTreeView::clicked, &mFileRepo, &FileRepository::nodeClicked);
    ensureCodecMenu("System");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initTabs()
{
    QPalette pal = ui->projectView->palette();
    pal.setColor(QPalette::Highlight, Qt::transparent);
    ui->projectView->setPalette(pal);

    createWelcomePage();
}

void MainWindow::createEdit(QTabWidget* tabWidget, QString codecName)
{
    createEdit(tabWidget, -1, codecName);
}

void MainWindow::createEdit(QTabWidget *tabWidget, int id, QString codecName)
{
    FileContext *fc = mFileRepo.fileContext(id);
    if (fc) {
        int tabIndex;
        if (fc->metrics().fileType() != FileType::Gdx) {

            CodeEditor *codeEdit = new CodeEditor(this);
            fc->addEditor(codeEdit);
            tabIndex = tabWidget->addTab(codeEdit, fc->caption());

            QTextCursor tc = codeEdit->textCursor();
            tc.movePosition(QTextCursor::Start);
            codeEdit->setTextCursor(tc);
            fc->load(codecName);

            if (fc->metrics().fileType() == FileType::Log ||
                    fc->metrics().fileType() == FileType::Lst) {  // TODO: add .ref ?

                codeEdit->setReadOnly(true);
                codeEdit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
            } else {
                connect(fc, &FileContext::changed, this, &MainWindow::fileChanged);
            }

        } else {
            tabIndex = ui->mainTab->addTab(new gdxviewer::GdxViewer(fc->location(), GAMSPaths::systemDir()), fc->caption());
            ui->mainTab->setCurrentIndex(tabIndex);
        }

        tabWidget->setTabToolTip(tabIndex, fc->location());
        tabWidget->setCurrentIndex(tabIndex);
        ensureCodecMenu(fc->codec());
    }
}

void MainWindow::ensureCodecMenu(QString codecName)
{
    bool actionFound = false;
    for (QAction *act: ui->menuEncoding->actions()) {
        if (act->text().compare(codecName, Qt::CaseInsensitive) == 0)
            actionFound = true;
    }
    if (!actionFound) {
        QAction *action = new QAction(codecName, ui->menuEncoding);
        action->setCheckable(true);
        action->setChecked(true);
        action->setActionGroup(mCodecGroup);
//        mCodecGroup->addAction(codecName);
        ui->menuEncoding->addActions(mCodecGroup->actions());
    }
}

void MainWindow::setOutputViewVisibility(bool visibility)
{
    ui->actionOutput_View->setChecked(visibility);
}

void MainWindow::setProjectViewVisibility(bool visibility)
{
    ui->actionProject_View->setChecked(visibility);
}

void MainWindow::gamsProcessStateChanged(FileGroupContext* group, QProcess::ProcessState newState)
{
    if (mRecent.group == group) updateRunState();
}

void MainWindow::on_actionNew_triggered()
{
    QString path = mRecent.path;
    if (mRecent.editFileId >= 0) {
        FileContext *fc = mFileRepo.fileContext(mRecent.editFileId);
        if (fc) path = QFileInfo(fc->location()).path();
    }
    auto filePath = QFileDialog::getSaveFileName(this,
                                                 "Create new file...",
                                                 path,
                                                 tr("GAMS code (*.gms *.inc );;"
                                                 "Text files (*.txt);;"
                                                 "All files (*)"));

    QFile file(filePath);
    if (!file.exists()) { // which should be the default!
        file.open(QIODevice::WriteOnly);
        file.close();
    } // TODO: else, ask for overwrite

    if (FileContext *fc = addContext("", filePath, true)) {
        fc->save();
    }
    addToOpenedFiles(filePath);
}

void MainWindow::on_actionOpen_triggered()
{
    QString fName = QFileDialog::getOpenFileName(this,
                                                 "Open file",
                                                 mRecent.path,
                                                 tr("GAMS code (*.gms *.inc *.gdx);;"
                                                    "Text files (*.txt);;"
                                                    "All files (*)"));
    addContext("", fName, true);
    addToOpenedFiles(fName);
}

void MainWindow::on_actionSave_triggered()
{
    FileContext* fc = mFileRepo.fileContext(mRecent.editFileId);
    if (!fc) return;
    if (fc->location().isEmpty()) {
        on_actionSave_As_triggered();
    } else if (fc->isModified()) {
        fc->save();
    }
}

void MainWindow::on_actionSave_As_triggered()
{
    QString path = mRecent.path;
    FileContext *formerFc;
    if (mRecent.editFileId >= 0) {
        formerFc = mFileRepo.fileContext(mRecent.editFileId);
        if (formerFc) path = QFileInfo(formerFc->location()).path();
    }
    auto filePath = QFileDialog::getSaveFileName(this,
                                                 "Save file as...",
                                                 path,
                                                 tr("GAMS code (*.gms *.inc );;"
                                                 "Text files (*.txt);;"
                                                 "All files (*)"));
    if (!filePath.isEmpty()) {
        mRecent.path = QFileInfo(filePath).path();
        FileContext* fc = mFileRepo.fileContext(mRecent.editFileId);
        if (!fc) return;

        if(fc->location().endsWith(".gms") && !filePath.endsWith(".gms")) {
            filePath = filePath + ".gms";
        } else if (fc->location().endsWith(".lst") && !filePath.endsWith(".lst")) {
            filePath = filePath + ".lst";
        } // TODO: check if there are others to add

        // given what happens on the drive when saving a file as... the old node should stay in project explorer

        fc->save(filePath);
//        mFileRepo.removeNode(formerFc);
        openOrShow(filePath, fc->parentEntry());

    }
}

void MainWindow::on_actionSave_All_triggered()
{
    int i = mFileRepo.saveAll();
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
    qDebug() << "Codec action triggered: " << action->text();
}

void MainWindow::activeTabChanged(int index)
{
    QWidget *editWidget = (index < 0 ? nullptr : ui->mainTab->widget(index));
    QPlainTextEdit* edit = static_cast<QPlainTextEdit*>(editWidget);
    if (edit) {
        FileContext* fc = mFileRepo.fileContext(edit);
        int fileId = (fc ? fc->id() : -1);
        if (fileId >= 0) {
            mRecent.editFileId = fileId;
            mRecent.editor = edit;
            mRecent.group = fc->parentEntry();
        }
    }
}

void MainWindow::fileChanged(int fileId)
{
    QList<QPlainTextEdit*> editors = mFileRepo.editors(fileId);
    for (QWidget *edit: editors) {
        int index = ui->mainTab->indexOf(edit);
        if (index >= 0) {
            FileContext *fc = mFileRepo.fileContext(fileId);
            if (fc) ui->mainTab->setTabText(index, fc->caption());
        }
    }
}

void MainWindow::fileChangedExtern(int fileId)
{
    FileContext *fc = mFileRepo.fileContext(fileId);

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
        if (fc->document() && !fc->isModified()) {
            msgBox.setText(fc->location()+" has been modified externally.");
            msgBox.setInformativeText("Reload?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        }

        // file has been changed in the editor: ASK, if intern or extern version should be kept.
        if (fc->isModified()) {
            msgBox.setText(fc->location() + " has been modified concurrently.");
            msgBox.setInformativeText("Do you want to"
                                      "\n- <b>Discard</b> your changes and reload the file"
                                      "\n- <b>Ignore</b> the external changes and keep your changes");
            msgBox.setStandardButtons(QMessageBox::Discard | QMessageBox::Ignore);
        }

        msgBox.setDefaultButton(QMessageBox::NoButton);
        choice = msgBox.exec();
    }

    if (choice == QMessageBox::Yes || choice == QMessageBox::Discard) {
        fc->load(fc->codec());
    }
}

void MainWindow::fileDeletedExtern(int fileId)
{
    FileContext *fc = mFileRepo.fileContext(fileId);
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
    if (ret == QMessageBox::No) {
        fileClosed(fileId);
    }
}

void MainWindow::fileClosed(int fileId)
{
    FileContext* fc = mFileRepo.fileContext(fileId);
    if (!fc)
        FATAL() << "FileId " << fileId << " is not of class FileContext.";
    while (!fc->editors().isEmpty()) {
        QPlainTextEdit *edit = fc->editors().first();
        ui->mainTab->removeTab(ui->mainTab->indexOf(edit));
        fc->removeEditor(edit);
        edit->deleteLater();
    }
}

void MainWindow::appendOutput(QProcess::ProcessChannel channel, QString text)
{
    Q_UNUSED(channel);
    QPlainTextEdit *outWin = ui->logView;
    if (!text.isNull()) {
        outWin->moveCursor(QTextCursor::End);
        outWin->insertPlainText(text);
        outWin->moveCursor(QTextCursor::End);
        outWin->document()->setModified(false);
    }
}

void MainWindow::postGamsRun(AbstractProcess* process)
{
    FileGroupContext* groupContext = process ? process->context() : nullptr;
    QFileInfo fileInfo(process->inputFile());
    if(groupContext && fileInfo.exists()) {// TODO: add .log and others)
        QString lstFile = fileInfo.path() + "/" + fileInfo.completeBaseName() + ".lst";
//        appendErrData(fileInfo.path() + "/" + fileInfo.completeBaseName() + ".err");
        openOrShow(lstFile, groupContext);
    } else {
        qDebug() << fileInfo.absoluteFilePath() << " not found. aborting.";
    }
    if (process) {
        process->deleteLater();
    }
    ui->actionRun->setEnabled(true);
}

void MainWindow::postGamsLibRun(AbstractProcess* process)
{
    // TODO(AF) Are there models without a GMS file? How to handle them?"
    Q_UNUSED(process);
    openOrShowContext(addContext(mLibProcess->targetDir(), mLibProcess->inputFile()));
    if (mLibProcess) {
        mLibProcess->deleteLater();
        mLibProcess = nullptr;
    }
}

void MainWindow::on_actionExit_Application_triggered()
{
    QCoreApplication::quit();
}

void MainWindow::on_actionOnline_Help_triggered()
{
    QDesktopServices::openUrl(QUrl("https://www.gams.com/latest/docs", QUrl::TolerantMode));
}

void MainWindow::on_actionAbout_triggered()
{
    QString about = "GAMS Studio Build Date: " __DATE__ " " __TIME__ "\n\n";
    about += GamsProcess::aboutGAMS();
    QMessageBox::about(this, "About GAMS Studio", about);
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::on_actionOutput_View_triggered(bool checked)
{
    if(checked)
        ui->dockLogView->show();
    else
        ui->dockLogView->hide();
}

void MainWindow::on_actionProject_View_triggered(bool checked)
{
    if(checked)
        ui->dockProjectView->show();
    else
        ui->dockProjectView->hide();
}

void MainWindow::on_mainTab_tabCloseRequested(int index)
{
    //TODO(CW): a closed tabWidget does not delte the underlying widget object. therefore we need to delete in manually. This needs review
    gdxviewer::GdxViewer* gdxViewer = dynamic_cast<gdxviewer::GdxViewer*>(ui->mainTab->widget(index));
    if(gdxViewer)
    {
        delete gdxViewer;
        return;
    }

    QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(ui->mainTab->widget(index));
    FileContext* fc = mFileRepo.fileContext(edit);
    if (!fc) {
        ui->mainTab->removeTab(index);
        // assuming we are closing a welcome page here
        wp = nullptr;
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
        if (fc->editors().size() == 1) {
            mFileRepo.close(fc->id());
        } else {
            fc->removeEditor(edit);
            ui->mainTab->removeTab(ui->mainTab->indexOf(edit));
        }
    }
}

void MainWindow::createWelcomePage()
{
    wp = new WelcomePage(history);
    ui->mainTab->insertTab(0, wp, QString("Welcome")); // always first position
    connect(wp, &WelcomePage::linkActivated, this, &MainWindow::openFile);
}

void MainWindow::on_actionShow_Welcome_Page_triggered()
{
    if(wp == nullptr)
        createWelcomePage();
    ui->mainTab->setCurrentIndex(0);
}

void MainWindow::renameToBackup(QFile *file)
{
    int suffix = 1;
    QString filename = file->fileName();
    if(!file->rename(filename + ".bak")) {
        while (!file->rename(filename + "." + QString::number(suffix) + ".bak")) {
            suffix++;
        }
    }
}

void MainWindow::triggerGamsLibFileCreation(LibraryItem *item, QString gmsFileName)
{
    mLibProcess = new GAMSLibProcess(this);
    mLibProcess->setApp(item->library()->execName());
    mLibProcess->setModelName(item->name());
    mLibProcess->setInputFile(gmsFileName);
    mLibProcess->setTargetDir(GAMSPaths::defaultWorkingDir());
    mLibProcess->execute();
    // This log is passed to the system-wide log
    connect(mLibProcess, &GamsProcess::newStdChannelData, this, &MainWindow::appendOutput);
    connect(mLibProcess, &GamsProcess::finished, this, &MainWindow::postGamsLibRun);
}

void MainWindow::saveSettings()
{
    if (mAppSettings == nullptr) {
        qDebug() << "ERROR: settings file missing.";
        return;
    }

    mAppSettings->beginGroup("uiState");
    mAppSettings->setValue("size", size());
    mAppSettings->setValue("pos", pos());

    mAppSettings->beginWriteArray("lastOpenedFiles");
    for (int i = 0; i < history->lastOpenedFiles.size(); i++) {
        mAppSettings->setArrayIndex(i);
        mAppSettings->setValue("file", history->lastOpenedFiles.at(i));
    }
    mAppSettings->endArray();
    mAppSettings->endGroup();
    mAppSettings->sync();
}

void MainWindow::loadSettings()
{
    if (mAppSettings == nullptr)
        mAppSettings = new QSettings("GAMS", "Studio");

    qDebug() << "loading settings from" << mAppSettings->fileName();

    mAppSettings->beginGroup("uiState");
    resize(mAppSettings->value("size", QSize(1024, 768)).toSize());
    move(mAppSettings->value("pos", QPoint(100, 100)).toPoint());

    mAppSettings->beginReadArray("lastOpenedFiles");
    for (int i = 0; i < history->MAX_FILE_HISTORY; i++) {
        mAppSettings->setArrayIndex(i);
        history->lastOpenedFiles.append(mAppSettings->value("file").toString());
    }
    mAppSettings->endArray();

    // TODO: before adding list of open tabs/files, add functionality to remove them from ui
    // TODO: add widget visibility, size, position, ...
    mAppSettings->endGroup();
}

QStringList MainWindow::getOpenedFiles()
{
    return history->lastOpenedFiles;
}

void MainWindow::openFile(const QString &filePath)
{
    openOrShow(filePath, nullptr);
}

void MainWindow::addToOpenedFiles(QString filePath)
{
    if (history->lastOpenedFiles.size() > history->MAX_FILE_HISTORY) {
        history->lastOpenedFiles.removeLast();
    }
    if (!history->lastOpenedFiles.contains(filePath))
        history->lastOpenedFiles.insert(0, filePath);
    else
        history->lastOpenedFiles.move(history->lastOpenedFiles.indexOf(filePath), 0);
}

void MainWindow::on_actionGAMS_Library_triggered()
{
    ModelDialog dialog(this);
    if(dialog.exec() == QDialog::Accepted)
    {
        QMessageBox msgBox;
        LibraryItem *item = dialog.selectedLibraryItem();
        QFileInfo fileInfo(item->files().first());
        QString gmsFileName = fileInfo.completeBaseName() + ".gms";
        QString gmsFilePath = GAMSPaths::defaultWorkingDir() + "/" + gmsFileName;
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
                addContext("", gmsFilePath);
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
        addToOpenedFiles(gmsFilePath);
    }
}

void MainWindow::on_projectView_doubleClicked(const QModelIndex &index)
{
    openContext(index);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QList<FileContext*> oFiles = mFileRepo.modifiedFiles();
    if (oFiles.size() > 0) {
        int ret = QMessageBox::Discard;
        QMessageBox msgBox;
        QString filesText = oFiles.size()==1 ? oFiles.first()->location() + " has been modified."
                                             : QString::number(oFiles.size())+" files have been modified";
        msgBox.setText(filesText);
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        ret = msgBox.exec();
        if (ret == QMessageBox::Save) {
            for (FileContext* fc: oFiles) {
                if (fc->isModified()) {
                    fc->save();
                }
            }
        }
        if (ret == QMessageBox::Cancel) {
            event->setAccepted(false);
        }
    }
    saveSettings();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (focusWidget() == ui->projectView && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
        openContext(ui->projectView->currentIndex());
    } else {
        QMainWindow::keyPressEvent(event);
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
            if (pathList.size() > 5) {
                break;
            }
            pathList << url.toLocalFile();
        }
        for (QString fName: pathList) {
            QFileInfo fi(fName);
            if (QFileInfo(fName).isFile()) {
                openOrShow(fi.canonicalFilePath(), nullptr, true);
            }
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons()) {
        QWidget* child = childAt(event->pos());
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::execute(QString commandLineStr)
{
    // TODO: add option to clear output view before running next job
        FileContext* fc = mFileRepo.fileContext(mRecent.editor);
        FileGroupContext *fgc = (fc ? fc->parentEntry() : nullptr);
        if (!fgc)
            return;

    ui->actionRun->setEnabled(false);
    mFileRepo.removeMarks(fgc);
    LogContext* logProc = mFileRepo.logContext(fgc);

    if (logProc->editors().isEmpty()) {
        QPlainTextEdit* logEdit = new QPlainTextEdit();
        ui->logTab->addTab(logEdit, logProc->caption());
        logProc->addEditor(logEdit);
    } else {
        logProc->clearRecentMarks();
    }
    logProc->markOld();
    ui->logTab->setCurrentWidget(logProc->editors().first());

    QString gmsFilePath = fgc->runableGms();
    QFileInfo gmsFileInfo(gmsFilePath);
    //    QString basePath = gmsFileInfo.absolutePath();

    GamsProcess* process = fgc->newGamsProcess();
    process->setWorkingDir(gmsFileInfo.path());
    process->setInputFile(gmsFilePath);
    process->setCommandLineStr(commandLineStr);
    process->execute();

    connect(process, &GamsProcess::newStdChannelData, logProc, &LogContext::addProcessData);
    connect(process, &GamsProcess::finished, this, &MainWindow::postGamsRun);
}

void MainWindow::updateRunState()
{
    QProcess::ProcessState state = mRecent.group ? mRecent.group->gamsProcessState() : QProcess::NotRunning;
    ui->actionRun->setEnabled(state != QProcess::Running);
}

void MainWindow::on_runWithCommandLineOption(QString options)
{
    execute(options);
}

void MainWindow::on_actionRun_triggered()
{
    execute(mCommandLineOption->getCurrentOption());
}

void MainWindow::on_actionCompile_triggered()
{
    execute("A=C");
}

void MainWindow::openOrShowContext(FileContext* fileContext)
{
    if (!fileContext) return;
    QPlainTextEdit* edit = nullptr;
    QTabWidget* tabWidget = fileContext->location().isEmpty() ? ui->logTab : ui->mainTab;
    if (!fileContext->editors().empty()) {
        edit = fileContext->editors().first();
    }
    if (edit) {
        tabWidget->setCurrentWidget(edit);
    } else {
        createEdit(tabWidget, fileContext->id());
    }
    if (tabWidget->currentWidget())
        tabWidget->currentWidget()->setFocus();
    if (tabWidget != ui->logTab) {
        // if there is already a log -> show it
        FileContext* logContext = mFileRepo.logContext(fileContext);
        if (logContext && !logContext->editors().isEmpty()) {
            QPlainTextEdit* logEdit = logContext->editors().first();
            if (ui->logTab->currentWidget() != logEdit) {
                ui->logTab->setCurrentWidget(logEdit);
            }
        }
    }
}

void MainWindow::openOrShow(QString filePath, FileGroupContext *parent, bool openedManually)
{
    QFileInfo fileInfo(filePath);
    FileSystemContext *fsc = mFileRepo.findContext(filePath, parent);
    FileContext *fc = (fsc && fsc->type() == FileSystemContext::File) ? static_cast<FileContext*>(fsc) : nullptr;

    if (!fsc) { // not yet opened by user, open file in new tab
        QString additionalFile = openedManually ? fileInfo.fileName() : "";
        FileGroupContext* group = mFileRepo.ensureGroup(fileInfo.canonicalFilePath(), additionalFile);

        fsc = mFileRepo.findContext(filePath, group);
        if (!fsc) {
            EXCEPT() << "File not found: " << filePath;
        }
        if (fsc->type() == FileSystemContext::File) {
            QTabWidget* tabWidget = fsc->location().isEmpty() ? ui->logTab : ui->mainTab;
            fc = static_cast<FileContext*>(fsc);
            createEdit(tabWidget, fc->id());
            if (tabWidget->currentWidget())
                tabWidget->currentWidget()->setFocus();
            ui->projectView->expand(mFileRepo.treeModel()->index(group));
        }
    } else if (fc) {
        openOrShowContext(fc);
    }
    if (!fc) {
        EXCEPT() << "invalid pointer found: FileContext expected.";
    }
    mRecent.path = fileInfo.path();
    mRecent.group = fc->parentEntry();
}

FileContext* MainWindow::addContext(const QString &path, const QString &fileName, bool openedManually)
{
    FileContext *fc = nullptr;
    if (!fileName.isEmpty()) {
        QFileInfo fInfo(path, fileName);


        FileType fType = FileType::from(fInfo.suffix());

        if (fType == FileType::Gsp) {
            // TODO(JM) Read project and create all nodes for associated files
        } else {
            openOrShow(fInfo.filePath(), nullptr, openedManually); // open all sorts of files
        }
    }
    return fc;
}

void MainWindow::openContext(const QModelIndex& index)
{
    FileSystemContext *fsc = static_cast<FileSystemContext*>(index.internalPointer());
    if (fsc && fsc->type() == FileSystemContext::File) {
        FileContext *fc = static_cast<FileContext*>(fsc);
        openOrShowContext(fc);
    }
}

void MainWindow::on_mainTab_currentChanged(int index)
{
    QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(ui->mainTab->widget(index));
    if (edit) {
        mFileRepo.editorActivated(edit);
        // if there is already a log -> show it
        FileContext* fc = mFileRepo.fileContext(edit);
        if (fc && mRecent.group != fc->parentEntry()) {
            mRecent.group = fc->parentEntry();
            updateRunState();
        }
        LogContext* logContext = mFileRepo.logContext(fc);
        if (logContext && !logContext->editors().isEmpty()) {
            QPlainTextEdit* logEdit = logContext->editors().first();
            if (ui->logTab->currentWidget() != logEdit) {
                ui->logTab->setCurrentWidget(logEdit);
            }
        }
    }
}

}
}
