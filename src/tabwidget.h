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

public slots:
    void updateSystemLogTab();
    void resetSystemLogTab(int index);

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
    int mMsgCount = 0;
};

}
}

#endif // TABWIDGET_H
