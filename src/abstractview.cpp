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
#include "abstractview.h"
#include "logger.h"

#include <QWheelEvent>
#include <QTimer>
#include <QCoreApplication>
#include <QScrollArea>

namespace gams {
namespace studio {

AbstractView::AbstractView(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
}

AbstractView::~AbstractView()
{
    headerResetAll();
}

void AbstractView::zoomIn(int range)
{
    zoomInF(range);
}

void AbstractView::zoomOut(int range)
{
    zoomInF(-range);
}

void AbstractView::zoomInF(qreal range)
{
    if (range == 0.)
        return;
    QFont f = font();
    const qreal newSize = f.pointSizeF() + range;
    if (newSize <= 0)
        return;
    f.setPointSizeF(newSize);
    setFont(f);
}

void AbstractView::headerRegister(QHeaderView *header)
{
    if (!header || header->font().pointSizeF() < 0.0) {
        DEB() << "Font size not specified in POINT";
        return;
    }
    if (!mHeaders.contains(header)) {
        connect(header, &QObject::destroyed, this, &AbstractView::headerUnregister);
        connect(header, &QHeaderView::sectionResized, this, [this, header](int logicalIndex, int oldSize, int newSize)
        { headerStore(header, logicalIndex, oldSize, newSize); });
    }
    mBaseScale = header->font().pointSizeF();
    QAbstractItemView *vport = qobject_cast<QAbstractItemView *>(header->parentWidget());
    if (vport) vport->viewport()->installEventFilter(this);
    HeadersData::iterator it = mHeaders.insert(header, ColumnWidths());
    headerUpdate(it);
}

void AbstractView::headerUnregister(QObject *object)
{
    QHeaderView *header = static_cast<QHeaderView*>(object);
    mHeaders.remove(header);
}

void AbstractView::headerResetAll()
{
    HeadersData::iterator it = mHeaders.begin();
    while (it != mHeaders.end()) {
        disconnect(it.key(), &QObject::destroyed, this, &AbstractView::headerUnregister);
        ++it;
    }
    mHeaders.clear();
}

void AbstractView::headerUpdateAll()
{
    if (mHeaders.isEmpty()) return;
    HeadersData::iterator it = mHeaders.begin();
    mBaseScale = it.key()->font().pointSizeF();

    while (it != mHeaders.end()) {
        headerUpdate(it);
        ++it;
    }
}

void AbstractView::headerUpdate(HeadersData::iterator &it)
{
    if (it.key()->orientation() == Qt::Vertical) {
        it.key()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
    } else if (it.key()->font().pointSizeF() > -0.5) {
        qreal scale = it.key()->font().pointSizeF();
        while (it.value().count() > it.key()->count()) it.value().removeLast();
        for (int i = 0; i < it.key()->count(); ++i) {
            if (i >= it.value().count()) {
                it.value() << it.key()->sectionSize(i) / scale;
            } else {
                it.key()->resizeSection(i, qRound(it.value().at(i) * scale));
            }
        }
    }
}

void AbstractView::headerStore(QHeaderView *header, int logicalIndex, int oldSize, int newSize)
{
    Q_UNUSED(oldSize)
    if (mInternalResize) return;
    HeadersData::iterator it = mHeaders.find(header);
    if (it == mHeaders.end()) return;

    qreal scale = header->font().pointSizeF();
    while (it.value().count() > it.key()->count()) it.value().removeLast();
    while (it.value().count() < it.key()->count()) it.value() << it.key()->sectionSize(it.value().count()) / scale;

    it.value().replace(logicalIndex, newSize / scale);

}

qreal AbstractView::headerCurrentScale() const
{
    return mBaseScale;
}

void AbstractView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        const int delta = e->angleDelta().y();
        if (delta) {
            emit zoomRequest(delta / qAbs(delta));
        }
        e->accept();
        return;
    }
    QWidget::wheelEvent(e);
}

bool AbstractView::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        headerUpdateAll();
    }
    return QWidget::event(event);
}

bool AbstractView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Wheel && watched != this) {
        QWheelEvent *we = static_cast<QWheelEvent*>(event);
        if (we->modifiers().testFlag(Qt::ControlModifier)) {
            wheelEvent(we);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}


} // namespace studio
} // namespace gams
