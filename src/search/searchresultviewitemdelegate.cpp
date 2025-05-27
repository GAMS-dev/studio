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
#include "searchresultviewitemdelegate.h"
#include "common.h"
#include "resultitem.h"

#include <QApplication>
#include <QPainter>
#include <QTextCursor>
#include <QTextDocument>

namespace gams {
namespace studio {
namespace search {

SearchResultViewItemDelegate::SearchResultViewItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , mFontMetrics(static_cast<QWidget*>(parent)->font())
{
    setFont(static_cast<QWidget*>(parent)->font());
}

void SearchResultViewItemDelegate::setFont(const QFont &font)
{
    mFont = font;
    mFontMetrics = QFontMetrics(mFont);
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
    doc.setHtml(elideRichText(opt.text, opt.rect.width(), mFontMetrics));

    opt.text = "";
    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter, opt.widget);

    painter->translate(opt.rect.left(), opt.rect.top());
    QRect clip(0, 0, opt.rect.width(), opt.rect.height());
    doc.setDefaultFont(mFont);
    doc.drawContents(painter, clip);

    painter->restore();
}

QSize SearchResultViewItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    auto size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(static_cast<int>(size.height()*TABLE_ROW_HEIGHT));
    if (index.isValid()) {
        auto item = static_cast<ResultItem*>(index.internalPointer());
        if (!item->hasChilds()) {
            QTextDocument doc;
            doc.setHtml(index.data().toString());
            size.setWidth(static_cast<int>(doc.size().width()));
        } else {
            auto s = mFontMetrics.size(Qt::TextSingleLine, " ").width();
            auto t = mFontMetrics.size(Qt::TextSingleLine, index.data().toString()).width();
            size.setWidth(t+s);
        }
    }
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
