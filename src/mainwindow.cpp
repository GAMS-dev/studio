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

namespace gams {
namespace studio {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops(true);

    ui->projectView->setModel(&mFileRepo);
    ui->projectView->setRootIndex(mFileRepo.rootTreeModelIndex());
    mFileRepo.setSuffixFilter(QStringList() << ".gms" << ".inc" << ".log" << ".lst" << ".txt");
    mFileRepo.setDefaultActions(QList<QAction*>() << ui->actionNew << ui->actionOpen);
    ui->projectView->setHeaderHidden(true);
    ui->projectView->setItemDelegate(new TreeItemDelegate(ui->projectView));
    ui->projectView->setIconSize(QSize(15,15));
    ui->mainToolBar->setIconSize(QSize(21,21));
    ui->outputView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    connect(this, &MainWindow::processOutput, this, &MainWindow::appendOutput);
    initTabs();
    mCodecGroup = new QActionGroup(this);
    connect(mCodecGroup, &QActionGroup::triggered, this, &MainWindow::codecChanged);
    connect(ui->mainTab, &QTabWidget::currentChanged, this, &MainWindow::activeTabChanged);
    connect(&mFileRepo, &FileRepository::fileClosed, this, &MainWindow::fileClosed);
    connect(&mFileRepo, &FileRepository::fileChangedExtern, this, &MainWindow::fileChangedExtern);
    connect(&mFileRepo, &FileRepository::fileDeletedExtern, this, &MainWindow::fileDeletedExtern);
    connect(ui->dockOutputView, &QDockWidget::visibilityChanged, this, &MainWindow::setOutputViewVisibility);
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
        if (fc->metrics().fileType() == FileType::Gms || fc->metrics().fileType() == FileType::Lst) {
            CodeEditor *codeEdit = new CodeEditor(this);
            fc->addEditor(codeEdit);
            int tabIndex = tabWidget->addTab(codeEdit, fc->caption());
            tabWidget->setTabToolTip(tabIndex, fc->location());
            tabWidget->setCurrentIndex(tabIndex);
            fc->load(codecName);
            QTextCursor tc = codeEdit->textCursor();
            tc.movePosition(QTextCursor::Start);
            codeEdit->setTextCursor(tc);
            ensureCodecMenu(fc->codec());
            if (fc->metrics().fileType() == FileType::Gms) {
                connect(fc, &FileContext::changed, this, &MainWindow::fileChanged);
            } else {
                codeEdit->setReadOnly(true);
                codeEdit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
            }
        }
        // TODO(JM) other kinds
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
    int i = mFileRepo.saveAll();
    qDebug() << i << (i==1 ? " file" : " files") << " saved.";
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

void MainWindow::addProcessData(QProcess::ProcessChannel channel, QString text)
{
    ui->outputView->setTextColor(channel ? Qt::red : Qt::black);
    emit processOutput(text);
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

void MainWindow::appendOutput(QString text)
{
    QTextEdit *outWin = ui->outputView;
    outWin->moveCursor(QTextCursor::End);
    outWin->insertPlainText(text);
    outWin->moveCursor(QTextCursor::End);
}

void MainWindow::postGamsRun()
{
    QFileInfo fileInfo(mProcess->inputFile());
    if(fileInfo.exists()) // TODO: add .log and others)
        openOrShow(fileInfo.path() + "/" + fileInfo.completeBaseName() + ".lst", mProcess->context());
    else
        qDebug() << fileInfo.absoluteFilePath() << " not found. aborting.";
    if (mProcess) {
        mProcess->deleteLater();
        mProcess = nullptr;
    }
    ui->actionRun->setEnabled(true);
}

void MainWindow::postGamsLibRun()
{// TODO(AF) Are there models without a GMS file? How to handle them?"
    openOrShow(addContext(mLibProcess->targetDir(), mLibProcess->inputFile()));
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
    auto about = GAMSProcess::aboutGAMS();
    QMessageBox::about(this, "About GAMS Studio", about);
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::on_actionOutput_View_triggered(bool checked)
{
    if(checked)
        ui->dockOutputView->show();
    else
        ui->dockOutputView->hide();
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
    QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(ui->mainTab->widget(index));
    FileContext* fc = mFileRepo.fileContext(edit);
    if (!fc) {
        ui->mainTab->removeTab(index);
        // assuming we are closing a welcome page here
        hasWelcomePage = false;
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

void MainWindow::on_actionShow_Welcome_Page_triggered()
{
    if(!hasWelcomePage) {
        ui->mainTab->insertTab(0, new WelcomePage(), QString("Welcome")); // always first position
        hasWelcomePage = true;
    }
    ui->mainTab->setCurrentIndex(0);
}

void MainWindow::on_actionGAMS_Library_triggered()
{
    ModelDialog dialog(this);
    if(dialog.exec() == QDialog::Accepted)
    {
        QMessageBox msgBox;
        LibraryItem *item = dialog.selectedLibraryItem();
        QFileInfo fileInfo(item->files().first());

        mLibProcess = new GAMSLibProcess(this);
        mLibProcess->setApp(item->library()->execName());
        mLibProcess->setModelName(item->name());
        mLibProcess->setInputFile(fileInfo.completeBaseName() + ".gms");
        mLibProcess->setTargetDir(GAMSPaths::defaultWorkingDir());
        mLibProcess->execute();
        connect(mLibProcess, &GAMSProcess::newStdChannelData, this, &MainWindow::addProcessData);
        connect(mLibProcess, &GAMSProcess::finished, this, &MainWindow::postGamsLibRun);
    }
}

void MainWindow::on_projectView_doubleClicked(const QModelIndex &index)
{
    openContext(index);
}

void MainWindow::on_projectView_clicked(const QModelIndex& index)
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
                openOrShow(fi.canonicalFilePath(), nullptr);
            }
        }
    }
}

void MainWindow::on_actionRun_triggered()
{// TODO: add option to clear output view before running next job
    FileContext* fc = mFileRepo.fileContext(mRecent.editor);
    FileGroupContext *fgc = (fc ? fc->parentEntry() : nullptr);
    if (!fgc)
        return;

    ui->actionRun->setEnabled(false);

    QString gmsFilePath = fgc->runableGms();
    QFileInfo gmsFileInfo(gmsFilePath);
    QString basePath = gmsFileInfo.absolutePath();

    mProcess = new GAMSProcess(this);
    mProcess->setWorkingDir(gmsFileInfo.path());
    mProcess->setInputFile(gmsFilePath);
    mProcess->setContext(fgc);
    mProcess->execute();

    connect(mProcess, &GAMSProcess::newStdChannelData, this, &MainWindow::addProcessData);
    connect(mProcess, &GAMSProcess::finished, this, &MainWindow::postGamsRun);
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
    if (ui->mainTab->currentWidget())
        ui->mainTab->currentWidget()->setFocus();
}

void MainWindow::openOrShow(QString filePath, FileGroupContext *parent)
{
    QFileInfo fileInfo(filePath);
    FileSystemContext *fsc = mFileRepo.findFile(filePath, parent);
    if (!fsc) {
        // not yet opened by user, open file in new tab
        QModelIndex groupMI = mFileRepo.ensureGroup(fileInfo.canonicalFilePath());
        QModelIndex fileMI = mFileRepo.addFile(fileInfo.fileName(), fileInfo.canonicalFilePath(), groupMI);
        FileContext *fc = mFileRepo.fileContext(fileMI);
        fsc = fc;
        createEdit(ui->mainTab, fc->id());
        if (ui->mainTab->currentWidget())
            ui->mainTab->currentWidget()->setFocus();
        ui->projectView->expand(groupMI);
    }
    mRecent.path = fileInfo.path();
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

        FileType fType = FileType::from(fInfo.suffix());

        if (fType == FileType::Gms) {
            // Create node for GIST directory and load all files of known filetypes
            openOrShow(fInfo.filePath(), nullptr);
        }
        if (fType == FileType::Gsp) {
            // TODO(JM) Read project and create all nodes for associated files
        }
    }
    return fc;
}

void MainWindow::openContext(const QModelIndex& index)
{
    FileSystemContext *fsc = static_cast<FileSystemContext*>(index.internalPointer());
    if (fsc && fsc->type() == FileSystemContext::File) {
        FileContext *fc = static_cast<FileContext*>(fsc);
        openOrShow(fc);
    }
}

void MainWindow::on_mainTab_currentChanged(int index)
{
    QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(ui->mainTab->widget(index));
    if (edit) mFileRepo.editorActivated(edit);
}

}
}

