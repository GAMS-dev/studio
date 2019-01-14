#include "nestedheaderview.h"

#include <QTableView>
#include <QScrollBar>
#include <QDrag>
#include <QMimeData>
#include <QApplication>

namespace gams {
namespace studio {
namespace gdxviewer {

NestedHeaderView::NestedHeaderView(Qt::Orientation orientation, QWidget *parent)
    :QHeaderView(orientation, parent)
{
    setAcceptDrops(true);
}

NestedHeaderView::~NestedHeaderView()
{

}

void NestedHeaderView::init()
{
    calcSectionSize();
    // need to update the first visible sections when scrolling in order to trigger the repaint for showing all labels for the first section
    if (orientation() == Qt::Vertical)
        connect((static_cast<QTableView*>(parent()))->verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() { model()->headerDataChanged(orientation(), 0, 2); });
    else
        connect((static_cast<QTableView*>(parent()))->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this]() { model()->headerDataChanged(orientation(), 0, 2); });
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
    painter->save();
    if (!rect.isValid())
        return;
    QStyleOptionHeader opt;
    initStyleOption(&opt);

    opt.rect = rect;
    opt.section = logicalIndex;

    QStringList labelCurSection = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toStringList();
    QStringList labelPrevSection;

    // first section needs always show all labels
    if (logicalIndex > 0 && sectionViewportPosition(logicalIndex) !=0) {
        int prevIndex = logicalIndex -1;
        while (prevIndex >0 && ((QTableView*)this->parent())->isColumnHidden(prevIndex)) //find the preceding column that is not hidden
            prevIndex--;
        labelPrevSection = model()->headerData(prevIndex, orientation(), Qt::DisplayRole).toStringList();
    }
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
    painter->restore();
}

void NestedHeaderView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        mDragStartPosition = event->pos();
    QHeaderView::mousePressEvent(event);
}

void NestedHeaderView::mouseMoveEvent(QMouseEvent *event)
{
    QHeaderView::mouseMoveEvent(event);
    mMousePos = event->pos();
    if ((event->buttons() & Qt::LeftButton) && (mMousePos - mDragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        if (orientation() == Qt::Vertical)
            mimeData->setData("GDXDRAGDROP/COL", QByteArray::number(pointToDimension(mDragStartPosition)));
        else
            mimeData->setData("GDXDRAGDROP/ROW", QByteArray::number(pointToDimension(mDragStartPosition)));
        drag->setMimeData(mimeData);
        drag->exec();
    }

    if(orientation() == Qt::Vertical)
        headerDataChanged(orientation(),0, qMax(0,model()->rowCount()-1));
    else
        headerDataChanged(orientation(),0, qMax(0,model()->columnCount()-1));
}

void NestedHeaderView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("GDXDRAGDROP/COL") || event->mimeData()->hasFormat("GDXDRAGDROP/ROW"))
        event->acceptProposedAction();
}

void NestedHeaderView::dragMoveEvent(QDragMoveEvent *event)
{
    int dimIdxEnd = pointToDimension(event->pos());
    int orientationEnd = orientation();
    if (orientationEnd == Qt::Horizontal)
        dimIdxEnd += sym()->dim()-sym()->tvColDim();
    if (orientation() == Qt::Horizontal)
        dimIdxEnd = dimIdxEnd - sym()->dim() + dim();
}

void NestedHeaderView::dropEvent(QDropEvent *event)
{
    int dimIdxStart;
    int orientationStart;
    if (event->mimeData()->hasFormat("GDXDRAGDROP/ROW")) {
        dimIdxStart = event->mimeData()->data("GDXDRAGDROP/ROW").toInt() + (sym()->dim()-sym()->tvColDim());
        orientationStart = Qt::Horizontal;
    }
    else {
        dimIdxStart = event->mimeData()->data("GDXDRAGDROP/COL").toInt();
        orientationStart = Qt::Vertical;
    }

    int dimIdxEnd = pointToDimension(event->pos());
    int orientationEnd = orientation();
    if (orientationEnd == Qt::Horizontal)
        dimIdxEnd += sym()->dim()-sym()->tvColDim();

    if (dimIdxStart == dimIdxEnd && orientationStart == orientationEnd) { //nothing happens
        event->accept();
        return;
    }

    int newColDim = sym()->tvColDim();
    if (orientationStart != orientationEnd) {
        if (orientationStart == Qt::Vertical)
            newColDim++;
        else
            newColDim--;
    }
    QVector<int> tvDims = sym()->tvDimOrder();
    tvDims.move(dimIdxStart, dimIdxEnd);

    event->accept();
    sym()->setTableView(true, newColDim, tvDims);
    init();
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

int NestedHeaderView::pointToDimension(QPoint p)
{
    if (orientation() == Qt::Vertical) {
        int totWidth = 0;
        for(int i=0; i<dim(); i++) {
            totWidth += mMaxSectionWidth[i];
            if (p.x() < totWidth)
                return i;
        }
    } else {
        int sectionHeight = QHeaderView::sectionSizeFromContents(0).height();
        int totHeight = 0;
        for(int i=0; i<dim(); i++) {
            totHeight += sectionHeight;
            if (p.y() < totHeight)
                return i;
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
