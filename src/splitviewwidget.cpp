#include "splitviewwidget.h"
#include "ui_splitviewwidget.h"
#include "theme.h"
#include "exception.h"
#include "logger.h"
#include "settings.h"

#include <QTimer>

namespace gams {
namespace studio {
namespace split {

SplitViewWidget::SplitViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SplitViewWidget)
{
    ui->setupUi(this);
    mSplitter = qobject_cast<QSplitter*>(parent);
    if (!mSplitter)
        FATAL() << "SplitViewWidget needs to be child of a QSplitter";
    connect(mSplitter, &QSplitter::splitterMoved, this, &SplitViewWidget::splitterMoved);
    mPrefSize = Settings::settings()->toSize(skSplitViewSize);
    if (mPrefSize == QSize(10,10))
        mPrefSize = QSize(mSplitter->width() / 2, mSplitter->height() / 2);
    ui->bClose->setIcon(Theme::icon(":/%1/remove"));
    ui->bSwitchOrientation->setIcon(Theme::icon(":/%1/split-h"));
    ui->bSyncScroll->setIcon(Theme::icon(":/%1/lock-open"));

    //TODO(JM) deactivated until fully defined
    ui->bSyncScroll->setVisible(false);
}

SplitViewWidget::~SplitViewWidget()
{
    delete ui;
}

void SplitViewWidget::setOrientation(Qt::Orientation orientation)
{
    bool visible = isVisible();
    if (!visible) setVisible(true);
    if (mSplitter->orientation() == orientation && visible) return;
    mSplitter->setOrientation(orientation);
    ui->bSwitchOrientation->setIcon(Theme::icon(mSplitter->orientation() == Qt::Horizontal ? ":/%1/split-h"
                                                                                           : ":/%1/split-v"));
    Settings::settings()->setInt(skSplitOrientation, orientation);
    QTimer::singleShot(0, this, [this, orientation](){
        int splitSize = qMax(50, orientation == Qt::Horizontal ? mPrefSize.width() : mPrefSize.height());
        int all = (orientation == Qt::Horizontal ? mSplitter->width() : mSplitter->height());
        mSplitter->setSizes({ all - mSplitter->handleWidth() - splitSize, splitSize });
    });
}

bool SplitViewWidget::setWidget(QWidget *widget, QString fileName)
{
    if (layout()->count() != 1)
        return false;
    widget->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::MinimumExpanding);
    layout()->addWidget(widget);
    mWidget = widget;
    ui->laFile->setText(fileName);
    return true;
}

void SplitViewWidget::removeWidget()
{
    if (mWidget) {
        layout()->removeWidget(mWidget);
        mWidget = nullptr;
    }
}

QWidget *SplitViewWidget::widget()
{
    return mWidget;
}

void SplitViewWidget::setScrollLocked(bool lock)
{
    emit scrollLocked(lock);
    ui->bSyncScroll->setIcon(Theme::icon(lock ? ":/%1/lock" : ":/%1/lock-open"));
}

bool SplitViewWidget::isScrollLocked()
{
    return ui->bSyncScroll->isChecked();
}

QSize SplitViewWidget::preferredSize()
{
    return mPrefSize;
}

void SplitViewWidget::showAndAdjust(Qt::Orientation orientation)
{
    setOrientation(orientation);
}

QList<int> SplitViewWidget::sizes()
{
    int splitSize = (mSplitter->orientation() == Qt::Horizontal) ? width() : height();
    return { mSplitter->width() - mSplitter->handleWidth() - splitSize, splitSize };
}

void SplitViewWidget::splitterMoved(int pos, int index)
{
    Q_UNUSED(pos)
    if (mSplitter->widget(index) != this) return;
    if (mSplitter->orientation() == Qt::Horizontal)
        mPrefSize.setWidth(mSplitter->sizes().at(index));
    else
        mPrefSize.setHeight(mSplitter->sizes().at(index));
    Settings::settings()->setSize(skSplitViewSize, preferredSize());
}

void SplitViewWidget::on_bSwitchOrientation_clicked()
{
    setOrientation(mSplitter->orientation() == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal);
}


void SplitViewWidget::on_bClose_clicked()
{
    setVisible(false);
    emit hidden();
}


void SplitViewWidget::on_bSyncScroll_toggled(bool checked)
{
    setScrollLocked(checked);
}

} // namespace split
} // namespace studio
} // namespace gams
