#include "abstractview.h"

namespace gams {
namespace studio {

AbstractView::AbstractView(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{}

AbstractView::~AbstractView()
{}

void AbstractView::columnsRegister(QHeaderView *header)
{
    if (!mHeaders.contains(header)) {
        connect(header, &QObject::destroyed, this, &AbstractView::columnsUnregister);
        mHeaders.insert(header, ColumnWidths());
    }
    for (int i = 0; i < header->count(); ++i) {
        mHeaders[header] << header->sectionSize(i);
    }
}

void AbstractView::columnsResetAll()
{
    mHeaders.clear();
}

QList<qreal> AbstractView::columnWidths(QHeaderView *header, qreal scale)
{
    QList<qreal> res;
    if (mHeaders.contains(header)) {
        if (scale < 0.000001) return mHeaders.value(header);
        for (int i = 0; i < header->count(); ++i) {
            res << header->sectionSize(i) * scale / mBaseScale;
        }
    }
    return res;
}

void AbstractView::setScale(qreal scale)
{
    mBaseScale = scale;
    storeColumnWidths();
}

void AbstractView::columnsUnregister(QObject *object)
{
    QHeaderView *header = static_cast<QHeaderView*>(object);
    mHeaders.remove(header);
}

void AbstractView::storeColumnWidths()
{
    HeadersData::iterator it = mHeaders.begin();
    while (it != mHeaders.end()) {
        it.value().clear();
        for (int i = 0; i < it.value().count(); ++i) {
            it.value() << it.key()->sectionSize(i);
        }
        ++it;
    }
}




} // namespace studio
} // namespace gams
