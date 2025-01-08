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
#ifndef GAMS_STUDIO_THEMEWIDGET_H
#define GAMS_STUDIO_THEMEWIDGET_H

#include <QFrame>
#include "theme.h"
#include "svgengine.h"

namespace Ui {
class ThemeWidget;
}

class QColorDialog;

namespace gams {
namespace studio {

class ThemeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ThemeWidget(QWidget *parent);
    explicit ThemeWidget(QList<Theme::ColorSlot> colors, QWidget *parent, bool iconExample = false);
    explicit ThemeWidget(Theme::ColorSlot slotFg = Theme::invalid,
                          Theme::ColorSlot slotBg = Theme::invalid, QWidget *parent = nullptr);
    explicit ThemeWidget(Theme::ColorSlot slotFg, Theme::ColorSlot slotBg,
                          Theme::ColorSlot slotBg2, QWidget *parent = nullptr);
    ~ThemeWidget() override;
    void setTextVisible(bool visible);
    void setFormatVisible(bool visible);
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showColorSelector(QFrame *frame);
    void refresh();
    void setAlignment(Qt::Alignment align);
    void setReadonly(bool readonly);

signals:
    void aboutToChange();
    void changed();
    void getTextBackground(QColor bkColor);

private slots:
    void colorChanged(const QColor &color);
    void fontFlagsChanged();

private:
    Ui::ThemeWidget *ui;
    Theme::ColorSlot mSlotFg = Theme::invalid;
    Theme::ColorSlot mSlotBg = Theme::invalid;
    Theme::ColorSlot mSlotBg2 = Theme::invalid;
    bool mChanged = false;
    SvgEngine *mIconEng = nullptr;
    QColorDialog *mColorDialog = nullptr;
    QFrame *mSelectedFrame;
    bool mReadonly = false;
    bool mHasAutoBackground = false;

    void baseInit();
    void initSlot(Theme::ColorSlot &slotVar, const Theme::ColorSlot &slotVal, QFrame *frame);
    void setColor(QFrame *frame, const QColor &color, int examplePart = 0);
};


} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_THEMEWIDGET_H
