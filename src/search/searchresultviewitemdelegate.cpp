/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "searchresultviewitemdelegate.h"
#include "common.h"

#include <QApplication>
#include <QPainter>
#include <QTextCursor>
#include <QTextDocument>

namespace gams {
namespace studio {
namespace search {

SearchResultViewItemDelegate::SearchResultViewItemDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}

void SearchResultViewItemDelegate::paint(QPainter *painter,
                                         const QStyleOptionViewItem &option,
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
    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter, opt.widget);

    painter->translate(opt.rect.left(), opt.rect.top());
    QRect clip(0, 0, opt.rect.width(), opt.rect.height());
    doc.drawContents(painter, clip);

    painter->restore();
}

QSize SearchResultViewItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(size.height()*TABLE_ROW_HEIGHT);
    //size.setWidth(size.width()*TABLE_ROW_HEIGHT);
    return size;
}

QString SearchResultViewItemDelegate::elideRichText(const QString &richText,
                                                    int maxWidth,
                                                    const QFontMetrics &metrics) const
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

        return doc.toHtml();
    }
    return richText;
}

}
}
}
