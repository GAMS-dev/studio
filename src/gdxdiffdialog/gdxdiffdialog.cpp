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
    QDir::setCurrent(mRecentPath);
    qDebug() << "recent:" << mRecentPath;
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
    this->close();
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pbOK_clicked()
{
    GdxDiffProcess* proc = new GdxDiffProcess(this);

    //TODO(CW): do we want to check the input before calling gdxdiff?

    proc->setInput1(ui->leInput1->text().trimmed());
    proc->setInput2(ui->leInput2->text().trimmed());
    proc->setDiff(ui->leDiff->text().trimmed());
    proc->setEps(ui->lineEdit_4->text().trimmed());
    proc->setRelEps(ui->lineEdit_5->text().trimmed());
    proc->setIgnoreSetText(ui->cbIgnoreSetText->isChecked());
    proc->setDiffOnly(ui->cbDiffOnly->isChecked());
    proc->setFieldOnly(ui->cbFieldOnly->isChecked());
    proc->setFieldToCompare(ui->cbFieldOnly->text().trimmed());

    proc->setWorkingDir(mRecentPath);
    proc->execute();

    mDiffFile = proc->diffFile();
    if (mDiffFile.isEmpty()) { // give an error pop up that no diff file was created
        //TODO(CW): in case of error add extra error text to system output
        QMessageBox msgBox;
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
