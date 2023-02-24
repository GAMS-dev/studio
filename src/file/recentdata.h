/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_RECENTDATA_H
#define GAMS_STUDIO_RECENTDATA_H

#include <QWidget>
#include "pexgroupnode.h"

namespace gams {
namespace studio {

class MainWindow;

class RecentData
{
public:
    RecentData();
    void init(MainWindow *mainWindow);

    void setEditor(FileMeta *fileMeta, QWidget* edit);
    void removeEditor(QWidget* edit);

    FileMeta* fileMeta() const;
    QWidget* editor() const;
    PExProjectNode* project(bool skipGamsSystem = true) const;
    FileId editFileId() const { return mEditFileId; }
    QString path() const { return mPath; }
    QWidget* persistentEditor() const;

private:
    void purgeEditor(QWidget *edit);

private:
    MainWindow *mMainWindow = nullptr;
    QVector<QWidget*> mEditList;
    QVector<FileMeta*> mMetaList;
    FileId mEditFileId;
    QString mPath;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_RECENTDATA_H
