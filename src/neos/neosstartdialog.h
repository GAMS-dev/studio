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
#ifndef GAMS_STUDIO_NEOSSTARTDIALOG_H
#define GAMS_STUDIO_NEOSSTARTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>

class QLabel;
class QCheckBox;

namespace gams {
namespace studio {
namespace neos {

class NeosProcess;

namespace Ui {
class NeosStartDialog;
}

class NeosStartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NeosStartDialog(const QString &eMail, QWidget *parent = nullptr);
    ~NeosStartDialog() override;
    void setProcess(neos::NeosProcess *proc);

signals:
    void noDialogFlagChanged(bool noDialog);
    void eMailChanged(const QString &eMail);

private slots:
    void buttonClicked(QAbstractButton *button);
    void updateCanStart();
    void updateValues();

protected:
    void showEvent(QShowEvent *event) override;

private:
    QString validateEmail(const QString &eMail);

private:
    Ui::NeosStartDialog *ui;
    NeosProcess *mProc = nullptr;
    bool mFirstShow = true;

};

} // namespace neos
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_NEOSSTARTDIALOG_H
