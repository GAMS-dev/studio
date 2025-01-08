/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "gdxsymbolview.h"
#include "nestedheaderview.h"

#include <QTableView>
#include <QScrollBar>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QMap>
#include <QTimer>
#include "theme.h"

namespace gams {
namespace studio {
namespace gdxviewer {

NestedHeaderView::NestedHeaderView(Qt::Orientation orientation, GdxSymbolView* symbolView, QWidget *parent)
    :QHeaderView(orientation, parent), mSymbolView(symbolView)
{
    setAcceptDrops(true);
    connect(this, &QHeaderView::sectionResized, this, [this]() { ddEnabled = false; });
}

NestedHeaderView::~NestedHeaderView()
{

}

void NestedHeaderView::setModel(QAbstractItemModel *model)
{
    QHeaderView::setModel(model);
    bindScrollMechanism();
}

void NestedHeaderView::reset()
{
    if (this->model()) {
        updateSectionWidths();
    }
    QHeaderView::reset();
}

int NestedHeaderView::dim() const
{
    if (orientation() == Qt::Vertical && sym()->needDummyRow())
        return 1;
    else if (orientation() == Qt::Horizontal && sym()->needDummyColumn())
        return 1;
    else if (orientation() == Qt::Vertical)
        return sym()->dim() - sym()->tvColDim();
    else {
        int d = sym()->tvColDim();
        if (sym()->type() == GMS_DT_VAR || sym()->type() == GMS_DT_EQU)
            return d+1;
        return d;
    }
}

void NestedHeaderView::setDdEnabled(bool value)
{
    ddEnabled = value;
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
        if (orientation() == Qt::Horizontal) { //find the preceeding column that is not hidden
            while (prevIndex >0 && ((QTableView*)this->parent())->isColumnHidden(prevIndex))
                prevIndex--;
        }
        labelPrevSection = model()->headerData(prevIndex, orientation(), Qt::DisplayRole).toStringList();
    }
    else {
        for(int i=0; i<dim(); i++)
            labelPrevSection << "";
    }
    QPointF oldBO = painter->brushOrigin();

    int lastRowWidth = 0;
    int lastHeight = 0;

    int fontVerticalOffset = opt.rect.top() + opt.rect.height() - fontMetrics().descent()
            - (opt.rect.height()-fontMetrics().height()) / 2;

