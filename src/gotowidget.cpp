#include "gotowidget.h"
#include "ui_gotowidget.h"
#include "filecontext.h"
#include "mainwindow.h"

namespace gams {
namespace studio {


GoToWidget::GoToWidget(MainWindow *parent) :
    QDialog(parent), ui(new Ui::GoToWidget), mMain(parent)
{
    ui->setupUi(this);
    setFixedSize(size());
}

GoToWidget::~GoToWidget()
{
    delete ui;
}

void GoToWidget::focusTextBox()
{
    this->setWindowTitle("Go To");
    ui->lineEdit->setFocus();
}

void GoToWidget::on_GoTo_clicked()
{
    int altLine =(ui->lineEdit->text().toInt())-1;
    QTextCursor cursor;
    FileContext* fc = mMain->fileRepository()->fileContext(mMain->recent()->editor);
    if (!fc) return;
    fc->jumpTo(cursor, true,altLine,0);
    ui->lineEdit->setText("");
}

void GoToWidget::keyPressEvent(QKeyEvent *event)
{
    if ( isVisible() &&  (event->key() == Qt::Key_Escape)) {
        hide();
    }
    if ( isVisible() &&  ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return))) {
        GoToWidget::on_GoTo_clicked();
    }
}

void GoToWidget::keyReleaseEvent(QKeyEvent *event)
{
    if ( isVisible() &&  ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return))) {
        hide();
    }
}

}
}
