#ifndef WPLABEL_H
#define WPLABEL_H

#include <QLabel>

namespace gams {
namespace studio {

class WpLabel : public QLabel
{
public:
    WpLabel(QWidget *parent = nullptr);
    WpLabel(const QString &content, const QString &link, QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);

private:
    QString mContent;
    QString mLink;
};

}
}
#endif // WPLABEL_H
