/*
 * This file is part of the GAMS IDE project.
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
#include "gamside.h"
#include "ui_gamside.h"
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "codeeditor.h"
#include "tabwidget.h"
#include "welcomepage.h"
#include "editor.h"

#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <iostream>
#include <QTime>
#include <QDebug>
#include <QFileDialog>

#include "modeldialog.h"

using namespace gams::ide;

GAMSIDE::GAMSIDE(QWidget *parent) : QMainWindow(parent), ui(new Ui::GAMSIDE)
{
    ui->setupUi(this);

    ui->treeView->setModel(&mFileRepo);
    ui->treeView->setRootIndex(mFileRepo.rootTreeModelIndex());
    ui->treeView->setHeaderHidden(true);
    connect(this, &GAMSIDE::processOutput, ui->processWindow, &QTextEdit::append);
    initTabs();
    mCodecGroup = new QActionGroup(this);
    connect(mCodecGroup, &QActionGroup::triggered, this, &GAMSIDE::codecChanged);
    ensureCodecMenue("System");
}

GAMSIDE::~GAMSIDE()
{
    delete ui;
}

void GAMSIDE::initTabs()
{
    ui->mainTab->addTab(new WelcomePage(), QString("Welcome"));
    ui->mainTab->addTab(new Editor(), QString("$filename"));
}

void GAMSIDE::createEdit(TabWidget* tabWidget, QString codecName)
{
    createEdit(tabWidget, -1, codecName);
}

void GAMSIDE::createEdit(TabWidget *tabWidget, int id, QString codecName)
{
    QStringList codecNames;
    if (!codecName.isEmpty()) {
        codecNames << codecName;
    } else {
        // Most error-prone codecs first and non-unicode last to prevent early false-success
        codecNames << "Utf-8" << "Shift-JIS" << "GB2312" << "System" << "Windows-1250" << "Latin-1";
    }

    FileContext *fc = mFileRepo.fileContext(id);
    if (fc) {
        CodeEditor *codeEdit = new CodeEditor(this);
        codeEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        int tabIndex = tabWidget->addTab(codeEdit, fc->name(), id);
        tabWidget->setTabToolTip(tabIndex, fc->location());
        tabWidget->setCurrentIndex(tabIndex);
        QFile file(fc->location());
        if (!file.fileName().isEmpty() && file.exists()) {
            if (file.open(QFile::ReadOnly | QFile::Text)) {
                const QByteArray data(file.readAll());
                QString text;
                QString nameOfUsedCodec;
                for (QString tcName: codecNames) {
                    QTextCodec::ConverterState state;
                    QTextCodec *codec = QTextCodec::codecForName(tcName.toLatin1().data());
                    if (codec) {
                        nameOfUsedCodec = tcName;
                        text = codec->toUnicode(data.constData(), data.size(), &state);
                        if (state.invalidChars == 0) {
                            qDebug() << "opened with codec " << nameOfUsedCodec;
                            break;
                        }
                        qDebug() << "Codec " << nameOfUsedCodec << " contains " << QString::number(state.invalidChars) << "invalid chars.";
                    } else {
                        qDebug() << "System doesn't contain codec " << nameOfUsedCodec;
                        nameOfUsedCodec = QString();
                    }
                }
                if (!nameOfUsedCodec.isEmpty()) {
                    codeEdit->setPlainText(text);
                    fc->setCodec(nameOfUsedCodec);
                    ensureCodecMenue(nameOfUsedCodec);
                }
                file.close();
            }
        }
        connect(codeEdit, &CodeEditor::textChanged, fc, &FileContext::textChanged);
        connect(fc, &FileContext::nameChanged, ui->mainTab, &TabWidget::tabNameChanged);
    }
}

void GAMSIDE::ensureCodecMenue(QString codecName)
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

void GAMSIDE::on_actionNew_triggered()
{
    QMessageBox::information(this, "New...", "t.b.d.");

}

void GAMSIDE::on_actionOpen_triggered()
{
    QString fName = QFileDialog::getOpenFileName(this,
                                                 "Open file",
                                                 ".",
                                                 tr("GAMS code (*.gms *.inc );;"
                                                    "Text files (*.txt);;"
                                                    "All files (*)"));
    if (!fName.isEmpty()) {
        QFileInfo fInfo(fName);
        // TODO(JM) extend for each possible type
        qDebug() << "Type: " << fInfo.suffix();
        FileType fType = (fInfo.suffix() == "gms") ? FileType::ftGms : FileType::ftTxt;

        if (fType == FileType::ftGms) {
            // Create node for GIST directory and load all files of known filetypes
            QString dir = fInfo.path();
            QModelIndex groupMI = mFileRepo.find(dir, mFileRepo.rootTreeModelIndex());
            if (!groupMI.isValid()) {
                groupMI = mFileRepo.addGroup(dir, dir, true, mFileRepo.rootTreeModelIndex());
            }
            QModelIndex fileMI = mFileRepo.addFile(fInfo.fileName(), fInfo.filePath(), true, groupMI);
            FileContext *fc = static_cast<FileContext*>(fileMI.internalPointer());
            createEdit(ui->mainTab, fc->id());
            ui->treeView->expand(groupMI);
        }
    }
}

void GAMSIDE::on_actionSave_triggered()
{
    // TODO(JM) with multiple open windows we need to store focus changes to get last active editor
    // CodeEditor* edit = qobject_cast<CodeEditor*>(ui->mainTab->currentWidget());
}

void GAMSIDE::on_actionSave_As_triggered()
{
    auto fileName = QFileDialog::getSaveFileName(this,
                                                 "Save file as...",
                                                 ".",
                                                 tr("GAMS code (*.gms *.inc );;"
                                                 "Text files (*.txt);;"
                                                 "All files (*)"));
}

void GAMSIDE::on_actionSave_All_triggered()
{
    QMessageBox::information(this, "Save All", "t.b.d.");
}

void GAMSIDE::on_actionClose_triggered()
{
    ui->mainTab->removeTab(ui->mainTab->currentIndex());
}

void GAMSIDE::on_actionClose_All_triggered()
{
    for(int i = ui->mainTab->count(); i > 0; i--) {
        ui->mainTab->removeTab(0);
    }
}

void GAMSIDE::clearProc(int exitCode)
{
    Q_UNUSED(exitCode);
    if (mProc) {
        qDebug() << "clear process";
        mProc->deleteLater();
        mProc = nullptr;
    }
}

void GAMSIDE::addLine(QProcess::ProcessChannel channel, QString text)
{
    ui->processWindow->setTextColor(channel ? Qt::red : Qt::black);
    emit processOutput(text);
}

void GAMSIDE::readyStdOut()
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

void GAMSIDE::readyStdErr()
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

void GAMSIDE::codecChanged(QAction *action)
{
    qDebug() << "Codec action triggered: " << action->text();
}

void GAMSIDE::on_actionExit_Application_triggered()
{
    QCoreApplication::quit();
}

void GAMSIDE::on_actionOnline_Help_triggered()
{
    QDesktopServices::openUrl(QUrl("https://www.gams.com/latest/docs", QUrl::TolerantMode));
}

void GAMSIDE::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About GAMSIDE", "Gams Studio v0.0.1 alpha");
}

void GAMSIDE::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void GAMSIDE::on_actionProject_Explorer_triggered(bool checked)
{
    if(checked)
        ui->dockProjectExplorer->show();
    else
        ui->dockProjectExplorer->hide();
}

void GAMSIDE::on_actionLog_Output_triggered(bool checked)
{

}

void GAMSIDE::on_actionBottom_Panel_triggered(bool checked)
{
    if(checked)
        ui->dockBottom->show();
    else
        ui->dockBottom->hide();
}

void GAMSIDE::on_actionSim_Process_triggered()
{
    qDebug() << "starting process";
    mProc = new QProcess(this);
    mProc->start("../../spawner/spawner.exe");
    connect(mProc, &QProcess::readyReadStandardOutput, this, &GAMSIDE::readyStdOut);
    connect(mProc, &QProcess::readyReadStandardError, this, &GAMSIDE::readyStdErr);
    connect(mProc, static_cast<void(QProcess::*)(int)>(&QProcess::finished), this, &GAMSIDE::clearProc);
}

void GAMSIDE::on_mainTab_tabCloseRequested(int index)
{
    ui->mainTab->removeTab(index);
}

void GAMSIDE::on_actionShow_Welcome_Page_triggered()
{
    ui->mainTab->insertTab(0, new WelcomePage(), QString("Welcome")); // always first position
}

void GAMSIDE::on_actionNew_Tab_triggered()
{
    ui->mainTab->addTab(new Editor(), QString("new"));
}

void GAMSIDE::on_actionGAMS_Library_triggered()
{
    ModelDialog dialog;
    dialog.exec();
}
