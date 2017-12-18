#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "mainwindow.h"
#include <QFrame>

namespace Ui {
class SearchWidget;
}

namespace gams {
namespace studio {

class SearchWidget : public QFrame
{
    Q_OBJECT

public:
    explicit SearchWidget(RecentData &rec, QWidget *parent = 0);
    ~SearchWidget();

private:
    Ui::SearchWidget *ui;
    RecentData &mRecent;

};

}
}
#endif // SEARCHWIDGET_H
