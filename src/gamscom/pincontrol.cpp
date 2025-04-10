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
#include "pincontrol.h"
#include "pinviewwidget.h"
#include "file/pexgroupnode.h"
#include "file/filemeta.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"
#include "viewhelper.h"

namespace gams {
namespace studio {
namespace gamscom {

struct PinData {
    PinData();
    PinData(QWidget *w, FileMeta *fm) : widget(w), fileMeta(fm) {}
    QWidget* widget = nullptr;
    FileMeta* fileMeta = nullptr;
};

PinControl::PinControl(QObject *parent) : QObject(parent)
{}

void PinControl::init(pin::PinViewWidget *pinView)
{
    mPinView = pinView;
}

void PinControl::setPinView(PExProjectNode *project, QWidget *pinChild, FileMeta *fileMeta)
{
    if (pinChild && !fileMeta) {
        SysLogLocator::systemLog()->append("Inconsistent PinView data, assignement ignored.", LogMsgType::Warning);
        return;
    }
    mLastProject = project;
    PinData *data = new PinData(pinChild, fileMeta);
    mData.insert(project, data);
    connect(pinChild, &QWidget::destroyed, this, &PinControl::removeView);
    projectSwitched(project);
}

bool PinControl::hasPinChild(PExProjectNode *project) const
{
    return mData.contains(project);
}

void PinControl::closedPinView()
{
    mData.remove(mLastProject);
}

PinData *PinControl::pinChild(PExProjectNode *project) const
{
    if (mData.contains(project))
        return mData.value(project);
    return mData.value(nullptr);
}

bool PinControl::hasPinView()
{
    if (!mPinView)
        SysLogLocator::systemLog()->append("PinView not assigned", LogMsgType::Error);
    return mPinView;
}

void PinControl::projectSwitched(PExProjectNode *project)
{
    if (!hasPinView()) return;
    PinData *data = pinChild(project);
    if (!data || !data->widget || !data->fileMeta) {
        if (!data || mCurrentWidget != data->widget) {
            mPinView->setWidget(nullptr);
            mPinView->setVisible(false);
            mCurrentWidget = nullptr;
        }
    } else if (mCurrentWidget != data->widget) {
        QWidget *wid = data->widget;
        mPinView->setWidget(wid);
        ViewHelper::setModified(wid, data->fileMeta->isModified());
        project::ProjectEdit *pEd = ViewHelper::toProjectEdit(wid);
        QString tabName = pEd ? pEd->tabName(NameModifier::editState)
                              : data->fileMeta->name(NameModifier::editState);
        mPinView->setFileName(tabName, QDir::toNativeSeparators(data->fileMeta->location()));
        mPinView->setFontGroup(data->fileMeta->fontGroup());
        mPinView->showAndAdjust(mPinView->orientation());
        mCurrentWidget = data->widget;
    }
    mLastProject = project;
}

void PinControl::resetToInitialView() // TODO(JM) check if useful
{
    projectSwitched(nullptr);
    if (!mPinView) return;
    PinData *defaultChild = nullptr;
    for (auto ind = mData.begin() ; ind != mData.end() ; ++ind) {
        if (ind.key()) {
            PinData *data = ind.value();
            data->widget->deleteLater();
            delete data;
        }
        else
            defaultChild = ind.value();
    }
    mData.clear();
    mData.insert(nullptr, defaultChild);
}

void PinControl::removeView(QObject *editWidget)
{
    PExProjectNode *project = nullptr;
    PinData *data = nullptr;
    for (auto ind = mData.begin() ; ind != mData.end() ; ++ind) {
        if (ind.value() && ind.value()->widget == editWidget) {
            data = ind.value();
            project = ind.key();
        }
    }
    if (data) {
        if (project)
            mData.remove(project);
        else
            mData.insert(nullptr, nullptr);
        delete data;
    }
}

} // namespace debugger
} // namespace studio
} // namespace gams
