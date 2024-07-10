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
#ifndef GDXSYMBOLHEADERVIEW_H
#define GDXSYMBOLHEADERVIEW_H

#include <QHeaderView>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    enum HeaderType { ListView, TableViewFilter };

    GdxSymbolHeaderView(Qt::Orientation orientation, GdxSymbolHeaderView::HeaderType headerType, QWidget *parent = nullptr);
    ~GdxSymbolHeaderView() override;
    QSize sectionSizeFromContents(int logicalIndex) const override;

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void mousePressEvent(QMouseEvent * event) override;
    bool event(QEvent * event) override;

private:
    bool pointFilterIconCollision(QPoint p);

private:
    HeaderType mHeaderType;

    QString iconFilterOn = ":/img/filter";
    QString iconFilterOff = ":/img/filter-off";
    const double ICON_SCALE_FACTOR = 0.5;
    const double ICON_MARGIN_LEFT = 0.2;
    const double ICON_MARGIN_BOTTOM = 0.1;
    const double SECTION_WIDTH_FACTOR = 2.0;
    const double SECTION_WIDTH_FACTOR_MACOS = 2.5;

    int mFilterIconWidth;
    int mFilterIconMarginLeft;
    int mFilterIconMarginBottom;
    mutable std::vector<int> mFilterIconX;
    mutable std::vector<int> mFilterIconY;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GDXSYMBOLHEADERVIEW_H
