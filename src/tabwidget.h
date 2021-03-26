#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>

namespace gams {
namespace studio {

class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    TabWidget(QWidget *parent = nullptr);

signals:
    void closeTab(int);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
};

}
}

#endif // TABWIDGET_H
