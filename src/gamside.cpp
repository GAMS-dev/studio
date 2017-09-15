#include "gamside.h"
#include "ui_gamside.h"
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "codeeditor.h"
#include "tabwidget.h"

using namespace gams::ide;

GAMSIDE::GAMSIDE(QWidget *parent) : QMainWindow(parent), ui(new Ui::GAMSIDE)
{
    ui->setupUi(this);
//    ui->dockBottom->hide();
    ui->treeView->setModel(&mFileRepo);
    ui->treeView->setRootIndex(mFileRepo.rootTreeModelIndex());
    ui->treeView->setHeaderHidden(true);
    connect(this, &GAMSIDE::processOutput, ui->processWindow, &QTextEdit::append);
    mCodecGroup = new QActionGroup(this);
    connect(mCodecGroup, &QActionGroup::triggered, this, &GAMSIDE::codecChanged);
    ensureCodecMenue("System");
}

GAMSIDE::~GAMSIDE()
{
    delete ui;
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

//    auto fileName =
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
    if(checked)
        ui->dockOutput->show();
    else
        ui->dockOutput->hide();
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






















