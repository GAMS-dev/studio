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
#ifndef GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H
#define GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H

#include "ui_columnfilterframe.h"
#include "filteruelmodel.h"
#include <QVector>

namespace gams {
namespace studio {
namespace gdxviewer {

class ColumnFilterFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ColumnFilterFrame(GdxSymbol* symbol, int column, QWidget *parent = nullptr);
    ~ColumnFilterFrame() override;
    void setFocusOnOpen();

protected:
    //mouse events overwritten to prevent closing of the filter menu if user click on empty spaces regions within the frame
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void keyPressEvent(QKeyEvent *e) override;

private slots:
    void apply();
    void selectAll();
    void invert();
    void deselectAll();
    void filterLabels();
    void toggleHideUnselected(bool checked);
    void listDataHasChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

private:
    Ui::ColumnFilterFrame ui;
    GdxSymbol* mSymbol;
    int mColumn;
    FilterUelModel* mModel;
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_COLUMNFILTERFRAME_H
