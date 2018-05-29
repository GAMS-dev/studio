/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef GOTOWIDGET_H
#define GOTOWIDGET_H

#include <QDialog>

namespace Ui {
class GoToWidget;
}

namespace gams {
namespace studio {

class MainWindow;

class GoToWidget : public QDialog
{
    Q_OBJECT

public:
    explicit GoToWidget(MainWindow *parent = nullptr);

    ~GoToWidget();

    void focusTextBox();

private slots:
    void on_goToButton_clicked();

private:
    Ui::GoToWidget *ui;
    MainWindow *mMain;
    void keyPressEvent(QKeyEvent *e);
};

}
}
#endif // GOTOWIDGET_H
