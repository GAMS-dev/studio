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
#ifndef GAMS_STUDIO_PINVIEWWIDGET_H
#define GAMS_STUDIO_PINVIEWWIDGET_H

#include <QWidget>
#include <QSplitter>

#include "common.h"

namespace gams {
namespace studio {
namespace pin {

namespace Ui {
class PinViewWidget;
}

class PinViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PinViewWidget(QWidget *parent = nullptr);
    ~PinViewWidget() override;
    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation();
    bool setWidget(QWidget *widget);
    QWidget *widget();
    void setTabName(const QString &tabName);
    void setFileName(const QString &tabName, const QString &filePath);
    void setFontGroup(FontGroup fontGroup);
    void setScrollLocked(bool lock);
    bool isScrollLocked();
    QSize preferredSize();
    void showAndAdjust(Qt::Orientation orientation);
    QList<int> sizes();
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void hidden();

private slots:
    void splitterMoved(int pos, int index);

    void onSwitchOrientation();
    void onSyncScroll(bool checked);
    void onClose();

protected:


private:
    Ui::PinViewWidget *ui;
    QAction *mActOrient;
    QAction *mActSync;
    QAction *mActClose;
    QSplitter *mSplitter = nullptr;
    QWidget *mWidget = nullptr;
    QSize mPrefSize;
};


} // namespace split
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_PINVIEWWIDGET_H
