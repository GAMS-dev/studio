#include "nestedheaderview.h"

#include <QTableView>

namespace gams {
namespace studio {
namespace gdxviewer {

NestedHeaderView::NestedHeaderView(Qt::Orientation orientation, QWidget *parent)
    :QHeaderView(orientation, parent)
{

}

NestedHeaderView::~NestedHeaderView()
{

}

void NestedHeaderView::init()
{
    calcSectionSize();
}

int NestedHeaderView::dim() const
{
    if (orientation() == Qt::Vertical)
        return sym()->dim() - sym()->tvColDim();
    else {
        int d = sym()->tvColDim();
        if (sym()->type() == GMS_DT_VAR || sym()->type() == GMS_DT_EQU)
            return d+1;
        return d;
    }
}

void NestedHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    if (!rect.isValid())
        return;
    QStyleOptionHeader opt;
    initStyleOption(&opt);

    opt.rect = rect;
    opt.section = logicalIndex;

    QStringList labelCurSection = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toStringList();
    QStringList labelPrevSection;

    if (logicalIndex > 0)
        labelPrevSection = model()->headerData(logicalIndex-1, orientation(), Qt::DisplayRole).toStringList();
    else {
        for(int i=0; i<dim(); i++)
            labelPrevSection << "";
    }

    QPointF oldBO = painter->brushOrigin();

    int lastRowWidth = 0;
    int lastHeight = 0;

    if(orientation() == Qt::Vertical) {
        for(int i=0; i<dim(); i++) {
            QStyle::State state = QStyle::State_None;
            if (isEnabled())
                state |= QStyle::State_Enabled;
            if (window()->isActiveWindow())
                state |= QStyle::State_Active;
            //int rowWidth = getSectionSize(logicalIndex, i).width();
            int rowWidth = mMaxSectionWidth[i];

            if (labelPrevSection[i] != labelCurSection[i])
                opt.text = labelCurSection[i];
            else
                opt.text = "";
            opt.rect.setLeft(opt.rect.left()+ lastRowWidth);
            lastRowWidth = rowWidth;
            opt.rect.setWidth(rowWidth);

            if (opt.rect.contains(mMousePos))
                state |= QStyle::State_MouseOver;
            opt.state = state;
            style()->drawControl(QStyle::CE_Header, &opt, painter, this);
        }
    } else {
        for(int i=0; i<dim(); i++) {
            QStyle::State state = QStyle::State_None;
            if (isEnabled())
                state |= QStyle::State_Enabled;
            if (window()->isActiveWindow())
                state |= QStyle::State_Active;
            //int rowWidth = getSectionSize(logicalIndex, i).width();
            //int rowWidth = mMaxSectionWidth[i];

            if (labelPrevSection[i] != labelCurSection[i])
                opt.text = labelCurSection[i];
            else
                opt.text = "";
            opt.rect.setTop(opt.rect.top()+ lastHeight);
            lastHeight = QHeaderView::sectionSizeFromContents(logicalIndex).height();

            opt.rect.setHeight(QHeaderView::sectionSizeFromContents(logicalIndex).height());
            if (opt.rect.contains(mMousePos))
                state |= QStyle::State_MouseOver;
            opt.state = state;
            style()->drawControl(QStyle::CE_Header, &opt, painter, this);
        }
    }
    painter->setBrushOrigin(oldBO);
}

void NestedHeaderView::mouseMoveEvent(QMouseEvent *event)
{
    QHeaderView::mouseMoveEvent(event);
    mMousePos = event->pos();
    if(orientation() == Qt::Vertical)
        headerDataChanged(orientation(),0, qMax(0,model()->rowCount()-1));
    else
        headerDataChanged(orientation(),0, qMax(0,model()->columnCount()-1));
}

void NestedHeaderView::leaveEvent(QEvent *event)
{
    QHeaderView::leaveEvent(event);
    mMousePos = QPoint(-1,-1);
}

void NestedHeaderView::calcSectionSize()
{
    mMaxSectionWidth.clear();
    for(int i=0; i<dim(); i++)
        mMaxSectionWidth << 0;
    QStyleOptionHeader opt;
    initStyleOption(&opt);

    QVector<QSet<QString>> labelAlreadySeen(dim());


    int indexCount;
    if (orientation() == Qt::Vertical)
        indexCount = model()->rowCount();
    else
        indexCount = model()->columnCount();

    for(int logicalIndex=0; logicalIndex<indexCount; logicalIndex++) {
        opt.section = logicalIndex;
        QVariant var = model()->headerData(logicalIndex, orientation(), Qt::FontRole);
        QFont fnt;
        if (var.isValid() && var.canConvert<QFont>())
            fnt = qvariant_cast<QFont>(var);
        else
            fnt = font();
        fnt.setBold(true);
        opt.fontMetrics = QFontMetrics(fnt);
        QStringList text = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toStringList();
        for(int d=0; d<dim(); d++) {
            QString label = text[d];
            if (!labelAlreadySeen[d].contains(label)) {
                labelAlreadySeen[d].insert(label);
                opt.text = label;
                //TODO(CW): This function is terribly slow. We avoid calling it multiple times for the same label in a dimension
                QSize s = style()->sizeFromContents(QStyle::CT_HeaderSection, &opt, QSize(), this);
                mMaxSectionWidth[d] = qMax(s.width(), mMaxSectionWidth[d]);
            }
        }
    }
}

GdxSymbol *NestedHeaderView::sym() const
{
    return static_cast<GdxSymbol*>(model());
}

QSize NestedHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    if (orientation() == Qt::Vertical) {
        QSize s(0,sectionSize(logicalIndex));
        for (int i=0; i<dim(); i++)
            s.setWidth(mMaxSectionWidth[i] + s.width());
        return s;
    } else {
        QSize s = QHeaderView::sectionSizeFromContents(logicalIndex);
        s.setHeight(s.height()*dim());
        return s;
    }
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
