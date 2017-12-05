#include "columnfilterframe.h"

#include <QSet>
#include <QDebug>
#include <QMenu>

namespace gams {
namespace studio {
namespace gdxviewer {

ColumnFilterFrame::ColumnFilterFrame(GdxSymbol *symbol, int column, QWidget *parent)
    :QFrame(parent), mSymbol(symbol), mColumn(column)
{
    ui.setupUi(this);
    connect(ui.pbApply, &QPushButton::clicked, this, &ColumnFilterFrame::apply);
    connect(ui.pbSelectAll, &QPushButton::clicked, this, &ColumnFilterFrame::selectAll);
    connect(ui.pbDeselectAll, &QPushButton::clicked, this, &ColumnFilterFrame::deselectAll);
    connect(ui.pbFilter, &QPushButton::clicked, this, &ColumnFilterFrame::filterLabels);

    connect(ui.cbToggleHideUnselected, &QCheckBox::toggled, this, &ColumnFilterFrame::toggleHideUnselected);

    mModel = new FilterUelModel(symbol, column, this);
    connect(mModel, &FilterUelModel::dataChanged, this, &ColumnFilterFrame::listDataHasChanged);

    ui.lvLabels->setModel(mModel);
}

ColumnFilterFrame::~ColumnFilterFrame()
{
    delete mModel;
}

void ColumnFilterFrame::apply()
{
    bool* showUelInColumn =  mSymbol->showUelInColumn().at(mColumn);
    QVector<int>* uelsInColumn = mSymbol->uelsInColumn().at(mColumn);
    bool checked;
    for (int idx=0; idx<uelsInColumn->size(); idx++)
    {
        checked = mModel->checked()[idx];
        showUelInColumn[uelsInColumn->at(idx)] = checked;
    }
    mSymbol->filterRows();
    static_cast<QMenu*>(this->parent())->close();
}

void ColumnFilterFrame::selectAll()
{
    for(int row=0; row<mModel->rowCount(); row++)
        mModel->setData(mModel->index(row,0), true, Qt::CheckStateRole); //TODO: do not call setData multipe times but one function for setAll
}

void ColumnFilterFrame::deselectAll()
{
    for(int row=0; row<mModel->rowCount(); row++)
        mModel->setData(mModel->index(row,0), false, Qt::CheckStateRole); //TODO: do not call setData multipe times but one function for setAll
}

void ColumnFilterFrame::filterLabels()
{
    QString filterString = ui.leSearch->text();
    mModel->filterLabels(filterString);
}

void ColumnFilterFrame::toggleHideUnselected(bool checked)
{
    if (checked)
    {
        for(int row=0; row<mModel->rowCount(); row++)
        {
            if(mModel->checked()[row])
                ui.lvLabels->setRowHidden(row, false);
            else
                ui.lvLabels->setRowHidden(row, true);
        }
    }
    else
        ui.lvLabels->reset();
}

void ColumnFilterFrame::listDataHasChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (ui.cbToggleHideUnselected->isChecked())
    {
        for(int row=topLeft.row(); row<=bottomRight.row(); row++)
        {
            if(mModel->checked()[row])
                ui.lvLabels->setRowHidden(row, false);
            else
                ui.lvLabels->setRowHidden(row, true);
        }
    }
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
