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

    mModel = new FilterUelModel(symbol, column, this);
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
        mModel->setData(mModel->index(row,0), true, Qt::CheckStateRole);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
