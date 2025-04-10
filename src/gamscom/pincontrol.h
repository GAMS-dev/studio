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
#ifndef GAMS_STUDIO_GAMSCOM_PINCONTROL_H
#define GAMS_STUDIO_GAMSCOM_PINCONTROL_H

#include "file/pexgroupnode.h"
#include <QHash>

namespace gams {
namespace studio {

namespace pin {
class PinViewWidget;
}

namespace gamscom {

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

private:
    pin::PinViewWidget *mPinView = nullptr;
    PExProjectNode *mLastProject = nullptr;
    QWidget *mCurrentWidget = nullptr;
    QHash<PExProjectNode*, PinData*> mData;
};

} // namespace debugger
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GAMSCOM_PINCONTROL_H
