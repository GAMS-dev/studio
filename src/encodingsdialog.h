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
    explicit SelectEncodings(const QStringList &selectedEncodings, const QString &defaultEncoding, QWidget *parent = nullptr);
    ~SelectEncodings() override;
    QStringList selectedEncodings();
    QString defaultEncoding();
    static const QString CDefaultEncodingSelection;
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
    QStringList mEncodingsBackup;
    QString mDefaultEncoding;
};


}
}

#endif // ENCODINGSDIALOG_H
