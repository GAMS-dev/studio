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
#ifndef GAMS_STUDIO_GDXVIEWER_VALUEFILTERDIALOG_H
#define GAMS_STUDIO_GDXVIEWER_VALUEFILTERDIALOG_H

#include "gdxsymbol.h"
#include <QDialog>

namespace gams {
namespace studio {
namespace gdxviewer {

namespace Ui {
class ValueFilterDialog;
}

class ValueFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ValueFilterDialog(GdxSymbol *symbol, int valueColumn, ValueFilter& valueFilter, QWidget *parent = nullptr);
    ~ValueFilterDialog();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private slots:
    void on_pbApply_clicked();
    void on_pbReset_clicked();

private:
    Ui::ValueFilterDialog *ui;
    double mMin;
    double mMax;

    GdxSymbol* mSymbol = nullptr;
    int mValueColumn;
    ValueFilter& mValueFilter;
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_VALUEFILTERDIALOG_H
