/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "encodingsdialog.h"
#include "ui_encodingsdialog.h"

#include <QCheckBox>
#include <QTextCodec>

namespace gams {
namespace studio {

SelectEncodings::SelectEncodings(QList<int> selectedMibs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectEncodings)
{
    ui->setupUi(this);
    QList<int> mibs = QTextCodec::availableMibs();
    std::sort(mibs.begin(), mibs.end());
    ui->tableWidget->setRowCount(mibs.count());
    ui->tableWidget->setWordWrap(false);

    int row = 0;
    QFont boldFont = font();
    boldFont.setBold(true);
    for (int mib: mibs) {
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
        ui->tableWidget->setRowHeight(row, int(ui->tableWidget->fontMetrics().height()*1.4));
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

}
}
