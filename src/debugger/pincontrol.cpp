#include "pincontrol.h"
#include "pinviewwidget.h"
#include "file/pexgroupnode.h"
#include "file/filemeta.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"
#include "viewhelper.h"

namespace gams {
namespace studio {
namespace debugger {

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
    debugData("set");
    projectSwitched(project);
}

bool PinControl::hasPinChild(PExProjectNode *project) const
{
    return mData.contains(project);
}

void PinControl::closedPinView()
{
    mData.remove(mLastProject);
    debugData("removed");
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

void PinControl::debugData(QString text)
{
    DEB() << "---- Project " << text << ":  " << mData.count() << "  " << mLastProject;
    PExProjectNode *curPro = mData.contains(mLastProject) ? mLastProject : nullptr;
    for (auto ind = mData.begin() ; ind != mData.end() ; ++ind) {
        DEB() << ind.key() << ":  " << (ind.value() && ind.value()->fileMeta ? ind.value()->fileMeta->name() : "none")
              << (ind.key() == curPro ? "   <CURRENT>" : "");
    }
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
    debugData("switched");
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
