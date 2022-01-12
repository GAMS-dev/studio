/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

#include <QDialog>

namespace Ui {
class ConfirmDialog;
}

namespace gams {
namespace studio {

class ConfirmDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfirmDialog(QString title, QString text, QString checkText, QWidget *parent = nullptr);
    ~ConfirmDialog();
    void setBoxAccepted(bool accept);

signals:
    void autoConfirm();
    void setAcceptBox(bool accept);

private slots:
    void on_checkBox_stateChanged(int state);
    void on_buttonAlwaysOk_clicked();

private:
    Ui::ConfirmDialog *ui;
};

}
}

#endif // CONFIRMDIALOG_H
