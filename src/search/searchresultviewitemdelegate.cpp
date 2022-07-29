#include "searchresultviewitemdelegate.h"
#include <QApplication>
#include <QPainter>
#include <QTextCursor>
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

    QTextDocument doc;
    doc.setHtml(elideRichText(opt.text, opt.rect.width(), opt.fontMetrics));

    opt.text = "";
    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

    painter->translate(opt.rect.left(), opt.rect.top());
    QRect clip(0, 0, opt.rect.width(), opt.rect.height());
    doc.drawContents(painter, clip);

    painter->restore();
}

QString SearchResultViewItemDelegate::elideRichText(const QString &richText, int maxWidth, QFontMetrics metrics) const
{
    QTextDocument doc;
    doc.setHtml(richText);
    doc.adjustSize();

    // Elide text
    if (metrics.horizontalAdvance(doc.toPlainText()) > maxWidth) {
        QTextCursor cursor(&doc);
        cursor.movePosition(QTextCursor::End);

        const QString elidedPostfix = "...";
        int postfixWidth = metrics.horizontalAdvance(elidedPostfix);

        while (metrics.horizontalAdvance(doc.toPlainText()) > maxWidth - postfixWidth) {
            cursor.deletePreviousChar();
            doc.adjustSize();
        }

        cursor.insertText(elidedPostfix);
        return doc.toRawText();
    }

    return richText;
}