    if(orientation() == Qt::Vertical) {
        opt.text = "";
        for(int i=0; i<dim(); i++) {
            QStyle::State state = QStyle::State_None;
            if (isEnabled())
                state |= QStyle::State_Enabled;
            if (window()->isActiveWindow())
                state |= QStyle::State_Active;
            int rowWidth = sectionWidth.at(i);

            QString text;
            QString textSearch;
            if (labelPrevSection[i] != labelCurSection[i]) {
                text = " "+labelCurSection[i];
                textSearch = labelCurSection[i];
            }
            opt.rect.setLeft(opt.rect.left()+ lastRowWidth);
            lastRowWidth = rowWidth;
            opt.rect.setWidth(rowWidth);

            if (opt.rect.contains(mMousePos))
                state |= QStyle::State_MouseOver;
            opt.state = state;
            opt.textAlignment = Qt::AlignLeft | Qt::AlignVCenter;
            style()->drawControl(QStyle::CE_Header, &opt, painter, this);
            painter->restore();
            QPen pen(painter->pen());
            pen.setColor(palette().text().color());
            if (!sym()->needDummyRow()) {  // do not highlight search matches in dummy rows
                if (!((TableViewModel *)model())->sym()->searchRegEx().pattern().isEmpty() && ((TableViewModel *)model())->sym()->searchRegEx().match(textSearch).hasMatch()) {
                    pen.setColor(QColor(Qt::white));
                    QPen pen2 = pen;
                    pen2.setStyle(Qt::NoPen);
                    painter->setPen(pen2);

                    QBrush bgBrush = painter->background();
                    bgBrush.setColor(toColor(Theme::Edit_matchesBg));
                    painter->setBrush(bgBrush);

                    QRect rect2 = opt.rect;
                    rect2.setWidth(rect2.width()-1);
                    rect2.setHeight(rect2.height()-1);

                    painter->drawRect(rect2);
                }
            }

            painter->setPen(pen);
            painter->drawText(opt.rect.left(), fontVerticalOffset, text);
            if (dimIdxEnd>-1) {
                if (dimIdxEnd == i)
                    painter->drawLine(opt.rect.left(), opt.rect.top(), opt.rect.left(), opt.rect.bottom());
                else if (dimIdxEnd-1 == i && dimIdxEnd == dim())
                    painter->drawLine(opt.rect.right(), opt.rect.top(), opt.rect.right(), opt.rect.bottom());
            }
            painter->save();
        }
    } else {
        for(int i=0; i<dim(); i++) {
            QStyle::State state = QStyle::State_None;
            if (isEnabled())
                state |= QStyle::State_Enabled;
            if (window()->isActiveWindow())
                state |= QStyle::State_Active;

            QString text;
            QString textSearch;
            if (labelPrevSection[i] != labelCurSection[i]) {
                text = " " + labelCurSection[i];
                textSearch = labelCurSection[i];
            }
            else
                text = "";
            opt.text = "";
            opt.rect.setTop(opt.rect.top()+ lastHeight);
            lastHeight = QHeaderView::sectionSizeFromContents(logicalIndex).height();

            opt.rect.setHeight(QHeaderView::sectionSizeFromContents(logicalIndex).height());
            if (opt.rect.contains(mMousePos))
                state |= QStyle::State_MouseOver;
            opt.state = state;

            style()->drawControl(QStyle::CE_Header, &opt, painter, this);

            painter->restore();
            QPen pen(painter->pen());
            pen.setColor(palette().text().color());

            // do not highlight search matches in dummy columns or var/equ attributes (e.g. level, marginal, ...)
            if ((!sym()->needDummyColumn() && sym()->type() != GMS_DT_VAR && sym()->type() != GMS_DT_EQU) || i<dim()-1) {
                if (!((TableViewModel *)model())->sym()->searchRegEx().pattern().isEmpty() && ((TableViewModel *)model())->sym()->searchRegEx().match(textSearch).hasMatch()) {
                    pen.setColor(QColor(Qt::white));
                    QPen pen2 = pen;
                    pen2.setStyle(Qt::NoPen);
                    painter->setPen(pen2);

                    QBrush bgBrush = painter->background();
                    bgBrush.setColor(toColor(Theme::Edit_matchesBg));
                    painter->setBrush(bgBrush);

                    QRect rect2 = opt.rect;
                    rect2.setWidth(rect2.width()-1);
                    rect2.setHeight(rect2.height()-1);

                    painter->drawRect(rect2);
                }
            }

            painter->setPen(pen);
            painter->drawText(opt.rect.left(), opt.rect.top()+fontMetrics().height()*4/5+4, text);
            painter->save();

            if (dimIdxEnd == i) {
                painter->restore();
                painter->drawLine(opt.rect.left(), opt.rect.top(), opt.rect.right(), opt.rect.top());
                painter->save();
            } else if (dimIdxEnd-1 == i && dimIdxEnd == dim()) {
                painter->restore();
                if (sym()->type() == GMS_DT_VAR || sym()->type() == GMS_DT_EQU)
                    painter->drawLine(opt.rect.left(), opt.rect.top(), opt.rect.right(), opt.rect.top());
                else
                    painter->drawLine(opt.rect.left(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
                painter->save();
            }
        }
    }
    painter->setBrushOrigin(oldBO);
    painter->restore();
}

void NestedHeaderView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        ddEnabled = true;
        mDragStartPosition = event->pos();
        dimIdxStart = pointToDimension(event->pos());
    }
    QHeaderView::mousePressEvent(event);
}

void NestedHeaderView::mouseReleaseEvent(QMouseEvent *event)
{
    QHeaderView::mouseReleaseEvent(event);
}

void NestedHeaderView::mouseMoveEvent(QMouseEvent *event)
{
    QHeaderView::mouseMoveEvent(event);
    mMousePos = event->pos();
    if (!mSymbolView->dragInProgress() && (event->buttons() & Qt::LeftButton) && (mMousePos - mDragStartPosition).manhattanLength() > QApplication::startDragDistance() && ddEnabled) {
        // do not allow to drag a dummy row/column
        if (orientation() == Qt::Vertical && pointToDimension(mDragStartPosition) == 0 && sym()->needDummyRow())
            return;
        if (orientation() == Qt::Horizontal && pointToDimension(mDragStartPosition) == 0 && sym()->needDummyColumn())
            return;
        //do not allow to drag the value column (lavel, marginal,...) of variables and equations
        if (orientation() == Qt::Horizontal && (sym()->type() == GMS_DT_EQU || sym()->type() == GMS_DT_VAR) && pointToDimension(mDragStartPosition)==dim()-1)
            return;
        mSymbolView->setDragInProgress(true);
        QMimeData *mimeData = new QMimeData;
        if (orientation() == Qt::Vertical)
            mimeData->setData("GDXDRAGDROP/COL", QByteArray::number(pointToDimension(mDragStartPosition)));
        else
            mimeData->setData("GDXDRAGDROP/ROW", QByteArray::number(pointToDimension(mDragStartPosition)));
        event->accept();
        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->exec(); // blocking
        QTimer::singleShot(0, this, [this](){ mSymbolView->setDragInProgress(false); });
    }

    if(orientation() == Qt::Vertical)
        headerDataChanged(orientation(),0, qMax(0,model()->rowCount()-1));
    else
        headerDataChanged(orientation(),0, qMax(0,model()->columnCount()-1));
}

void NestedHeaderView::dragEnterEvent(QDragEnterEvent *event)
{
    decideAcceptDragEvent(event);
    event->acceptProposedAction();
}

void NestedHeaderView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("GDXDRAGDROP/ROW")) {
        dimIdxStart = event->mimeData()->data("GDXDRAGDROP/ROW").toInt();
        dragOrientationStart = Qt::Horizontal;
    }
    else {
        dimIdxStart = event->mimeData()->data("GDXDRAGDROP/COL").toInt();
        dragOrientationStart = Qt::Vertical;
    }
    dimIdxEnd = pointToDropDimension(event->position().toPoint());
    dragOrientationEnd = orientation();
    decideAcceptDragEvent(event);
    viewport()->update();
}

