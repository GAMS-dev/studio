#include "searchresultviewitemdelegate.h"
#include <QApplication>
#include <QPainter>

SearchResultViewItemDelegate::SearchResultViewItemDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{ }

void SearchResultViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                     const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    int padding = 3;

    painter->save();
    painter->setClipRect(opt.rect);
    opt.rect = opt.rect.adjusted(padding, padding, -padding, -padding);
    painter->drawText(opt.rect, Qt::AlignLeft | Qt::AlignVCenter,
                      opt.fontMetrics.elidedText(opt.text, Qt::ElideRight,
                                                 opt.rect.width()));
    painter->restore();

    opt.rect = option.rect;
    opt.textElideMode = Qt::ElideNone;
    opt.text = "";

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
}
