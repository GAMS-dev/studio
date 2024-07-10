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
#ifndef ADDOPTIONHEADERVIEW_H
#define ADDOPTIONHEADERVIEW_H

#include <QHeaderView>

namespace gams {
namespace studio {
namespace option {

class AddOptionHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    AddOptionHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);
    ~AddOptionHeaderView() override;

protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

private:
    const double ICON_SCALE_FACTOR = 0.7;
    const double ICON_MARGIN_FACTOR = 0.3;

    mutable int mIconWidth;
    mutable int mIconX;
    mutable int mIconY;
    mutable int mLogicalIndex;

    bool isAddOptionCoordinate(QPoint p);
};

} // namepsage option
} // namespace studio
} // namespace gams

#endif // ADDOPTIONHEADERVIEW_H
