/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#ifndef ENCODINGSDIALOG_H
#define ENCODINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SelectEncodings;
}

namespace gams {
namespace studio {

class SelectEncodings : public QDialog
{
    Q_OBJECT

public:
    explicit SelectEncodings(const QList<int> &selectedMibs, int defaultMib, QWidget *parent = nullptr);
    ~SelectEncodings() override;
    QList<int> selectedMibs();
    int defaultCodec();

private slots:

    void on_pbCancel_clicked();
    void on_pbSave_clicked();
    void on_pbReset_clicked();

protected:
    void showEvent(QShowEvent *e) override;

private:
    void centerCurrent();

private:
    Ui::SelectEncodings *ui;
    QList<int> mSelectedMibs;
    int mDefaultMib;
};


}
}

#endif // ENCODINGSDIALOG_H
