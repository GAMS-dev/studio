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
#include "valuefilterwidget.h"
#include "ui_valuefilterwidget.h"
#include "valuefilter.h"

#include <QKeyEvent>
#include <QMenu>

namespace gams {
namespace studio {
namespace gdxviewer {

ValueFilterWidget::ValueFilterWidget(ValueFilter* valueFilter, QWidget *parent) :
    QWidget(parent), mValueFilter(valueFilter),
    ui(new Ui::ValueFilterWidget)
{
    ui->setupUi(this);

    ui->leMin->setValidator(new QDoubleValidator());
    ui->leMax->setValidator(new QDoubleValidator());

    // we do not have numerical values other than special values and therefore disable the numerical range
    if (mValueFilter->min() == INT_MAX && mValueFilter->max() == INT_MIN) {
        ui->leMin->setEnabled(false);
        ui->leMax->setEnabled(false);
        ui->cbExclude->setEnabled(false);
    } else {
        ui->leMin->setText(numerics::DoubleFormatter::format(mValueFilter->currentMin(), numerics::DoubleFormatter::g, numerics::DoubleFormatter::gFormatFull, true));
        ui->leMax->setText(numerics::DoubleFormatter::format(mValueFilter->currentMax(), numerics::DoubleFormatter::g, numerics::DoubleFormatter::gFormatFull, true));
    }

    ui->cbExclude->setChecked(mValueFilter->exclude());

    ui->cbUndef->setChecked(mValueFilter->showUndef());
    ui->cbNa->setChecked(mValueFilter->showNA());
    ui->cbPInf->setChecked(mValueFilter->showPInf());
    ui->cbMInf->setChecked(mValueFilter->showMInf());
    ui->cbEps->setChecked(mValueFilter->showEps());
    ui->cbAcronym->setChecked(mValueFilter->showAcronym());
}

ValueFilterWidget::~ValueFilterWidget()
{
    delete ui;
}

void ValueFilterWidget::setFocusOnOpen()
{
    ui->leMin->setFocus();
}

void ValueFilterWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void ValueFilterWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void ValueFilterWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        e->accept();
        emit ui->pbApply->clicked();
    }
    QWidget::keyPressEvent(e);
}

void ValueFilterWidget::on_pbApply_clicked()
{
    if (ui->leMin->text().trimmed().isEmpty())
        mValueFilter->setCurrentMin(mValueFilter->min());
    else
        mValueFilter->setCurrentMin(ui->leMin->text().toDouble());
    if (ui->leMax->text().trimmed().isEmpty())
        mValueFilter->setCurrentMax(mValueFilter->max());
    else
        mValueFilter->setCurrentMax(ui->leMax->text().toDouble());
    mValueFilter->setExclude(ui->cbExclude->isChecked());
    mValueFilter->setShowUndef(ui->cbUndef->isChecked());
    mValueFilter->setShowNA(ui->cbNa->isChecked());
    mValueFilter->setShowPInf(ui->cbPInf->isChecked());
    mValueFilter->setShowMInf(ui->cbMInf->isChecked());
    mValueFilter->setShowEps(ui->cbEps->isChecked());
    mValueFilter->setShowAcronym(ui->cbAcronym->isChecked());
    mValueFilter->updateFilter();
    static_cast<QMenu*>(this->parent())->close();
}

void ValueFilterWidget::on_pbReset_clicked()
{
    mValueFilter->reset();
    static_cast<QMenu*>(this->parent())->close();
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
