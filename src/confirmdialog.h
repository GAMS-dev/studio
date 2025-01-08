/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
    explicit ConfirmDialog(const QString &title,
                           const QString &text,
                           const QString &checkText,
                           QWidget *parent = nullptr);
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
