#include "gdxdiffdialog.h"
#include "ui_gdxdiffdialog.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <editors/viewhelper.h>
#include <gdxviewer/gdxviewer.h>

namespace gams {
namespace studio {
namespace gdxdiffdialog {

GdxDiffDialog::GdxDiffDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GdxDiffDialog)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->lineEdit_4->setValidator(new QDoubleValidator());
    ui->lineEdit_5->setValidator(new QDoubleValidator());
}

GdxDiffDialog::~GdxDiffDialog()
{
    delete ui;
}

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pushButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose GDX File...",
                                                            nullptr,
                                                            tr("GDX file (*.gdx);;"
                                                               "All files (*.*)"));
    if (!filePath.isEmpty())
        ui->leInput1->setText(filePath);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pushButton_2_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose GDX File...",
                                                            nullptr,
                                                            tr("GDX file (*.gdx);;"
                                                               "All files (*.*)"));
    if (!filePath.isEmpty())
        ui->leInput2->setText(filePath);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pushButton_3_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose GDX File...",
                                                            nullptr,
                                                            tr("GDX file (*.gdx);;"
                                                               "All files (*.*)"));
    if (!filePath.isEmpty())
        ui->leDiff->setText(filePath);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbCancel_clicked()
{
    reject();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbOK_clicked()
{
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
        mLastInput1 = QDir::cleanPath(mRecentPath + QDir::separator() + mLastInput1);

    if (QFileInfo(mLastInput2).isRelative())
        mLastInput2 = QDir::cleanPath(mRecentPath + QDir::separator() + mLastInput2);

    mLastDiffFile = ui->leDiff->text().trimmed();
    if (mLastDiffFile.isEmpty())
        mLastDiffFile = QDir::cleanPath(mRecentPath + QDir::separator() + "diffile.gdx");
    else if (QFileInfo(mLastDiffFile).isRelative())
        mLastDiffFile = QDir::cleanPath(mRecentPath + QDir::separator() + mLastDiffFile);
    if (QFileInfo(mLastDiffFile).suffix().isEmpty())
        mLastDiffFile = mLastDiffFile + ".gdx";

    GdxDiffProcess* proc = new GdxDiffProcess(this);
    proc->setWorkingDir(mRecentPath);
    proc->setInput1(mLastInput1);
    proc->setInput2(mLastInput2);
    proc->setDiff(mLastDiffFile);
    proc->setEps(ui->lineEdit_4->text().trimmed());
    proc->setRelEps(ui->lineEdit_5->text().trimmed());
    proc->setIgnoreSetText(ui->cbIgnoreSetText->isChecked());
    proc->setDiffOnly(ui->cbDiffOnly->isChecked());
    proc->setFieldOnly(ui->cbFieldOnly->isChecked());
    proc->setFieldToCompare(ui->cbFieldToCompare->itemText(ui->cbFieldToCompare->currentIndex()));

    MainWindow* mainWindow = static_cast<MainWindow*>(parent());
    FileMeta* fm = mainWindow->fileRepo()->fileMeta(mLastDiffFile);
    if (fm && !fm->editors().isEmpty()) {
        gdxviewer::GdxViewer* gdxViewer = ViewHelper::toGdxViewer(fm->editors().first());
        gdxViewer->releaseFile();
        proc->execute();
        gdxViewer->setHasChanged(true);
        fm->reload();
    } else
        proc->execute();

    mLastDiffFile = proc->diffFile();
    if (mLastDiffFile.isEmpty()) { // give an error pop up that no diff file was created
        //TODO(CW): in case of error add extra error text to system output
        QMessageBox msgBox;
        msgBox.setWindowTitle("GDX Diff");
        msgBox.setText("Unable to create diff file. gdxdiff return code: " + QString::number(proc->exitCode()) + ". See the system output for details.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
    delete proc;
    accept();
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
    QDir::setCurrent(mRecentPath);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::setInput1(QString filePath)
{
    ui->leInput1->setText(filePath);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::setInput2(QString filePath)
{
    ui->leInput2->setText(filePath);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::clear()
{
    ui->leInput1->clear();
    ui->leInput2->clear();
    ui->leDiff->clear();
    ui->lineEdit_4->setText("0.0");
    ui->lineEdit_5->setText("0.0");
    ui->cbDiffOnly->setChecked(false);
    ui->cbFieldOnly->setChecked(false);
    ui->cbIgnoreSetText->setChecked(false);
    ui->cbFieldToCompare->setCurrentIndex(0);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbClear_clicked()
{
    clear();
}

QString  gams::studio::gdxdiffdialog::GdxDiffDialog::lastInput2() const
{
    return mLastInput2;
}

QString  gams::studio::gdxdiffdialog::GdxDiffDialog::lastInput1() const
{
    return mLastInput1;
}

QString  gams::studio::gdxdiffdialog::GdxDiffDialog::lastDiffFile() const
{
    return mLastDiffFile;
}
