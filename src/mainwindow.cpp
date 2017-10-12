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
#include "modeldialog.h"
#include "exception.h"
#include "treeitemdelegate.h"
#include "gamsinfo.h"
#include "newdialog.h"

namespace gams {
namespace studio {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->treeView->setModel(&mFileRepo);
    ui->treeView->setRootIndex(mFileRepo.rootTreeModelIndex());
    mFileRepo.setSuffixFilter(QStringList() << ".gms" << ".inc" << ".log" << ".lst" << ".txt");
    mFileRepo.setDefaultActions(QList<QAction*>() << ui->actionNew << ui->actionOpen);
    ui->treeView->setHeaderHidden(true);
    ui->treeView->setItemDelegate(new TreeItemDelegate(ui->treeView));
    ui->treeView->setIconSize(QSize(15,15));
    ui->mainToolBar->setIconSize(QSize(21,21));
    ui->processWindow->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    connect(this, &MainWindow::processOutput, this, &MainWindow::appendOutput);
    initTabs();
    mCodecGroup = new QActionGroup(this);
    connect(mCodecGroup, &QActionGroup::triggered, this, &MainWindow::codecChanged);
    connect(ui->mainTab, &QTabWidget::currentChanged, this, &MainWindow::activeTabChanged);
    connect(&mFileRepo, &FileRepository::fileClosed, this, &MainWindow::fileClosed);
    connect(&mFileRepo, &FileRepository::fileChangedExtern, this, &MainWindow::fileChangedExtern);
    connect(&mFileRepo, &FileRepository::fileDeletedExtern, this, &MainWindow::fileDeletedExtern);
    connect(ui->treeView, &QTreeView::clicked, &mFileRepo, &FileRepository::nodeClicked);
    ensureCodecMenu("System");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initTabs()
{
    ui->mainTab->addTab(new WelcomePage(), QString("Welcome"));
    hasWelcomePage = true;
}

void MainWindow::createEdit(QTabWidget* tabWidget, QString codecName)
{
    createEdit(tabWidget, -1, codecName);
}

void MainWindow::createEdit(QTabWidget *tabWidget, int id, QString codecName)
{
    FileContext *fc = mFileRepo.fileContext(id);
    if (fc) {
        CodeEditor *codeEdit = new CodeEditor(this);
        fc->addEditor(codeEdit);
        int tabIndex = tabWidget->addTab(codeEdit, fc->caption());
        tabWidget->setTabToolTip(tabIndex, fc->location());
        tabWidget->setCurrentIndex(tabIndex);
        fc->load(codecName);
        ensureCodecMenu(fc->codec());
        connect(fc, &FileContext::changed, this, &MainWindow::fileChanged);
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

void MainWindow::on_actionNew_triggered()
{
    NewDialog dialog(this);
    dialog.exec();
    auto fileName = dialog.fileName();
    auto location = dialog.location();

    QFile file(QDir(location).filePath(fileName));
    if (file.open(QIODevice::WriteOnly))
        file.close();

    if (FileContext *fc = addContext(location, fileName)) {
        fc->save();
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString fName = QFileDialog::getOpenFileName(this,
                                                 "Open file",
                                                 mRecent.path,
                                                 tr("GAMS code (*.gms *.inc );;"
                                                    "Text files (*.txt);;"
                                                    "All files (*)"));
    addContext("", fName);
}

void MainWindow::on_actionSave_triggered()
{
    // TODO(JM) with multiple open windows we need to store focus changes to get last active editor
    // CodeEditor* edit = qobject_cast<CodeEditor*>(ui->mainTab->currentWidget());
    FileContext* fc = mFileRepo.fileContext(mRecent.editFileId);
    if (!fc) return;
    if (fc->location().isEmpty()) {
        on_actionSave_As_triggered();
    } else if (fc->crudState() == CrudState::eUpdate) {
        fc->save();
    }
}

void MainWindow::on_actionSave_As_triggered()
{
    QString path = mRecent.path;
    if (mRecent.editFileId >= 0) {
        FileContext *fc = mFileRepo.fileContext(mRecent.editFileId);
        if (fc) path = QFileInfo(fc->location()).path();
    }
    auto fileName = QFileDialog::getSaveFileName(this,
                                                 "Save file as...",
                                                 path,
                                                 tr("GAMS code (*.gms *.inc );;"
                                                 "Text files (*.txt);;"
                                                 "All files (*)"));
    if (!fileName.isEmpty()) {
        mRecent.path = QFileInfo(fileName).path();
        FileContext* fc = mFileRepo.fileContext(mRecent.editFileId);
        if (!fc) return;
        // TODO(JM) renaming should create a new node (and maybe a new group)
        fc->setLocation(fileName);
        fc->save();
    }
}

void MainWindow::on_actionSave_All_triggered()
{
    QMessageBox::information(this, "Save All", "t.b.d.");
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
    qDebug() << "current index " << except;
    for(int i = ui->mainTab->count(); i >= 0; i--) {
        if(i != except) {
            qDebug() << "removing tab " << i;
            on_mainTab_tabCloseRequested(i);
        }
    }
}

void MainWindow::clearProc(int exitCode)
{
    Q_UNUSED(exitCode);
    if (mProc) {
        qDebug() << "clear process";
        mProc->deleteLater();
        mProc = nullptr;
    }
}

void MainWindow::addLine(QProcess::ProcessChannel channel, QString text)
{
    ui->processWindow->setTextColor(channel ? Qt::red : Qt::black);
    emit processOutput(text);
}

void MainWindow::readyStdOut()
{
    mOutputMutex.lock();
    mProc->setReadChannel(QProcess::StandardOutput);
    bool avail = mProc->bytesAvailable();
    mOutputMutex.unlock();

    while (avail) {
        mOutputMutex.lock();
        mProc->setReadChannel(QProcess::StandardOutput);
        addLine(QProcess::StandardOutput, mProc->readLine());
        avail = mProc->bytesAvailable();
        mOutputMutex.unlock();
    }
}

void MainWindow::readyStdErr()
{
    mOutputMutex.lock();
    mProc->setReadChannel(QProcess::StandardError);
    bool avail = mProc->bytesAvailable();
    mOutputMutex.unlock();

    while (avail) {
        mOutputMutex.lock();
        mProc->setReadChannel(QProcess::StandardError);
        addLine(QProcess::StandardError, mProc->readLine());
        avail = mProc->bytesAvailable();
        mOutputMutex.unlock();
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

    QMessageBox msgBox;
    msgBox.setWindowTitle("File modified");

    // TODO(JM) Handle other file-types

    // file is loaded but unchanged: ASK, if it should be reloaded
    if (fc->crudState() == CrudState::eRead) {
        msgBox.setText(fc->location()+" has been modified externally.");
        msgBox.setInformativeText("Reload?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    }

    // file has been changed in the editor: ASK, if intern or extern version should be kept.
    if (fc->crudState() == CrudState::eUpdate) {
        msgBox.setText(fc->location() + " has been modified concurrently.");
        msgBox.setInformativeText("Do you want to"
                                  "\n- <b>Discard</b> your changes and reload the file"
                                  "\n- <b>Ignore</b> the external changes and keep your changes");
        msgBox.setStandardButtons(QMessageBox::Discard | QMessageBox::Ignore);
    }

    msgBox.setDefaultButton(QMessageBox::NoButton);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Yes || ret == QMessageBox::Discard) {
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

void MainWindow::appendOutput(QString text)
{
    QTextEdit *outWin = ui->processWindow;
    outWin->moveCursor(QTextCursor::End);
    outWin->insertPlainText(text);
    outWin->moveCursor(QTextCursor::End);
}

void MainWindow::postGamsRun()
{
    if(mProcLstFileInfo.exists())
        openOrShow(mProcLstFileInfo.absoluteFilePath(), mProcFgc);
    else
        qDebug() << mProcLstFileInfo.absoluteFilePath() << " not found. aborting.";
    ui->actionRun->setEnabled(true);
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
    auto systemDir = GAMSInfo::systemDir();
    QProcess process(this);
    process.start(systemDir + "/gams ? lo=3");
    QString info;
    if (process.waitForFinished()) {
        info = process.readAllStandardOutput();;
    }
    QMessageBox::about(this, "About GAMS Studio", info);
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::on_actionProject_Explorer_triggered(bool checked)
{
    if(checked)
        ui->dockProjectExplorer->show();
    else
        ui->dockProjectExplorer->hide();
}

void MainWindow::on_actionBottom_Panel_triggered(bool checked)
{
    if(checked)
        ui->dockBottom->show();
    else
        ui->dockBottom->hide();
}

void MainWindow::on_mainTab_tabCloseRequested(int index)
{
    QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(ui->mainTab->widget(index));
    FileContext* fc = mFileRepo.fileContext(edit);
    if (!fc) {
        ui->mainTab->removeTab(index);
        // assuming we are closing a welcome page here
        hasWelcomePage = false;
        return;
    }

    int ret = QMessageBox::Discard;
    if (fc->editors().size() == 1 && fc->crudState() == CrudState::eUpdate) {
        // only ask, if this is the last editor of this file
        QMessageBox msgBox;
        msgBox.setText(ui->mainTab->tabText(index)+" has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        ret = msgBox.exec();
    }
    if (ret == QMessageBox::Save) fc->save();
    if (ret != QMessageBox::Cancel) {
        if (fc->editors().size() == 1) {
            mFileRepo.close(fc->id());
        } else {
            fc->removeEditor(edit);
            ui->mainTab->removeTab(ui->mainTab->indexOf(edit));
        }
    }
}

void MainWindow::on_actionShow_Welcome_Page_triggered()
{
    if(!hasWelcomePage) {
        ui->mainTab->insertTab(0, new WelcomePage(), QString("Welcome")); // always first position
        hasWelcomePage = true;
    } else {
        // TODO: jump to welcome page
    }
}

void MainWindow::on_actionGAMS_Library_triggered()
{
    ModelDialog dialog(this);
    if(dialog.exec() == QDialog::Accepted)
    {
        QMessageBox msgBox;
        LibraryItem *item = dialog.selectedLibraryItem();

        QProcess libProc(this);

        QString libExec = QDir(GAMSInfo::systemDir()).filePath(item->library()->execName());

        //TODO(CW): is this the correct working directory? We need a descent working directory
        libProc.setWorkingDirectory(".");
        QStringList args(item->name());
        libProc.start(libExec, args);
        libProc.waitForFinished();

        //TODO(CW): check if the creation of fileName is correct
        QString fileName = item->files().at(0).split(".").at(0) + ".gms";
        FileContext *fc = addContext(".", fileName);
        openOrShow(fc);
    }
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    FileSystemContext *fsc = static_cast<FileSystemContext*>(index.internalPointer());
    if (fsc && fsc->type() == FileSystemContext::File) {
        FileContext *fc = static_cast<FileContext*>(fsc);
        openOrShow(fc);
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QSet<int> openIds;
    QString lastName;
    for (QPlainTextEdit *ed: mFileRepo.editors(mFileRepo.groupContext(mFileRepo.rootTreeModelIndex())->id())) {
        FileContext *fc = mFileRepo.fileContext(ed);
        if (!ed) continue;
        CrudState cs = fc->crudState();
        if (cs == CrudState::eUpdate || cs == CrudState::eCreate) {
            qDebug() << "CRUD-state: " << (int)cs;
            openIds << fc->id();
            lastName = fc->location();
        }
    }
    if (openIds.size() > 0) {
        int ret = QMessageBox::Discard;
        QMessageBox msgBox;
        QString filesText = openIds.size()==1 ? lastName+" has been modified."
                                              : QString::number(openIds.size())+" files have been modified";
        msgBox.setText(filesText);
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        ret = msgBox.exec();
        if (ret == QMessageBox::Save) {
            // TODO(JM) iterate over all files to save.
        }
        if (ret == QMessageBox::Cancel) {
            event->setAccepted(false);
        }
    }
}

void MainWindow::on_actionRun_triggered()
{
    QString gamsPath = GAMSInfo::systemDir() + "/gams";
    // TODO: add option to clear output view before running next job

    FileContext* fc = mFileRepo.fileContext(mRecent.editor);
    FileGroupContext *fgc = (fc ? fc->parentEntry() : nullptr);
    mProcFgc = fgc;
    if (!fgc)
        return;

    ui->actionRun->setEnabled(false);

    qDebug() << "starting process";
    mProc = new QProcess(this);

    QString gmsFilePath = fgc->runableGms();
    QFileInfo gmsFileInfo(gmsFilePath);
    QString basePath = gmsFileInfo.absolutePath();

    QString lstFileName = gmsFileInfo.completeBaseName() + ".lst"; // TODO: add .log and others
    mProcLstFileInfo = QFileInfo(basePath + "/" + lstFileName);

    mProc->setWorkingDirectory(gmsFileInfo.path());
    QStringList args(QDir::toNativeSeparators(gmsFilePath)); //TODO(CW): call toNativeSeparators here or in runableGms?
    mProc->start(gamsPath, args);

    connect(mProc, &QProcess::readyReadStandardOutput, this, &MainWindow::readyStdOut);
    connect(mProc, &QProcess::readyReadStandardError, this, &MainWindow::readyStdErr);
    connect(mProc, static_cast<void(QProcess::*)(int)>(&QProcess::finished), this, &MainWindow::clearProc);
    connect(mProc, static_cast<void(QProcess::*)(int)>(&QProcess::finished), this, &MainWindow::postGamsRun);

}

void MainWindow::openOrShow(FileContext* fileContext)
{
    if (!fileContext) return;
    QPlainTextEdit* edit = fileContext->editors().empty() ? nullptr : fileContext->editors().first();
    if (edit) {
        ui->mainTab->setCurrentWidget(edit);
    } else {
        createEdit(ui->mainTab, fileContext->id());
    }
}

void MainWindow::openOrShow(QString filePath, FileGroupContext *parent)
{
    QFileInfo fileInfo(filePath);
    QModelIndex qmi = mFileRepo.findEntry(fileInfo.fileName(), fileInfo.filePath(), mFileRepo.index(parent));
    if (!qmi.isValid()) {
        // not yet opened by user, open file in new tab
        QModelIndex groupMI = mFileRepo.ensureGroup(fileInfo.fileName());
        qmi = mFileRepo.addFile(fileInfo.fileName(), fileInfo.canonicalFilePath(), groupMI);
    }
    FileSystemContext *fsc = static_cast<FileSystemContext*>(qmi.internalPointer());
    if (fsc->type() != FileSystemContext::File) {
        EXCEPT() << "invalid pointer found: FileContext expected.";
    }
    openOrShow(static_cast<FileContext*>(fsc));
}

FileContext* MainWindow::addContext(const QString &path, const QString &fileName)
{
    FileContext *fc = nullptr;
    if (!fileName.isEmpty()) {
        QFileInfo fInfo(path, fileName);
        // TODO(JM) extend for each possible type
        qDebug() << "Type: " << fInfo.suffix();
        FileType fType = (fInfo.suffix() == "gms") ? FileType::ftGms : FileType::ftTxt;

        if (fType == FileType::ftGms || fType == FileType::ftTxt) {
            // Create node for GIST directory and load all files of known filetypes
            FileSystemContext *hit = mFileRepo.findFile(fInfo.filePath());
            if(hit == nullptr) {
                QModelIndex groupMI = mFileRepo.ensureGroup(fInfo.canonicalFilePath());

                QModelIndex fileMI = mFileRepo.addFile(fInfo.fileName(), fInfo.canonicalFilePath(), groupMI);
                FileContext *fc = mFileRepo.fileContext(fileMI);
                createEdit(ui->mainTab, fc->id());
                ui->treeView->expand(groupMI);
                mRecent.path = fInfo.path();
            } else {
                auto groupContext = static_cast<FileGroupContext*>(mFileRepo.findFile(fInfo.absolutePath() +
                                                                                      "/" + fInfo.baseName()));
                openOrShow(fInfo.filePath(), groupContext);
            }
        }
        if (fType == FileType::ftGsp) {
            // TODO(JM) Read project and create all nodes for associated files
        }
    }
    return fc;
}

}
}
