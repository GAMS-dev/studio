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

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>

namespace gams {
namespace studio {
namespace modeldialog {

class TableView : public QTableView
{
public:
    TableView(QWidget *parent = nullptr);

    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    void resetZoom();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

private:
    void zoom(int range);

private:
    QFont mBaseFont;
    int mZoomFactor = 2;
};

}
}
}

#endif // TABLEVIEW_H