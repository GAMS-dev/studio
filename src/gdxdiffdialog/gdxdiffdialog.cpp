#include "gdxdiffdialog.h"
#include "ui_gdxdiffdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <editors/viewhelper.h>
#include <gdxviewer/gdxviewer.h>

namespace gams {
namespace studio {
namespace gdxdiffdialog {

GdxDiffDialog::GdxDiffDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GdxDiffDialog),
    mProc(new GdxDiffProcess(this))
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->lineEdit_4->setValidator(new QDoubleValidator());
    ui->lineEdit_5->setValidator(new QDoubleValidator());

    connect(mProc.get(), &GdxDiffProcess::finished, this, &GdxDiffDialog::diffDone);

    connect(ui->leDiff, &QLineEdit::textEdited, [this]() {mPrepopulateDiff = false;});
    connect(ui->leInput1, &QLineEdit::textChanged, [this]() {prepopulateDiff();});

    adjustSize();
    reset();
}

GdxDiffDialog::~GdxDiffDialog()
{
    cancelProcess(50);
    delete ui;
}

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbInput1_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose GDX File...",
                                                            mRecentPath,
                                                            tr("GDX file (*.gdx);;"
                                                               "All files (*.*)"));
    if (!filePath.isEmpty()) {
        mRecentPath = QFileInfo(filePath).path();
        setInput1(filePath);
    }
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbInput2_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose GDX File...",
                                                            mRecentPath,
                                                            tr("GDX file (*.gdx);;"
                                                               "All files (*.*)"));
    if (!filePath.isEmpty()) {
        mRecentPath = QFileInfo(filePath).path();
        setInput2(filePath);
    }
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbDiff_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Choose GDX File...",
                                                            QDir::cleanPath(mRecentPath + QDir::separator() + defaultDiffFile) ,
                                                            tr("GDX file (*.gdx);;"
                                                               "All files (*.*)"), nullptr,
                                                    QFileDialog::DontConfirmOverwrite);
    if (!filePath.isEmpty()) {
        mRecentPath = QFileInfo(filePath).path();
        ui->leDiff->setText(QDir::toNativeSeparators(filePath));
    }
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbCancel_clicked()
{
    cancelProcess();
    reject();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbOK_clicked()
{
    mWasCanceled = false;
    mLastInput1 = ui->leInput1->text().trimmed();
    mLastInput2 = ui->leInput2->text().trimmed();
    if (mLastInput1.isEmpty() || mLastInput2.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("GDX Diff");
        msgBox.setText("Please specify two GDX files to be compared.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    if (QFileInfo(mLastInput1).isRelative())
        mLastInput1 = QDir::toNativeSeparators(mWorkingDir + QDir::separator() + mLastInput1);

    if (QFileInfo(mLastInput2).isRelative())
        mLastInput2 = QDir::toNativeSeparators(mWorkingDir + QDir::separator() + mLastInput2);

    mLastDiffFile = ui->leDiff->text().trimmed();
    if (mLastDiffFile.isEmpty())
        mLastDiffFile = QDir::toNativeSeparators(mWorkingDir + QDir::separator() + defaultDiffFile);
    else if (QFileInfo(mLastDiffFile).isRelative())
        mLastDiffFile = QDir::toNativeSeparators(mWorkingDir + QDir::separator() + mLastDiffFile);
    if (QFileInfo(mLastDiffFile).suffix().isEmpty())
        mLastDiffFile = mLastDiffFile + ".gdx";

    if (QFileInfo(mLastDiffFile).exists()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Overwrite Existing File");
        msgBox.setText(QFileInfo(mLastDiffFile).fileName() + " already exists.\nDo you want to overwrite it?");
        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msgBox.setIcon(QMessageBox::Warning);
        if (msgBox.exec() == QMessageBox::No)
            return;
    }
    setControlsEnabled(false);

    mProc->setWorkingDir(mWorkingDir);
    mProc->setInput1(mLastInput1);
    mProc->setInput2(mLastInput2);
    mProc->setDiff(mLastDiffFile);
    mProc->setEps(ui->lineEdit_4->text().trimmed());
    mProc->setRelEps(ui->lineEdit_5->text().trimmed());
    mProc->setIgnoreSetText(ui->cbIgnoreSetText->isChecked());
    mProc->setDiffOnly(ui->cbDiffOnly->isChecked());
    mProc->setFieldOnly(ui->cbFieldOnly->isChecked());
    mProc->setFieldToCompare(ui->cbFieldToCompare->itemText(ui->cbFieldToCompare->currentIndex()));

    MainWindow* mainWindow = static_cast<MainWindow*>(parent());
    mDiffFm = mainWindow->fileRepo()->fileMeta(QDir::cleanPath(mLastDiffFile));
    if (mDiffFm && !mDiffFm->editors().isEmpty()) {
        mDiffGdxViewer = ViewHelper::toGdxViewer(mDiffFm->editors().first());
        if (mDiffGdxViewer)
            mDiffGdxViewer->releaseFile();
    }
    mProc->execute();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_cbFieldOnly_toggled(bool checked)
{
    if(checked) {
        // uncheck diff only
        ui->cbDiffOnly->setChecked(false);
        // Deselect "All" in the fields-to-compare combo box and select the second entry(L) instead
        if (ui->cbFieldToCompare->currentIndex() == 0)
            ui->cbFieldToCompare->setCurrentIndex(1);
    }
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_cbDiffOnly_toggled(bool checked)
{
    if(checked)
        // uncheck field only
        ui->cbFieldOnly->setChecked(false);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_cbFieldToCompare_currentIndexChanged(int index)
{
    // uncheck the fild only check box if "All" is selected
    if (index==0)
        ui->cbFieldOnly->setChecked(false);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::setRecentPath(const QString &recentPath)
{
    mRecentPath = recentPath;
    if (QFileInfo(ui->leInput1->text()).isFile())
        mWorkingDir = QFileInfo(ui->leInput1->text()).path();
    else
        mWorkingDir = mRecentPath;
    prepopulateDiff();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::setInput1(QString filePath)
{
    ui->leInput1->setText(QDir::toNativeSeparators(filePath));
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::setInput2(QString filePath)
{
    ui->leInput2->setText(QDir::toNativeSeparators(filePath));
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::reset()
{
    ui->leInput1->clear();
    ui->leInput2->clear();
    ui->lineEdit_4->setText("0.0");
    ui->lineEdit_5->setText("0.0");
    ui->cbDiffOnly->setChecked(false);
    ui->cbFieldOnly->setChecked(false);
    ui->cbIgnoreSetText->setChecked(false);
    ui->cbFieldToCompare->setCurrentIndex(0);
    mPrepopulateDiff = true;
    prepopulateDiff();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbClear_clicked()
{
    reset();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::diffDone()
{
    if (mDiffGdxViewer)
        mDiffGdxViewer->setHasChanged(true);
    if (mDiffFm)
        mDiffFm->reload();
    setControlsEnabled(true);
    if (!mWasCanceled) {
        mWasCanceled = false;
        mLastDiffFile = mProc->diffFile();
        if (mLastDiffFile.isEmpty()) { // give an error pop up that no diff file was created
            QMessageBox msgBox;
            msgBox.setWindowTitle("GDX Diff");
            msgBox.setText("Unable to create diff file. gdxdiff return code: " + QString::number(mProc->exitCode()) + ". See the system output for details.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }
        accept();
    }
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::setControlsEnabled(bool enabled)
{
    ui->pbOK->setEnabled(enabled);
    ui->pbClear->setEnabled(enabled);
    ui->pbInput1->setEnabled(enabled);
    ui->pbInput2->setEnabled(enabled);
    ui->pbDiff->setEnabled(enabled);
    ui->leDiff->setEnabled(enabled);
    ui->leInput1->setEnabled(enabled);
    ui->leInput2->setEnabled(enabled);
    ui->lineEdit_4->setEnabled(enabled);
    ui->lineEdit_5->setEnabled(enabled);
    ui->cbDiffOnly->setEnabled(enabled);
    ui->cbFieldOnly->setEnabled(enabled);
    ui->cbIgnoreSetText->setEnabled(enabled);
    ui->cbFieldToCompare->setEnabled(enabled);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::cancelProcess(int waitMSec)
{
    if (mProc->state() != QProcess::NotRunning) {
        mWasCanceled = true;
        mProc->stop(waitMSec);
    }
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e)
    on_pbCancel_clicked();
}

QString  gams::studio::gdxdiffdialog::GdxDiffDialog::lastInput2() const
{
    return mLastInput2;
}

QString gams::studio::gdxdiffdialog::GdxDiffDialog::input1() const
{
    return ui->leInput1->text();
}

QString gams::studio::gdxdiffdialog::GdxDiffDialog::input2() const
{
    return ui->leInput2->text();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::prepopulateDiff()
{
    if (mPrepopulateDiff) {
        if (QFileInfo(ui->leInput1->text()).isFile()) {
            QString directory = QFileInfo(ui->leInput1->text()).path();
            mWorkingDir = directory;
            ui->leDiff->setText(QDir::toNativeSeparators(directory + QDir::separator() + defaultDiffFile));
        }
        else
            ui->leDiff->setText(QDir::toNativeSeparators(mWorkingDir + QDir::separator() + defaultDiffFile));
    }
}

QString  gams::studio::gdxdiffdialog::GdxDiffDialog::lastInput1() const
{
    return mLastInput1;
}

QString  gams::studio::gdxdiffdialog::GdxDiffDialog::lastDiffFile() const
{
    return mLastDiffFile;
}
