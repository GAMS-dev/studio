/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
 */
#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QToolButton>

namespace gams {
namespace studio {

class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    TabWidget(QWidget *parent = nullptr);

signals:
    void closeTab(int);
    void openPinView(int tabIndex, Qt::Orientation orientation);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject*sender, QEvent* event) override;

private:
    QToolButton *bLeft = nullptr;
    QToolButton *bRight = nullptr;
    int mWheelSum = 0;
};

}
}

#endif // TABWIDGET_H
