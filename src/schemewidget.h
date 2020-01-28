#ifndef GAMS_STUDIO_SCHEMEWIDGET_H
#define GAMS_STUDIO_SCHEMEWIDGET_H

#include <QFrame>
#include "scheme.h"
#include "svgengine.h"

namespace Ui {
class SchemeWidget;
}

namespace gams {
namespace studio {

class SchemeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SchemeWidget(QWidget *parent);
    explicit SchemeWidget(Scheme::ColorSlot slotFg, QWidget *parent, bool iconExample = false);
    explicit SchemeWidget(Scheme::ColorSlot slotFg = Scheme::invalid,
                          Scheme::ColorSlot slotBg = Scheme::invalid, QWidget *parent = nullptr);
    explicit SchemeWidget(Scheme::ColorSlot slotFg, Scheme::ColorSlot slotBg,
                          Scheme::ColorSlot slotBg2, QWidget *parent = nullptr);
    ~SchemeWidget() override;
    void setText(const QString &text);
    QString text() const;
    void setTextVisible(bool visible);
    void setFormatVisible(bool visible);
    bool eventFilter(QObject *watched, QEvent *event) override;
    void selectColor(QFrame *frame, Scheme::ColorSlot slot);
    void saveToScheme();
    void refresh();
    void setAlignment(Qt::Alignment align);

signals:
    void changed();

private:
    Ui::SchemeWidget *ui;
    Scheme::ColorSlot mSlotFg = Scheme::invalid;
    Scheme::ColorSlot mSlotBg = Scheme::invalid;
    Scheme::ColorSlot mSlotBg2 = Scheme::invalid;
    bool mChanged = false;
    SvgEngine *mIconEng = nullptr;

    void initSlot(Scheme::ColorSlot &slotVar, const Scheme::ColorSlot &slotVal, QFrame *frame);
    void setColor(QFrame *frame, const QColor &color, int examplePart = 0);
};


} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_SCHEMEWIDGET_H