void NestedHeaderView::decideAcceptDragEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("GDXDRAGDROP/COL") || event->mimeData()->hasFormat("GDXDRAGDROP/ROW"))
        event->acceptProposedAction();
    else
        event->ignore();
}

int NestedHeaderView::toGlobalDim(int localDim, int orientation)
{
    if (orientation == Qt::Horizontal)
        return localDim + sym()->dim()-sym()->tvColDim();
    else
        return localDim;
}

void NestedHeaderView::updateSectionWidths()
{
    if (!sym()) return;
    int borderWidth = 10;
    QFont fnt = font();
    fnt.setBold(true);
    QFontMetrics fm(fnt);
    int dimension = dim();
    sectionWidth.clear();
    if (orientation() == Qt::Vertical) {
        sectionWidth.resize(dimension);
        for (int i=0; i<dimension; i++) {
            QVector<QList<QString>> labelsInRows = sym()->labelsInRows();
            for (const QString &label : labelsInRows.at(i))
                sectionWidth.replace(i, qMax(sectionWidth.at(i), fm.horizontalAdvance(label)));
        }
        for (int i=0; i<dimension; i++)
            sectionWidth.replace(i, sectionWidth.at(i) + borderWidth);
    } else {
        QMap<QString, int> labelWidth;
        sectionWidth.resize(this->model()->columnCount());
        for (int i=0; i<this->model()->columnCount(); i++) {
            const auto data = model()->headerData(i, Qt::Horizontal).toStringList();
            for (const QString &label : data) {
                if (!labelWidth.contains(label))
                    labelWidth.insert(label, fm.horizontalAdvance(label));
                sectionWidth.replace(i, qMax(sectionWidth.at(i), labelWidth[label]));
            }
        }
        for (int i=0; i<this->model()->columnCount(); i++)
            sectionWidth.replace(i, sectionWidth.at(i) + borderWidth);
    }
}

void NestedHeaderView::dropEvent(QDropEvent *event)
{
    dimIdxStart = toGlobalDim(dimIdxStart, dragOrientationStart);
    dimIdxEnd = toGlobalDim(dimIdxEnd, dragOrientationEnd);

    if (dimIdxStart == dimIdxEnd && dragOrientationStart == dragOrientationEnd) { //nothing happens
        event->accept();
        dimIdxEnd = -1;
        dimIdxStart = -1;
        return;
    }

    int newColDim = sym()->tvColDim();
    if (dragOrientationStart != dragOrientationEnd) {
        if (dragOrientationStart == Qt::Vertical)
            newColDim++;
        else
            newColDim--;
    }
    QVector<int> tvDims = sym()->tvDimOrder();

    if (dimIdxStart < dimIdxEnd)
        dimIdxEnd--;

    tvDims.move(dimIdxStart, dimIdxEnd);

    sym()->setTableView(newColDim, tvDims);
    event->accept();

    emit (static_cast<QTableView*>(this->parent()))->horizontalHeader()->geometriesChanged();
    emit (static_cast<QTableView*>(this->parent()))->verticalHeader()->geometriesChanged();

    GdxSymbolView *symView = static_cast<GdxSymbolView*>(parent()->parent());
    symView->moveTvFilterColumns(dimIdxStart, dimIdxEnd);

    dimIdxEnd = -1;
    dimIdxStart = -1;

    symView->toggleColumnHidden();
    symView->autoResizeTableViewColumns();
}

