/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "headerviewproxy.h"

#include <QCheckBox>
#include <QRadioButton>

namespace gams {
namespace studio {

SelectEncodings::SelectEncodings(const QStringList &selectedEncodings, const QString &defaultEncoding, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectEncodings)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    mSelectedEncodings = selectedEncodings;
    mDefaultEncoding = defaultEncoding;

    QStringList encList = QStringConverter::availableCodecs();
    std::sort(encList.begin(), encList.end());
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->tableWidget->horizontalHeader()->setStyle(HeaderViewProxy::instance());
    ui->tableWidget->setRowCount(encList.count());
    ui->tableWidget->setWordWrap(false);

    int row = 0;
    QFont boldFont = font();
    boldFont.setBold(true);
    for (const QString &enc: std::as_const(encList)) {
        QRadioButton *rad = new QRadioButton("");
        rad->setStyleSheet("::indicator {subcontrol-position: center; subcontrol-origin: padding;}");
        rad->setChecked(enc == defaultEncoding);
        ui->tableWidget->setCellWidget(row, 0, rad);

        QCheckBox *box = new QCheckBox("");
        box->setStyleSheet("::indicator {subcontrol-position: center; subcontrol-origin: padding;}");
        if (selectedEncodings.contains(enc) || enc.compare("System") == 0) box->setChecked(true);
        if (enc == "System") box->setEnabled(false);
        ui->tableWidget->setCellWidget(row, 1, box);

        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(enc));
        ui->tableWidget->item(row, 2)->setTextAlignment(Qt::AlignRight);
        ui->tableWidget->item(row, 2)->setData(Qt::EditRole, enc);

        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(" " + enc + " "));
        ui->tableWidget->item(row, 3)->setFont(boldFont);

        ui->tableWidget->setVerticalHeaderItem(row, new QTableWidgetItem());
        ui->tableWidget->setRowHeight(row, int(ui->tableWidget->fontMetrics().height()*1.4));
        row++;
    }
    ui->tableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableWidget->hideColumn(2);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

SelectEncodings::~SelectEncodings()
{
    delete ui;
}

QList<int> SelectEncodings::selectedMibs()
{
    QList<int> res;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QCheckBox *box = static_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 1));
        QRadioButton *rb = static_cast<QRadioButton*>(ui->tableWidget->cellWidget(row, 0));
        if (box->isChecked() || rb->isChecked()) res << ui->tableWidget->item(row, 2)->data(Qt::EditRole).toInt();
    }
    // ensure to have UTF-8 on top and System at the 2nd place
    if (res.contains(0)) res.move(res.indexOf(0), 0);
    if (res.contains(106)) res.move(res.indexOf(106), 0);
    return res;
}

QStringList SelectEncodings::selectedEncodings()
{
    QStringList res;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QCheckBox *box = static_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 1));
        QRadioButton *rb = static_cast<QRadioButton*>(ui->tableWidget->cellWidget(row, 0));
        if (box->isChecked() || rb->isChecked()) res << ui->tableWidget->item(row, 2)->text();
    }
    // TODO(JM) check if necessary
    // ensure to have UTF-8 on top and System at the 2nd place
    // if (res.contains(0)) res.move(res.indexOf(0), 0);
    // if (res.contains(106)) res.move(res.indexOf(106), 0);

    return res;
}

int SelectEncodings::defaultCodec()
{
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QCheckBox *box = static_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 0));
        if (box->isChecked()) return ui->tableWidget->item(row, 2)->data(Qt::EditRole).toInt();
    }
    return 104;
}

QString SelectEncodings::defaultEncoding()
{
    return mDefaultEncoding;
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
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        int mib = ui->tableWidget->item(row, 2)->data(Qt::EditRole).toInt();
        QRadioButton *rad = static_cast<QRadioButton*>(ui->tableWidget->cellWidget(row, 0));
        rad->setChecked(mib == mDefaultMib);
        QCheckBox *box = static_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 1));
        box->setChecked(mib == 0 || mSelectedMibs.contains(mib));
    }
    centerCurrent();
}

void SelectEncodings::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    centerCurrent();
}

void SelectEncodings::centerCurrent()
{
    int rbRow = 0;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        int mib = ui->tableWidget->item(row, 2)->data(Qt::EditRole).toInt();
        if (mib == mDefaultMib) rbRow = row;
    }
    QModelIndex mi = ui->tableWidget->model()->index(rbRow, 0);
    ui->tableWidget->setCurrentIndex(mi);
    ui->tableWidget->scrollTo(mi, QTableWidget::PositionAtCenter);
}

}
}
