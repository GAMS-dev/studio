#ifndef GAMS_STUDIO_SCHEMEWIDGET_H
#define GAMS_STUDIO_SCHEMEWIDGET_H

#include <QWidget>
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
    explicit SchemeWidget(QWidget *parent = nullptr, Scheme::ColorSlot slot = Scheme::invalid);
    ~SchemeWidget() override;
    void setColorSlot(const Scheme::ColorSlot slot);
    void setText(const QString &text);
    QString text() const;
    void setTextVisible(bool visible);
    void setFormatVisible(bool visible);
    bool eventFilter(QObject *watched, QEvent *event) override;
    void selectColor();
    void saveToScheme();
    void refresh();

signals:
    void changed();

private:
    Ui::SchemeWidget *ui;
    Scheme::ColorSlot mSlot = Scheme::invalid;
    bool mChanged = false;

    void setColor(const QColor &color);

};


} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_SCHEMEWIDGET_H
