/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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

    void setEditor(QWidget* edit);
    void removeEditor(QWidget* edit);

    QWidget* editor() const;
    PExProjectNode* project() const;
    FileId editFileId() const { return mEditFileId; }
    QString path() const { return mPath; }
    QWidget* persistentEditor() const;

private:
    MainWindow *mMainWindow = nullptr;
    QVector<QWidget*> mEditList;
    FileId mEditFileId;
    QString mPath;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_RECENTDATA_H
