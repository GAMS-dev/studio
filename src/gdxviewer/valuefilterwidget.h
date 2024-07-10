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
#ifndef GAMS_STUDIO_GDXVIEWER_VALUEFILTERWIDGET_H
#define GAMS_STUDIO_GDXVIEWER_VALUEFILTERWIDGET_H

#include "gdxsymbol.h"
#include "valuefilter.h"

#include <QWidget>

namespace gams {
namespace studio {
namespace gdxviewer {

namespace Ui {
class ValueFilterWidget;
}

class ValueFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ValueFilterWidget(ValueFilter* valueFilter, QWidget *parent = nullptr);
    ~ValueFilterWidget();
    void setFocusOnOpen();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void on_pbApply_clicked();
    void on_pbReset_clicked();

private:
    ValueFilter* mValueFilter;
    Ui::ValueFilterWidget *ui;
    double mMin;
    double mMax;
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_VALUEFILTERWIDGET_H
