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
#ifndef GAMS_STUDIO_CONNECT_CONNECTDATAACTIONDELEGATE_H
#define GAMS_STUDIO_CONNECT_CONNECTDATAACTIONDELEGATE_H

#include <QStyledItemDelegate>

namespace gams {
namespace studio {
namespace connect {

class ConnectDataActionDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ConnectDataActionDelegate( QObject *parent = nullptr);
    ~ConnectDataActionDelegate() override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

signals:
    void requestDeleteItem(const QModelIndex &index);
    void requestMoveDownItem(const QModelIndex &index);
    void requestMoveUpItem(const QModelIndex &index);

private:
    mutable int mIconWidth;
    mutable int mIconHeight;
    mutable int  mIconMargin;

    mutable QMap<QModelIndex, QRect> mDeleteActionPosition;
    mutable QMap<QModelIndex, QRect> mMoveUpActionPosition;
    mutable QMap<QModelIndex, QRect> mMoveDownActionPosition;
};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_CONNECT_CONNECTDATAACTIONDELEGATE_H
