#ifndef WPLABEL_H
#define WPLABEL_H

#include <QLabel>

namespace gams {
namespace studio {

class WpLabel : public QLabel
{
    Q_OBJECT

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

signals:
    void relayActionLab(QString action);
    void relayModLibLoad(QString lib);
};

}
}
#endif // WPLABEL_H
