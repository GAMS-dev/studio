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
#ifndef GAMS_STUDIO_FILECHANGEDIALOG_H
#define GAMS_STUDIO_FILECHANGEDIALOG_H

#include <QMessageBox>
#include <QPushButton>
#include <QCheckBox>

namespace gams {
namespace studio {


class FileChangeDialog : public QMessageBox
{
    Q_OBJECT
public:
    enum class Result {
        rClose,
        rReload,
        rReloadAlways,
        rKeep,
        rCount
    };
    Q_ENUM(Result)

public:
    explicit FileChangeDialog(QWidget *parent = nullptr);
    ~FileChangeDialog() override;

    void show(const QString &filePath, bool deleted, bool modified, int count);
    bool isForAll();
    static Result enumResult(int result) { return Result(result % int(Result::rCount)); }
    static bool isForAll(int result) { return result >= int(Result::rCount); }
    static bool isAutoReload(int result) { return ((result+1) % int(Result::rCount)) == 3; }

signals:
    void ready(int result);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void buttonClicked();

private:
    QVector<QPushButton*> mButtons;
    QCheckBox *mCbAll;

};


} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_FILECHANGEDIALOG_H
