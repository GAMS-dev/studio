/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
    RecentData() { reset(); }

    void reset();
    void setEditor(QWidget* edit, MainWindow* window);

    /**
     * @brief Name of the main model.
     * @remark Call <c>hasValidProject()</c> before.
     */
    bool hasValidProject();

    QWidget* editor() const { return mEditor; }
    PExGroupNode* group() const {return mGroup; }
    FileId editFileId() const { return mEditFileId; }
    QString path() const { return mPath; }

private:
    QWidget* mEditor;
    FileId mEditFileId;
    PExGroupNode* mGroup;
    QString mPath;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_RECENTDATA_H
