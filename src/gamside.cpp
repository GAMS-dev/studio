#include "gamside.h"
#include "ui_gamside.h"
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <iostream>
#include <QTime>
#include <QDebug>
#include <QFileDialog>

GAMSIDE::GAMSIDE(QWidget *parent) : QMainWindow(parent), ui(new Ui::GAMSIDE)
{
    ui->setupUi(this);
//    ui->dockBottom->hide();
    connect(this, &GAMSIDE::processOutput, ui->processWindow, &QTextEdit::append);
}

GAMSIDE::~GAMSIDE()
{
    delete ui;
}

void GAMSIDE::on_actionNew_triggered()
{
    QMessageBox::information(this, "New...", "t.b.d.");
}

void GAMSIDE::on_actionOpen_triggered()
{
    auto fileName = QFileDialog::getOpenFileName(this,
                                                 "Open file",
                                                 ".",
                                                 tr("GAMS code (*.gms *.inc );;"
                                                 "Text files (*.txt);;"
                                                 "All files (*)"));
}

void GAMSIDE::on_actionSave_triggered()
{
    auto fileName = QFileDialog::getSaveFileName(this,
                                                 "Save file as...",
                                                 ".",
                                                 tr("GAMS code (*.gms *.inc );;"
                                                 "Text files (*.txt);;"
                                                 "All files (*)"));
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
    QMessageBox::information(this, "Close", "t.b.d.");
}

void GAMSIDE::on_actionClose_All_triggered()
{
    QMessageBox::information(this, "Close All", "t.b.d.");
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

void GAMSIDE::on_actionExit_Application_triggered()
{
    QCoreApplication::quit();
}

void GAMSIDE::on_actionOnline_Help_triggered()
{
    QDesktopServices::openUrl(QUrl("https://www.gams.com/latest/docs/welcome.html", QUrl::TolerantMode));
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






















