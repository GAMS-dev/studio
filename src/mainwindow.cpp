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

namespace gams {
namespace studio {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->treeView->setModel(&mFileRepo);
    ui->treeView->setRootIndex(mFileRepo.rootTreeModelIndex());
    mFileRepo.setSuffixFilter(QStringList() << ".gms" << ".inc" << ".log" << ".lst" << ".txt");
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
    connect(ui->treeView, &QTreeView::expanded, &mFileRepo, &FileRepository::nodeExpanded);
    ensureCodecMenue("System");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initTabs()
{
    ui->mainTab->addTab(new WelcomePage(), QString("Welcome"));
    // TODO(JM) implement new-file logic
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
        mEditors.insert(codeEdit, fc->id());
        codeEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        int tabIndex = tabWidget->addTab(codeEdit, fc->caption());
        qDebug() << "codeedit-parent-parentWidget == tabedit? " << (codeEdit->parentWidget()->parentWidget() == tabWidget ? "true" : "false");
        tabWidget->setTabToolTip(tabIndex, fc->location());
        tabWidget->setCurrentIndex(tabIndex);
        fc->setDocument(codeEdit->document());
        fc->load(codecName);
        ensureCodecMenue(fc->codec());
        connect(codeEdit, &CodeEditor::textChanged, fc, &FileContext::textChanged);
        connect(fc, &FileContext::changed, this, &MainWindow::fileChanged);
    }
}

void MainWindow::ensureCodecMenue(QString codecName)
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
    QMessageBox::information(this, "New...", "t.b.d.");

}

