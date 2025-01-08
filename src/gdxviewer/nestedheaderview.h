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
#ifndef NESTEDHEADERVIEW_H
#define NESTEDHEADERVIEW_H

#include "gdxsymbolview.h"
#include "tableviewmodel.h"
#include <QHeaderView>

#include <QPainter>
#include <QMouseEvent>

namespace gams {
namespace studio {
namespace gdxviewer {

class NestedHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    NestedHeaderView(Qt::Orientation orientation, GdxSymbolView* symbolView, QWidget *parent = nullptr);
    ~NestedHeaderView() override;
    void setModel(QAbstractItemModel *model) override;

    void setDdEnabled(bool value);
    int dim() const;

public slots:
    void reset() override;

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void leaveEvent(QEvent *event) override;
    QSize sectionSizeFromContents(int logicalIndex) const override;
    bool event(QEvent *e) override;

private:
    GdxSymbolView *mSymbolView = nullptr;
    int pointToDimension(QPoint p);
    int pointToDropDimension(QPoint p);
    void bindScrollMechanism();
    void decideAcceptDragEvent(QDragMoveEvent* event);
    int toGlobalDim(int localDim, int orientation);
    void updateSectionWidths();

    TableViewModel* sym() const;
    QPoint mMousePos = QPoint(-1,-1);
    QPoint mDragStartPosition;

    int dimIdxStart = -1;
    int dimIdxEnd = -1;

    int dragOrientationStart = -1;
    int dragOrientationEnd = -1;

    QVector<int> sectionWidth;
    bool ddEnabled = true;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // NESTEDHEADERVIEW_H
