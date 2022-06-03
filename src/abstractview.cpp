#include "abstractview.h"
#include "logger.h"

#include <QWheelEvent>

namespace gams {
namespace studio {

AbstractView::AbstractView(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{}

AbstractView::~AbstractView()
{
    columnsResetAll();
}

void AbstractView::columnsRegister(QHeaderView *header)
{
    if (!header || header->font().pointSizeF() < 0.0) {
        DEB() << "Font size not specified in POINT";
        return;
    }
    if (!mHeaders.contains(header))
        connect(header, &QObject::destroyed, this, &AbstractView::columnsUnregister);

    mBaseScale = header->font().pointSizeF();
    mHeaders.insert(header, ColumnWidths());
}

void AbstractView::columnsUnregister(QObject *object)
{
    QHeaderView *header = static_cast<QHeaderView*>(object);
    mHeaders.remove(header);
}

void AbstractView::columnsResetAll()
{
    mHeaders.clear();
}

void AbstractView::columnsUpdateScale()
{
    if (mHeaders.isEmpty()) return;
    HeadersData::iterator it = mHeaders.begin();
    mBaseScale = it.key()->font().pointSizeF();

    while (it != mHeaders.end()) {
        if (it.key()->font().pointSizeF() > -0.5) {
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
        ++it;
    }
}

qreal AbstractView::columnsCurrentScale() const
{
    return mBaseScale;
}

void AbstractView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        const int delta = e->angleDelta().y();
        if (delta) {
            emit zoomRequest(delta / qAbs(delta));
            e->accept();
        }
        return;
    }
    QWidget::wheelEvent(e);
}

bool AbstractView::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        columnsCurrentScale();
    }
    return QWidget::event(event);
}


} // namespace studio
} // namespace gams
