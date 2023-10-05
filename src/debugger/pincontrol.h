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
 */
#ifndef GAMS_STUDIO_DEBUGGER_PINCONTROL_H
#define GAMS_STUDIO_DEBUGGER_PINCONTROL_H

#include "file/pexgroupnode.h"
#include <QHash>

namespace gams {
namespace studio {

namespace pin {
class PinViewWidget;
}

namespace debugger {

struct PinData;

class PinControl: public QObject
{
    Q_OBJECT
public:
    PinControl(QObject *parent = nullptr);
    void init(gams::studio::pin::PinViewWidget *pinView);
    void closedPinView();
    void setPinView(PExProjectNode *project, QWidget *pinChild, FileMeta *fileMeta);
    bool hasPinChild(PExProjectNode *project) const;
    void projectSwitched(PExProjectNode *project);
    void resetToInitialView();

private slots:
    void removeView(QObject *editWidget);

private:
    PinData *pinChild(PExProjectNode *project) const;
    bool hasPinView();
    void debugData(QString text);

private:
    pin::PinViewWidget *mPinView = nullptr;
    PExProjectNode *mLastProject = nullptr;
    QWidget *mCurrentWidget = nullptr;
    QHash<PExProjectNode*, PinData*> mData;
};

} // namespace debugger
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_DEBUGGER_PINCONTROL_H
