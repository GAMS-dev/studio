#ifndef GOTOWIDGET_H
#define GOTOWIDGET_H

#include <QDialog>
#include "mainwindow.h"
#include "filecontext.h"

namespace Ui {
class GoToWidget;
}

namespace gams {
namespace studio {

class GoToWidget : public QDialog
{
    Q_OBJECT

public:
    explicit GoToWidget(MainWindow *parent = 0);

    ~GoToWidget();

    void focusTextBox();

protected:
   // void closeEvent(QCloseEvent *event);
   // void showEvent(QShowEvent *event);

private slots:
    void on_GoTo_clicked();
    void on_Cancel_clicked();

private:
    Ui::GoToWidget *ui;
    MainWindow *mMain;
    QTextCursor mSelection;
    QList<TextMark*> mAllTextMarks;

};

}
}
#endif // GOTOWIDGET_H
