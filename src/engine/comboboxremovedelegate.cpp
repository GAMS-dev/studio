/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "comboboxremovedelegate.h"
#include "theme.h"
#include "logger.h"

#include <QPainter>
#include <QMouseEvent>
#include <QListView>
#include <QToolTip>

namespace gams {
namespace studio {

ComboboxRemoveDelegate::ComboboxRemoveDelegate(QComboBox *parent)
    : QStyledItemDelegate{parent}
{}

void ComboboxRemoveDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    bool isEmpty = index.model()->data(index, Qt::UserRole).toHash().isEmpty();
    painter->save();

    QRect r = opt.rect;
    int btnWidth = 24;
    QString text = opt.text;
    opt.text = "";
    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    r.setRight(r.right() - btnWidth);
    painter->drawText(r.adjusted(5, 0, 0, 0), opt.displayAlignment, text);

    QRect btnRect(opt.rect.right() - btnWidth, opt.rect.top(), btnWidth, opt.rect.height());
    int padding = 4;
    QRect iconRect = btnRect.adjusted(padding, padding, -padding, -padding);
    QIcon::Mode mode = QIcon::Normal;
    if (opt.state & QStyle::State_MouseOver) {
        QPoint mousePos = opt.widget->mapFromGlobal(QCursor::pos());
        if (btnRect.contains(mousePos) && !isEmpty)
            mode = QIcon::Active;
    }
    if (!isEmpty)
        Theme::icon(":/solid/delete-all").paint(painter, iconRect, Qt::AlignCenter, mode);

    painter->restore();
}

QSize ComboboxRemoveDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setWidth(s.width() - s.height());
    s.setHeight(s.height() + 4);
    return s;
}

bool ComboboxRemoveDelegate::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseMove
        || event->type() == QEvent::MouseButtonPress
        || event->type() == QEvent::ToolTip) {
        auto* viewport = qobject_cast<QWidget*>(watched);
        auto* listView = viewport ? qobject_cast<QListView*>(viewport->parentWidget()) : nullptr;

        if (listView) {
            QPoint pos = static_cast<QHelpEvent*>(event)->pos();
            QModelIndex index = listView->indexAt(pos);

            if (index.isValid()) {
                bool isEmpty = index.model()->data(index, Qt::UserRole).toHash().isEmpty();
                QRect itemRect = listView->visualRect(index);
                QRect btnRect(itemRect.right() - 24, itemRect.top(), 24, itemRect.height());
                bool inBtn = btnRect.contains(pos) && !isEmpty;

                if (event->type() == QEvent::ToolTip) {
                    inBtn ? QToolTip::showText(static_cast<QHelpEvent*>(event)->globalPos(), tr("Delete entry"), viewport)
                          : QToolTip::hideText();
                    return true;
                }
                if (!inBtn)
                    QToolTip::hideText();

                if (event->type() == QEvent::MouseButtonPress && inBtn
                    && static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) {
                    if (auto* model = listView->model()) {
                        model->removeRow(index.row());
                        viewport->update();
                        return true;
                    }
                }
                viewport->update(itemRect);
            }
        }
    }
    return QStyledItemDelegate::eventFilter(watched, event);
}


} // namespace studio
} // namespace gams
