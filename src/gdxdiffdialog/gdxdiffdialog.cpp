#include "gdxdiffdialog.h"
#include "ui_gdxdiffdialog.h"
#include <QFileDialog>
#include <QDebug>

namespace gams {
namespace studio {
namespace gdxdiffdialog {

GdxDiffDialog::GdxDiffDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GdxDiffDialog)
{
    ui->setupUi(this);
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
                                                            "",
                                                            tr("GDX file (*.gdx);;"
                                                               "All files (*.*)"));
    if (!filePath.isEmpty())
        ui->lineEdit->setText(filePath);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pushButton_2_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose GDX File...",
                                                            "",
                                                            tr("GDX file (*.gdx);;"
                                                               "All files (*.*)"));
    if (!filePath.isEmpty())
        ui->lineEdit_2->setText(filePath);
}

void gams::studio::gdxdiffdialog::GdxDiffDialog::on_pushButton_3_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose GDX File...",
                                                            "",
                                                            tr("GDX file (*.gdx);;"
                                                               "All files (*.*)"));
    if (!filePath.isEmpty())
        ui->lineEdit_3->setText(filePath);
}
