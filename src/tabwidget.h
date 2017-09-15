/*
 * This file is part of the GAMS IDE project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QtWidgets>
#include "codeeditor.h"

namespace gams {
namespace ide {

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = nullptr);
    ~TabWidget();

    int addTab(QWidget *page, const QString &label, int fileId = -1);
    int addTab(QWidget *page, const QIcon &icon, const QString &label, int fileId = -1);

signals:
    void fileActivated(int fileId, CodeEditor* edit);

public slots:
    void tabNameChanged(int fileId, QString newName);

private slots:
    void onTabChanged(int index);

private:
    QHash<int, int> mFileId2TabIndex;
};

} // namespace ide
} // namespace gams

#endif // TABWIDGET_H