void NestedHeaderView::dragLeaveEvent(QDragLeaveEvent *event)
{
    QHeaderView::dragLeaveEvent(event);
    dimIdxEnd = -1;
    viewport()->update();
}

void NestedHeaderView::leaveEvent(QEvent *event)
{
    QHeaderView::leaveEvent(event);
    mMousePos = QPoint(-1,-1);
}

int NestedHeaderView::pointToDimension(QPoint p)
{
    if (orientation() == Qt::Vertical) {
        int totWidth = 0;
        for(int i=0; i<dim(); i++) {
            totWidth += sectionWidth.at(i);
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
    return 0; //is never reached
}

int NestedHeaderView::pointToDropDimension(QPoint p)
{
    int globalStart = toGlobalDim(dimIdxStart, dragOrientationStart);
    int localEnd = pointToDimension(p);
    int globalEnd   = toGlobalDim(localEnd, dragOrientationEnd);

    if ((sym()->type() == GMS_DT_EQU || sym()->type() == GMS_DT_VAR) && globalEnd == sym()->dim()) {
        if (globalStart+1 == globalEnd) {
            if (dragOrientationStart == dragOrientationEnd)
                return localEnd-1;
            else
                return 0;
        }
        else
            return dim()-1;
    }

    //special behavior for switching adjacent dimensions when start and end orientation is the same
    if (dragOrientationStart == dragOrientationEnd) {
        if (globalStart == globalEnd)
            return dimIdxStart;
        else if (globalStart+1 == globalEnd)
            return dimIdxStart+2;
        else if (globalStart-1 == globalEnd)
            return dimIdxStart-1;
    }
    if (dragOrientationEnd == Qt::Vertical) {
        if (sym()->needDummyRow())
            return 0;
        int totWidth = 0;
        for(int i=0; i<dim(); i++) {
            int curWidth = sectionWidth.at(i);
            totWidth += curWidth;
            if (p.x() < totWidth) {
                int relX = p.x() - totWidth + curWidth;
                if (relX > curWidth/2)
                    i++;
                return i;
            }
        }
    } else {
        if (sym()->needDummyColumn())
            return 0;
        int sectionHeight = QHeaderView::sectionSizeFromContents(0).height();
        int totHeight = 0;
        for(int i=0; i<dim(); i++) {
            totHeight += sectionHeight;
            if (p.y() < totHeight) {
                int relY = p.y() - totHeight + sectionHeight;
                if (relY > sectionHeight/2)
                    i++;
                return i;
            }
        }
    }
    return 0; //is never reached
}

void NestedHeaderView::bindScrollMechanism()
{
    if (!model())
        return;
    // need to update the first visible sections when scrolling in order to trigger the repaint for showing all labels for the first section
    if (orientation() == Qt::Vertical)
        connect((static_cast<QTableView*>(parent()))->verticalScrollBar(), &QScrollBar::valueChanged, static_cast<TableViewModel*>(model()), &TableViewModel::scrollVTriggered, Qt::UniqueConnection);
    else
        connect((static_cast<QTableView*>(parent()))->horizontalScrollBar(), &QScrollBar::valueChanged, static_cast<TableViewModel*>(model()), &TableViewModel::scrollHTriggered, Qt::UniqueConnection);
}

TableViewModel *NestedHeaderView::sym() const
{
    return static_cast<TableViewModel*>(model());
}


QSize NestedHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    if (orientation() == Qt::Vertical) {
        QSize s(0,sectionSize(logicalIndex));
        int totWidth = 0;
        for (int i=0; i<dim(); i++)
            totWidth += sectionWidth.at(i);
        s.setWidth(totWidth);
        return s;
    } else {
        QSize s = QHeaderView::sectionSizeFromContents(logicalIndex);
        s.setHeight(s.height()*dim());
        s.setWidth(sectionWidth.at(logicalIndex));
        return s;
    }
}

bool NestedHeaderView::event(QEvent *e)
{
    if (e->type() == QEvent::FontChange) {
        if (orientation() == Qt::Vertical) {
            updateSectionWidths();
        }
    }
    return QHeaderView::event(e);
}


} // namespace gdxviewer
} // namespace studio
} // namespace gams
