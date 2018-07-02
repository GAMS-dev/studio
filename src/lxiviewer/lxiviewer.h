/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_LXIVIEWER_LXIVIEWER_H
#define GAMS_STUDIO_LXIVIEWER_LXIVIEWER_H

#include <QWidget>
#include <QModelIndex>

namespace gams {
namespace studio {

class CodeEdit;
class ProjectFileNode;

namespace lxiviewer {

namespace Ui {
class LxiViewer;
}

class LxiViewer : public QWidget
{
    Q_OBJECT

public:
    explicit LxiViewer(CodeEdit *codeEditor, ProjectFileNode *fc, QWidget *parent);
    ~LxiViewer();

    CodeEdit *codeEdit() const;

private slots:
    void loadLxiFile();
    void loadLstFile();
    void jumpToTreeItem();
    void jumpToLine(QModelIndex modelIndex);

private:
    Ui::LxiViewer *ui;
    CodeEdit* mCodeEdit;
    ProjectFileNode *mFileNode;
    QString mLstFile;
    QString mLxiFile;
};


} // namespace lxiviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_LXIVIEWER_LXIVIEWER_H
