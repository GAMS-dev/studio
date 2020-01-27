#ifndef GAMS_STUDIO_SCHEMEWIDGET_H
#define GAMS_STUDIO_SCHEMEWIDGET_H

#include <QFrame>
#include "scheme.h"

namespace Ui {
class SchemeWidget;
}

namespace gams {
namespace studio {

class SchemeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SchemeWidget(QWidget *parent = nullptr, Scheme::ColorSlot slotFg = Scheme::invalid,
                          Scheme::ColorSlot slotBg = Scheme::invalid, Scheme::ColorSlot slotBg2 = Scheme::invalid);
    ~SchemeWidget() override;
    void setColorSlot(const Scheme::ColorSlot slotFG, const Scheme::ColorSlot slotBG = Scheme::invalid,
                      const Scheme::ColorSlot slotBG2 = Scheme::invalid);
    void setText(const QString &text);
    QString text() const;
    void setTextVisible(bool visible);
    void setFormatVisible(bool visible);
    bool eventFilter(QObject *watched, QEvent *event) override;
    void selectColor(QFrame *frame);
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

    void setColor(QFrame *frame, const QColor &color);

};


} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_SCHEMEWIDGET_H
