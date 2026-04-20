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
#include "labelfilterdialog.h"
#include "gdxsymbol.h"

#include <QSet>
#include <QMenu>
#include <QMouseEvent>
#include <QTimer>


namespace gams {
namespace studio {
namespace gdxviewer {

LabelFilterDialog::LabelFilterDialog(GdxSymbol *symbol, int column, LabelFilter *labelFilter, QWidget *parent)
    : QDialog(parent),
      mSymbol(symbol),
      mColumn(column),
      mLabelFilter(labelFilter),
      mModel(new FilterUelModel(symbol, column, this))
{
    setAttribute(Qt::WA_DeleteOnClose);  // dialog should be destroyed when closed
    ui.setupUi(this);

    connect(ui.pbApply, &QPushButton::clicked, this, &LabelFilterDialog::apply);
    connect(ui.pbSelectAll, &QPushButton::clicked, this, &LabelFilterDialog::selectAll);
    connect(ui.pbDeselectAll, &QPushButton::clicked, this, &LabelFilterDialog::deselectAll);
    connect(ui.leSearch, &FilterLineEdit::regExpChanged, this, &LabelFilterDialog::filterLabels);
    connect(ui.pbInvert, &QPushButton::clicked, this, &LabelFilterDialog::invert);
    connect(ui.cbToggleHideUnselected, &QCheckBox::toggled, this, &LabelFilterDialog::toggleHideUnselected);
    connect(mModel, &FilterUelModel::dataChanged, this, &LabelFilterDialog::listDataHasChanged);
    connect(ui.lvLabels, &QuickSelectListView::quickSelect, this, &LabelFilterDialog::apply);

    ui.lvLabels->setModel(mModel);
    setAttribute(Qt::WA_LayoutUsesWidgetRect);

    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
    connect(qApp, &QApplication::focusChanged, this, [this](QWidget* , QWidget* now) {
        if (!now || !this->isAncestorOf(now))
            QTimer::singleShot(0, this, &LabelFilterDialog::close);
    });
}

LabelFilterDialog::~LabelFilterDialog()
{
    delete mModel;
}

void LabelFilterDialog::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void LabelFilterDialog::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void LabelFilterDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        e->accept();
        emit ui.pbApply->clicked();
    }
    else if (e->key() == Qt::Key_Escape)
        this->reject();
    QWidget::keyPressEvent(e);
}

void LabelFilterDialog::focusInEvent(QFocusEvent *event)
{
    QDialog::focusInEvent(event);
    ui.leSearch->setFocus();
}

void LabelFilterDialog::apply()
{
    std::vector<int>* uelsInColumn = mSymbol->uelsInColumn().at(mColumn);
    bool checked;
    bool activateFilter = false;
    for (size_t idx=0; idx<uelsInColumn->size(); idx++) {
        checked = mModel->checked()[idx];
        mLabelFilter->setUelShown(uelsInColumn->at(idx), checked);
        if(!checked)
            activateFilter = true;
    }
    mLabelFilter->setActive(activateFilter);
    mSymbol->filterRows();
    close();
}

void LabelFilterDialog::selectAll()
{
    for(int row=0; row<mModel->rowCount(); row++)
        mModel->setData(mModel->index(row,0), true, Qt::CheckStateRole);
}

void LabelFilterDialog::deselectAll()
{
    for(int row=0; row<mModel->rowCount(); row++)
        mModel->setData(mModel->index(row,0), false, Qt::CheckStateRole);
}

void LabelFilterDialog::invert()
{
    for(int row=0; row<mModel->rowCount(); row++) {
        QModelIndex index = mModel->index(row,0);
        mModel->setData(index, !mModel->data(index, Qt::CheckStateRole).toBool(), Qt::CheckStateRole);
    }
}

void LabelFilterDialog::filterLabels()
{
    QRegularExpression regExp = ui.leSearch->regExp();
    mModel->filterLabels(regExp);
}

void LabelFilterDialog::toggleHideUnselected(bool checked)
{
    if (checked) {
        for(int row=0; row<mModel->rowCount(); row++)
            ui.lvLabels->setRowHidden(row, !mModel->checked()[row]);
    }
    else
        ui.lvLabels->reset();
}

void LabelFilterDialog::listDataHasChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(roles)
    if (ui.cbToggleHideUnselected->isChecked()) {
        for(int row=topLeft.row(); row<=bottomRight.row(); row++)
            ui.lvLabels->setRowHidden(row, !mModel->checked()[row]);
    }
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
