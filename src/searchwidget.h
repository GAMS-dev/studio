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

private slots:
    void on_buttonFind_clicked();

    void on_buttonReplace_clicked();

    void on_buttonFindAll_clicked();

    void on_buttonReplaceAll_clicked();

private:
    Ui::SearchWidget *ui;
    RecentData &mRecent;

};

}
}
#endif // SEARCHWIDGET_H
