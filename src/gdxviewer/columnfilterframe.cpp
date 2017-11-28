#include "columnfilterframe.h"

#include <QSet>
#include <QDebug>
#include <QMenu>

namespace gams {
namespace studio {
namespace gdxviewer {

//TODO(CW): refactor
ColumnFilterFrame::ColumnFilterFrame(GdxSymbol *symbol, int column, QWidget *parent)
    :QFrame(parent), mSymbol(symbol), mColumn(column)
{
    ui.setupUi(this);
    connect(ui.pbApply, &QPushButton::clicked, this, &ColumnFilterFrame::apply);
    connect(ui.pbSelectAll, &QPushButton::clicked, this, &ColumnFilterFrame::selectAll);

    mModel = new FilterUelModel(symbol, column);
    ui.lvLabels->setModel(mModel);
}

void ColumnFilterFrame::apply()
{
    qDebug() << "apply";

    for(int i=0; i<mModel->changed().count(); i++)
        mSymbol->filterUels().at(mColumn)->insert(mModel->changed().keys().at(i), mModel->changed().values().at(i));
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
