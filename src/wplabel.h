#ifndef WPLABEL_H
#define WPLABEL_H

#include <QLabel>

namespace gams {
namespace studio {

class WpLabel : public QLabel
{
public:
    WpLabel(const QString &content);

protected:
    void mousePressEvent(QMouseEvent *event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);

private:
    const QString &mContent;
};

}
}
#endif // WPLABEL_H
