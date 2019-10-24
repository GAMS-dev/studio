#include "gdxdiffdialog.h"
#include "ui_gdxdiffdialog.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>

namespace gams {
namespace studio {
namespace gdxdiffdialog {

GdxDiffDialog::GdxDiffDialog(QString recentPath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GdxDiffDialog),
    mRecentPath(recentPath)
{
    ui->setupUi(this);

    ui->lineEdit_4->setValidator(new QDoubleValidator());
    ui->lineEdit_5->setValidator(new QDoubleValidator());

    QDir::setCurrent(mRecentPath);


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
    GdxDiffProcess* proc = new GdxDiffProcess(this);

    //TODO(CW): do we want to check the input before calling gdxdiff?

    QString input1 = ui->leInput1->text().trimmed();
    QString input2 = ui->leInput2->text().trimmed();
    if (input1.isEmpty() || input2.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("GDX Diff");
        msgBox.setText("Please specify two GDX files to be compared.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    proc->setInput1(input1);
    proc->setInput2(input2);
    proc->setDiff(ui->leDiff->text().trimmed());
    proc->setEps(ui->lineEdit_4->text().trimmed());
    proc->setRelEps(ui->lineEdit_5->text().trimmed());
    proc->setIgnoreSetText(ui->cbIgnoreSetText->isChecked());
    proc->setDiffOnly(ui->cbDiffOnly->isChecked());
    proc->setFieldOnly(ui->cbFieldOnly->isChecked());
    proc->setFieldToCompare(ui->cbFieldToCompare->itemText(ui->cbFieldToCompare->currentIndex()));

    //TODO(CW): close diff GDX file if already open since it will prevent gdxdiff from writing the file
    //MainWindow* mainWindow = static_cast<MainWindow*>(parent());

    proc->setWorkingDir(mRecentPath);
    proc->execute();

    mDiffFile = proc->diffFile();
    if (mDiffFile.isEmpty()) { // give an error pop up that no diff file was created
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

QString gams::studio::gdxdiffdialog::GdxDiffDialog::diffFile()
{
    return mDiffFile;
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
