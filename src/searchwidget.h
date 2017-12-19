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
    void on_btn_Find_clicked();

    void on_btn_FindAll_clicked();

    void on_btn_Replace_clicked();

    void on_btn_ReplaceAll_clicked();

    void on_txt_search_returnPressed();

private:
    Ui::SearchWidget *ui;
    RecentData &mRecent;
    QTextCursor mSelection;       // selected with find
    QTextCursor mLastSelection;   // last selection, as starting point for find next
    bool mMutliSelection = false; // 'find all' pressed

    void showEvent(QShowEvent *event);
    QFlags<QTextDocument::FindFlag> getFlags();
};

}
}
#endif // SEARCHWIDGET_H
