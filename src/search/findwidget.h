#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QWidget>

namespace gams {
namespace studio {
namespace find {

namespace Ui {
class FindWidget;
}

class FindWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindWidget(QWidget *parent = nullptr);
    ~FindWidget();
    bool active() const;
    void setActive(bool newActive);
    void toggleActive();

protected:
    void focusInEvent(QFocusEvent *event);

private:
    Ui::FindWidget *ui;
    bool mActive = false;

};

} // namespace find
} // namespace studio
} // namespace gams
#endif // FINDWIDGET_H
