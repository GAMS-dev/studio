#include "gotowidget.h"
#include "ui_gotowidget.h"
#include <QDebug>
#include "syntax.h"
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


void GoToWidget::on_GoTo_clicked()
{
    int altLine =(ui->lineEdit->text().toInt())-1;
    QTextCursor cursor;
    FileContext* fc = mMain->fileRepository()->fileContext(mMain->recent()->editor);
    fc->jumpTo(cursor, true,altLine, 1);
}

void GoToWidget::on_Cancel_clicked()
{
hide();
}

}
}
