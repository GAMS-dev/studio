#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QToolButton>

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
    bool eventFilter(QObject*sender, QEvent* event) override;

private:
    QToolButton *bLeft = nullptr;
    QToolButton *bRight = nullptr;
};

}
}

#endif // TABWIDGET_H