void MainWindow::on_actionOpen_triggered()
{
    QString fName = QFileDialog::getOpenFileName(this,
                                                 "Open file",
                                                 mRecent.path,
                                                 tr("GAMS code (*.gms *.inc );;"
                                                    "Text files (*.txt);;"
                                                    "All files (*)"));
    if (!fName.isEmpty()) {
        QFileInfo fInfo(fName);
        // TODO(JM) extend for each possible type
        qDebug() << "Type: " << fInfo.suffix();
        FileType fType = (fInfo.suffix() == "gms") ? FileType::ftGms : FileType::ftTxt;

        if (fType == FileType::ftGms || fType == FileType::ftTxt) {
            // Create node for GIST directory and load all files of known filetypes
            QModelIndex groupMI = mFileRepo.ensureGroup(fInfo.canonicalFilePath());

            QModelIndex fileMI = mFileRepo.addFile(fInfo.fileName(), fInfo.canonicalFilePath(), groupMI);
            FileContext *fc = static_cast<FileContext*>(fileMI.internalPointer());
            createEdit(ui->mainTab, fc->id());
            ui->treeView->expand(groupMI);
        }
        if (fType == FileType::ftGpr) {
            // TODO(JM) Read project and create all nodes for associated files

        }
    }
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
    auto fileName = QFileDialog::getSaveFileName(this,
                                                 "Save file as...",
                                                 mRecent.path,
                                                 tr("GAMS code (*.gms *.inc );;"
                                                 "Text files (*.txt);;"
                                                 "All files (*)"));
    if (!fileName.isEmpty()) {
        mRecent.path = QFileInfo(fileName).path();
        FileContext* fc = mFileRepo.fileContext(mRecent.editFileId);
        if (!fc) return;
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
    ui->mainTab->removeTab(ui->mainTab->currentIndex());
}

void MainWindow::on_actionClose_All_triggered()
{
    for(int i = ui->mainTab->count(); i > 0; i--) {
        ui->mainTab->removeTab(0);
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
    QWidget *edit = nullptr;
    if (index >= 0)
        edit = ui->mainTab->widget(index);
    if (edit) {
        int fileId = mEditors.value(edit, -1);
        if (fileId >= 0) {
            mRecent.editFileId = fileId;
            mRecent.editor = edit;
        }
    }
}

void MainWindow::fileChanged(int fileId)
{
    QWidgetList editors = mEditors.keys(fileId);
    for (QWidget *edit: editors) {
        int index = ui->mainTab->indexOf(edit);
        if (index >= 0) {
            FileContext *fc = mFileRepo.fileContext(fileId);
            if (fc) ui->mainTab->setTabText(index, fc->caption());
        }
    }
}

void MainWindow::fileClosed(int fileId)
{
    QWidgetList editors = mEditors.keys(fileId);
    for (QWidget *edit: editors) {
        ui->mainTab->removeTab(ui->mainTab->indexOf(edit));
        edit->deleteLater();
        mEditors.remove(edit);
    }
}

void MainWindow::appendOutput(QString text)
{
    QTextEdit *outWin = ui->processWindow;
    outWin->moveCursor(QTextCursor::End);
    outWin->insertPlainText(text);
    outWin->moveCursor(QTextCursor::End);
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
    QMessageBox::about(this, "About GAMS Studio", "Gams Studio v0.0.1 alpha");
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

void MainWindow::on_actionLog_Output_triggered(bool checked)
{
    Q_UNUSED(checked)
}

void MainWindow::on_actionBottom_Panel_triggered(bool checked)
{
    if(checked)
        ui->dockBottom->show();
    else
        ui->dockBottom->hide();
}

void MainWindow::on_actionSim_Process_triggered()
{
    mFileRepo.dump(static_cast<FileSystemContext*>(mFileRepo.rootModelIndex().internalPointer()));
}

void MainWindow::on_mainTab_tabCloseRequested(int index)
{
    QMessageBox msgBox;
    int ret = QMessageBox::Discard;
    int fileId = mEditors.value(ui->mainTab->widget(index), -1);
    if (mEditors.keys(fileId).size() == 1) {
        FileContext *fc = mFileRepo.fileContext(fileId);
        if (fc && fc->crudState() == CrudState::eUpdate) {
            // only ask, if this is the last editor of this file
            msgBox.setText(ui->mainTab->tabText(index)+" has been modified.");
            msgBox.setInformativeText("Do you want to save your changes?");
            msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Save);
            ret = msgBox.exec();
        }
    }
    if (ret == QMessageBox::Save) {
        FileContext *fc = mFileRepo.fileContext(fileId);
        if (!fc)
            throw FATAL() << "Could not find file context to closed editor";
        fc->save();
    }
    if (ret != QMessageBox::Cancel) {
        int id = mEditors.value(ui->mainTab->widget(index));
        // TODO(JM) close the file after last tab-remove
        mFileRepo.close(id);
    }
}

void MainWindow::on_actionShow_Welcome_Page_triggered()
{
    ui->mainTab->insertTab(0, new WelcomePage(), QString("Welcome")); // always first position
}

void MainWindow::on_actionNew_Tab_triggered()
{
    ui->mainTab->addTab(new CodeEditor(), QString("new"));
}

void MainWindow::on_actionGAMS_Library_triggered()
{
    ModelDialog dialog;
    dialog.exec();
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    FileContext *fc = static_cast<FileContext*>(index.internalPointer());
    if (fc) {
        QWidget* edit = mEditors.key(fc->id(), nullptr);
        if (edit) {
            ui->mainTab->setCurrentWidget(edit);
        } else {
            createEdit(ui->mainTab, fc->id());
        }
    }
}

void MainWindow::on_actionRunWithGams_triggered()
{
    // TODO: get gams path dynamically
    QString gamsPath = "/home/rogo/gams/gams24.9_linux_x64_64_sfx/gams";

    // TODO: add option to clear output view before running next job
    int fileId = mEditors.value(mRecent.editor);
    FileGroupContext *fgc = (FileGroupContext*)mFileRepo.fileContext(fileId)->parent();
    QString filePath = fgc->runableGms();

    qDebug() << "starting process";
    mProc = new QProcess(this);
    mProc->start(gamsPath + " " + filePath);
    connect(mProc, &QProcess::readyReadStandardOutput, this, &MainWindow::readyStdOut);
    connect(mProc, &QProcess::readyReadStandardError, this, &MainWindow::readyStdErr);
    connect(mProc, static_cast<void(QProcess::*)(int)>(&QProcess::finished), this, &MainWindow::clearProc);
}

}
}



