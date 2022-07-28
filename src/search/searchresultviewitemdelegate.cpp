#include "searchresultviewitemdelegate.h"
#include <QApplication>
#include <QPainter>
#include <QTextDocument>

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
    int padding = 0;

    painter->save();
    painter->setClipRect(opt.rect);
    opt.rect = opt.rect.adjusted(padding, padding, -padding, -padding);

// TODO(RG): restore this elide behavior:
//    painter->drawText(opt.rect, Qt::AlignLeft | Qt::AlignVCenter,
//                      opt.fontMetrics.elidedText(opt.text, Qt::ElideRight,
//                                                 opt.rect.width()));


    QTextDocument doc;
    doc.setHtml(opt.text);

    opt.text = "";
    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

    painter->translate(opt.rect.left(), opt.rect.top());
    QRect clip(0, 0, opt.rect.width(), opt.rect.height());
    doc.drawContents(painter, clip);


    painter->restore();
}
