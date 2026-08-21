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

#include <QPainter>
#include <QMouseEvent>

namespace gams {
namespace studio {

ComboboxRemoveDelegate::ComboboxRemoveDelegate(QComboBox *parent)
    : QStyledItemDelegate{parent}
{}

void ComboboxRemoveDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->save();

    QRect r = opt.rect;
    int btnWidth = opt.rect.height();
    r.setRight(r.right() - btnWidth);
    opt.text = "";
    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    r.setRight(r.right() - (btnWidth * 2));
    painter->drawText(r.adjusted(5, 0, 0, 0), opt.displayAlignment, option.text);

    painter->setPen(opt.state & QStyle::State_Selected ? Qt::white : QColor(200, 50, 50));
    QRect btn(opt.rect.right() - (btnWidth * 2), opt.rect.top(), btnWidth, opt.rect.height());
    painter->drawText(btn, Qt::AlignCenter, "✕");

    painter->restore();
}

QSize ComboboxRemoveDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setWidth(s.width() + 30);
    s.setHeight(qMax(s.height(), 28));
    return s;
}

bool ComboboxRemoveDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                         const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QRect btn(option.rect.right() - 20, option.rect.top(), 20, option.rect.height());
        if (btn.contains(static_cast<QMouseEvent*>(event)->pos())) {
            model->removeRow(index.row());
            mComboBox->showPopup(); // Hält Dropdown offen
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

} // namespace studio
} // namespace gams
