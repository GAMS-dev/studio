#ifndef MODELDIALOG_H
#define MODELDIALOG_H

#include "ui_modeldialog.h"

class ModelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModelDialog(QWidget *parent = 0);

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

private:
    Ui::ModelDialog ui;
    bool populateTable(QTableWidget *tw, QString glbFile);

    QList<QPair<QTableWidget*, QString>> libraryList;
};

#endif // MODELDIALOG_H
