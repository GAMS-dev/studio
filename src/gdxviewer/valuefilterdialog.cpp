/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "valuefilterdialog.h"
#include "ui_valuefilterdialog.h"

#include <QKeyEvent>
#include <QMenu>
#include <QTimer>

namespace gams {
namespace studio {
namespace gdxviewer {

ValueFilterDialog::ValueFilterDialog(GdxSymbol *symbol, int valueColumn, ValueFilter& valueFilter, QWidget *parent) :
    QDialog(parent), ui(new Ui::ValueFilterDialog), mSymbol(symbol),
    mValueColumn(valueColumn), mValueFilter(valueFilter)
{
    setAttribute(Qt::WA_DeleteOnClose);  // dialog should be destroyed when closed
    ui->setupUi(this);

    ui->leMin->setValidator(new QDoubleValidator());
    ui->leMax->setValidator(new QDoubleValidator());

    const double symbolMin = mSymbol->minDouble(valueColumn);
    const double symbolMax = mSymbol->maxDouble(valueColumn);
    const bool hasRange = !(symbolMin == INT_MAX && symbolMax == INT_MIN);

    ui->leMin->setEnabled(hasRange);
    ui->leMax->setEnabled(hasRange);
    ui->cbExclude->setEnabled(hasRange);

    auto formatVal = [](double val) {
        return numerics::DoubleFormatter::format(
            val,
            numerics::DoubleFormatter::g,
            numerics::DoubleFormatter::gFormatFull,
            true
            );
    };

    if (hasRange) {
        const double minToShow = mValueFilter.active ? mValueFilter.min : symbolMin;
        const double maxToShow = mValueFilter.active ? mValueFilter.max : symbolMax;

        ui->leMin->setText(formatVal(minToShow));
        ui->leMax->setText(formatVal(maxToShow));
    }

    if (mValueFilter.active) {
        ui->cbExclude->setChecked(mValueFilter.exclude);
        ui->cbUndef->setChecked(mValueFilter.showUndef);
        ui->cbNa->setChecked(mValueFilter.showNA);
        ui->cbPInf->setChecked(mValueFilter.showPInf);
        ui->cbMInf->setChecked(mValueFilter.showMInf);
        ui->cbEps->setChecked(mValueFilter.showEps);
        ui->cbAcronym->setChecked(mValueFilter.showAcronym);
    }

    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
    connect(qApp, &QApplication::focusChanged, this, [this](QWidget* , QWidget* now) {
        if (!now || !this->isAncestorOf(now))
            QTimer::singleShot(0, this, &ValueFilterDialog::close);
    });

}

ValueFilterDialog::~ValueFilterDialog()
{
    delete ui;
}

void ValueFilterDialog::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void ValueFilterDialog::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void ValueFilterDialog::focusInEvent(QFocusEvent *event)
{
    QDialog::focusInEvent(event);
    ui->leMin->setFocus();
}

void ValueFilterDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        e->accept();
        emit ui->pbApply->clicked();
    }
    else if (e->key() == Qt::Key_Escape)
        this->reject();
    QWidget::keyPressEvent(e);
}

void ValueFilterDialog::on_pbApply_clicked()
{
    if (ui->leMin->text().trimmed().isEmpty())
        mValueFilter.min = mSymbol->minDouble(mValueColumn);
    else
        mValueFilter.min = ui->leMin->text().toDouble();
    if (ui->leMax->text().trimmed().isEmpty())
        mValueFilter.max = mSymbol->maxDouble(mValueColumn);
    else
        mValueFilter.max = ui->leMax->text().toDouble();
    mValueFilter.exclude = ui->cbExclude->isChecked();
    mValueFilter.showUndef = ui->cbUndef->isChecked();
    mValueFilter.showNA = ui->cbNa->isChecked();
    mValueFilter.showPInf = ui->cbPInf->isChecked();
    mValueFilter.showMInf = ui->cbMInf->isChecked();
    mValueFilter.showEps = ui->cbEps->isChecked();
    mValueFilter.showAcronym = ui->cbAcronym->isChecked();

    if (mValueFilter.min == mSymbol->minDouble(mValueColumn) &&
        mValueFilter.max == mSymbol->maxDouble(mValueColumn) &&
        !mValueFilter.exclude &&
        mValueFilter.showUndef &&
        mValueFilter.showNA &&
        mValueFilter.showPInf &&
        mValueFilter.showMInf &&
        mValueFilter.showEps &&
        mValueFilter.showAcronym)
        mValueFilter.active = false;
    else
        mValueFilter.active = true;
    mSymbol->filterRows();
    close();
}

void ValueFilterDialog::on_pbReset_clicked()
{
    //mValueFilter->reset();
    mValueFilter.active = false;
    // TODO: reset the filter values as well
    mSymbol->filterRows();
    close();
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
