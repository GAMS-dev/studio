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
#include "tabwidget.h"
#include "codeeditor.h"

namespace gams {
namespace ide {

TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent)
{
    connect(this, &TabWidget::currentChanged, this, &TabWidget::onTabChanged);
}

TabWidget::~TabWidget()
{}

int TabWidget::addTab(QWidget* page, const QString& label, int fileId)
{
    if (mFileId2TabIndex.contains(fileId))
        throw std::runtime_error("error: TabWidget::addTab already contains fileId");
    int index = QTabWidget::addTab(page, label);
    if (fileId >= 0)
        mFileId2TabIndex.insert(fileId, index);
    return index;
}

int TabWidget::addTab(QWidget* page, const QIcon& icon, const QString& label, int fileId)
{
    if (mFileId2TabIndex.contains(fileId))
        throw std::runtime_error("error: TabWidget::addTab already contains fileId");
    int index = QTabWidget::addTab(page, icon, label);
    if (fileId >= 0)
        mFileId2TabIndex.insert(fileId, index);
    return index;
}

void TabWidget::tabNameChanged(int fileId, QString newName)
{
    int index = mFileId2TabIndex.value(fileId, -1);
    if (index < 0)
        throw std::runtime_error("error: TabWidget::tabNameChanged fileId not registered");
    setTabText(index, newName);
}

void TabWidget::onTabChanged(int index)
{
    CodeEditor *edit = qobject_cast<CodeEditor*>(widget(index));
    if (!edit) return;
    for (int fileId: mFileId2TabIndex.keys()) {
        if (mFileId2TabIndex.value(fileId) == index) {
            emit fileActivated(fileId, edit);
            break;
        }
    }
}

} // namespace ide
} // namespace gams
