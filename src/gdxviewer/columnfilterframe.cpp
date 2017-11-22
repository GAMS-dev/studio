#include "columnfilterframe.h"

#include <QSet>
#include <QDebug>
#include <QMenu>

namespace gams {
namespace studio {
namespace gdxviewer {

//TODO(CW): refactor
ColumnFilterFrame::ColumnFilterFrame(GdxSymbol *symbol, int column, QWidget *parent) :
    QFrame(parent), mSymbol(symbol), mColumn(column)
{
    ui.setupUi(this);
    connect(ui.pbApply, &QPushButton::clicked, this, &ColumnFilterFrame::apply);
    QSet<int>* p = symbol->uelsInColumn().at(column);

    for(int i=0; i<p->size(); i++)
    {
        int uel = p->values().at(i);
        QListWidgetItem* item = new QListWidgetItem(QString::number(uel), ui.listWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        if(symbol->filterUels().at(column)->find(uel) != symbol->filterUels().at(column)->end())
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked);
    }
}

void ColumnFilterFrame::apply()
{
    qDebug() << "apply";
    for(int i=0; i<ui.listWidget->count(); i++)
    {
        QListWidgetItem* item = ui.listWidget->item(i);
        if(item->checkState() == Qt::Checked)
        {
            qDebug() << "checked";
            mSymbol->filterUels().at(mColumn)->insert(item->text().toInt());
        }
        else
        {
            qDebug() << "unchecked";
            mSymbol->filterUels().at(mColumn)->remove(item->text().toInt());
        }
    }
    mSymbol->filterRows();
    static_cast<QMenu*>(this->parent())->close();
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
