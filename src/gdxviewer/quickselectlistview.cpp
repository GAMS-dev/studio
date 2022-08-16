#include "columnfilterframe.h"
#include "quickselectlistview.h"
#include <QMouseEvent>

namespace gams {
namespace studio {
namespace gdxviewer {

QuickSelectListView::QuickSelectListView(QWidget *parent) :
    QListView(parent)
{

}

void QuickSelectListView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier)) {
        QModelIndex idx = this->indexAt(event->pos());
        if (idx.isValid()) {
            for(int row=0; row<model()->rowCount(); row++)
                model()->setData(model()->index(row,0), false, Qt::CheckStateRole);
            this->model()->setData(idx, true, Qt::CheckStateRole);
            emit quickSelect();
        }
        event->accept();
    }
    else
        QListView::mouseReleaseEvent(event);
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
