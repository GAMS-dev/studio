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
#ifndef COMBOBOXREMOVEDELEGATE_H
#define COMBOBOXREMOVEDELEGATE_H

#include <QComboBox>
#include <QStyledItemDelegate>

namespace gams {
namespace studio {

class ComboboxRemoveDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ComboboxRemoveDelegate(QComboBox *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void entryRemoved();
};

} // namespace studio
} // namespace gams

#endif // COMBOBOXREMOVEDELEGATE_H
