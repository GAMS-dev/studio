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
    explicit ThemeWidget(Theme::ColorSlot slotFg, QWidget *parent, bool iconExample = false);
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


signals:
    void changed();

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

    void initSlot(Theme::ColorSlot &slotVar, const Theme::ColorSlot &slotVal, QFrame *frame);
    void setColor(QFrame *frame, const QColor &color, int examplePart = 0);
};


} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_THEMEWIDGET_H
