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
#ifndef GAMS_STUDIO_PATHSELECT_H
#define GAMS_STUDIO_PATHSELECT_H

#include <QDialog>
#include <QLineEdit>

namespace gams {
namespace studio {
namespace pathselect {

namespace Ui {
class PathSelect;
}

class PathSelect : public QDialog
{
    Q_OBJECT

public:
    explicit PathSelect(QWidget *parent = nullptr);
    ~PathSelect() override;

signals:
    void workDirSelected(const QString &workDir);
    void canceled();

private slots:
    void on_edWorkDir_textChanged(const QString &text);
    void on_bWorkDir_clicked();
    void on_pbOk_clicked();

    void on_pbCancel_clicked();

private:
    void showDirDialog(const QString &title, QLineEdit *lineEdit, const QString &defaultDir);

private:
    Ui::PathSelect *ui;
};

} // namespace pathselect
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_PATHSELECT_H
