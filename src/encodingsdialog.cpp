/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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

const QString SelectEncodings::CDefaultEncodingSelection = QString("UTF-8,ISO-8859-1,Shift_JIS,GB2312");

SelectEncodings::SelectEncodings(const QStringList &selectedEncodings, const QString &defaultEncoding, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectEncodings)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    mEncodingsBackup = selectedEncodings;
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
        int i = mEncodingsBackup.indexOf(enc, 0, Qt::CaseInsensitive);
        if (i >= 0) mEncodingsBackup.remove(i);
        rad->setStyleSheet("::indicator {subcontrol-position: center; subcontrol-origin: padding;}");
        rad->setChecked(enc == defaultEncoding);
        ui->tableWidget->setCellWidget(row, 0, rad);

        QCheckBox *box = new QCheckBox("");
        box->setStyleSheet("::indicator {subcontrol-position: center; subcontrol-origin: padding;}");
        box->setChecked(selectedEncodings.contains(enc));
        if (enc == "System") {
            box->setChecked(true);
            box->setEnabled(false);
        }
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

QStringList SelectEncodings::selectedEncodings()
{
    QStringList res;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QCheckBox *box = static_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 1));
        QRadioButton *rb = static_cast<QRadioButton*>(ui->tableWidget->cellWidget(row, 0));
        if (box->isChecked() || rb->isChecked())
            res << ui->tableWidget->item(row, 2)->text();
    }
    // This keeps encodings from a recent run in the list that are currently not available
    // preventing encodings be removed when running without ICU
    res << mEncodingsBackup;

    return res;
}

QString SelectEncodings::defaultEncoding()
{
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QCheckBox *box = static_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 0));
        if (box->isChecked()) return ui->tableWidget->item(row, 2)->data(Qt::EditRole).toString();
    }
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
    mEncodingsBackup = CDefaultEncodingSelection.split(",");
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QString enc = ui->tableWidget->item(row, 2)->text();
        QRadioButton *rad = static_cast<QRadioButton*>(ui->tableWidget->cellWidget(row, 0));
        rad->setChecked(enc.compare("UTF-8", Qt::CaseInsensitive) == 0);
        QCheckBox *box = static_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 1));
        int i = mEncodingsBackup.indexOf(enc, 0, Qt::CaseInsensitive);
        box->setChecked(i >= 0);
        if (i >= 0) mEncodingsBackup.remove(i);
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
        QString enc = ui->tableWidget->item(row, 2)->text();
        if (enc == mDefaultEncoding) rbRow = row;
    }
    QModelIndex mi = ui->tableWidget->model()->index(rbRow, 0);
    ui->tableWidget->setCurrentIndex(mi);
    ui->tableWidget->scrollTo(mi, QTableWidget::PositionAtCenter);
}

}
}
