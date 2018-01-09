#include "resultsview.h"
#include "searchwidget.h"
#include "ui_resultsview.h"

namespace gams {
namespace studio {

ResultsView::ResultsView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResultsView)
{
    ui->setupUi(this);
}

ResultsView::~ResultsView()
{
    delete ui;
}

void ResultsView::addItem(Result r)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(r.locFile()));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(r.locLineNr())));
    ui->tableWidget->setItem(row, 2, new QTableWidgetItem(r.context()));
}

void ResultsView::resizeColumnsToContent()
{
    ui->tableWidget->resizeColumnsToContents();
}

}
}
