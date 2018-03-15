#include "selectencodings.h"
#include "ui_selectencodings.h"
#include <QtWidgets>


SelectEncodings::SelectEncodings(QList<int> selectedMibs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectEncodings)
{
    ui->setupUi(this);
    QList<int> mibs = QTextCodec::availableMibs();
    qSort(mibs);
    ui->tableWidget->setRowCount(mibs.count());

    int row = 0;
    QFont boldFont = font();
    boldFont.setBold(true);
    foreach (int mib, mibs) {
        QCheckBox *box = new QCheckBox("");
        box->setStyleSheet("::indicator {subcontrol-position: center; subcontrol-origin: padding;}");
        if (selectedMibs.contains(mib) || mib == 0) box->setChecked(true);
        if (mib == 0) box->setEnabled(false);
        ui->tableWidget->setCellWidget(row, 0, box);

        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(mib));
        ui->tableWidget->item(row, 1)->setTextAlignment(Qt::AlignRight);
        ui->tableWidget->item(row, 1)->setData(Qt::EditRole, mib);

        QTextCodec *codec = QTextCodec::codecForMib(mib);
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(" "+QString(codec->name())+" "));
        ui->tableWidget->item(row, 2)->setFont(boldFont);

        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(" "+QString(codec->aliases().join(", "))+" "));
        ui->tableWidget->setVerticalHeaderItem(row, new QTableWidgetItem());
        ui->tableWidget->setRowHeight(row, ui->tableWidget->fontMetrics().height()*1.3);
        row++;
    }
    ui->tableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

SelectEncodings::~SelectEncodings()
{
    delete ui;
}

QList<int> SelectEncodings::selectedMibs()
{
    QList<int> res;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QCheckBox *box = static_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 0));
        if (box->isChecked()) res << ui->tableWidget->item(row, 1)->data(Qt::EditRole).toInt();
    }
    // ensure to have UTF-8 on top and System at the 2nd place
    if (res.contains(0)) res.move(res.indexOf(0), 0);
    if (res.contains(106)) res.move(res.indexOf(106), 0);
    return res;
}


void SelectEncodings::on_pbCancel_clicked()
{
    reject();
}

void SelectEncodings::on_pbSave_clicked()
{
    accept();
}

void SelectEncodings::on_pbReset_clicked()
{
    QList<int> defaultEncodings {106, 0, 4, 17, 2025};
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QCheckBox *box = static_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 0));
        int mib = ui->tableWidget->item(row, 1)->data(Qt::EditRole).toInt();
        box->setChecked(mib == 0 || defaultEncodings.contains(mib));
    }
}
