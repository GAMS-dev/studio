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
    ui->toolBar->setIconSize(QSize(16,16));

    mActOrient = new QAction(Theme::icon(":/%1/split-h"), "Split horizontally", this);
    connect(mActOrient, &QAction::triggered, this, &SplitViewWidget::onSwitchOrientation);
    ui->toolBar->addAction(mActOrient);

    mActSync = new QAction(Theme::icon(":/%1/lock-open"), "Synchronize scrolling", this);
    mActSync->setCheckable(true);
    connect(mActSync, &QAction::triggered, this, &SplitViewWidget::onSyncScroll);
    ui->toolBar->addAction(mActSync);
    mActSync->setChecked(Settings::settings()->toBool(skSplitScollLock));

    mActClose = new QAction(Theme::icon(":/%1/remove"), "Close split view", this);
    connect(mActClose, &QAction::triggered, this, &SplitViewWidget::onClose);
    ui->toolBar->addAction(mActClose);
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
    mActOrient->setIcon(Theme::icon(mSplitter->orientation() == Qt::Horizontal ? ":/%1/split-h" : ":/%1/split-v"));
    Settings::settings()->setInt(skSplitOrientation, orientation);
    QTimer::singleShot(0, this, [this, orientation](){
        int splitSize = qMax(50, orientation == Qt::Horizontal ? mPrefSize.width() : mPrefSize.height());
        int all = (orientation == Qt::Horizontal ? mSplitter->width() : mSplitter->height());
        mSplitter->setSizes({ all - mSplitter->handleWidth() - splitSize, splitSize });
    });
}

bool SplitViewWidget::setWidget(QWidget *widget)
{
    if (layout()->count() != 1)
        return false;
    widget->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::MinimumExpanding);
    layout()->addWidget(widget);
    mWidget = widget;
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

void SplitViewWidget::setFileName(const QString &fileName, const QString &filePath)
{
    ui->laFile->setText(fileName);
    ui->laFile->setToolTip(filePath);
}

void SplitViewWidget::setScrollLocked(bool lock)
{
    mActSync->setIcon(Theme::icon(lock ? ":/%1/lock" : ":/%1/lock-open"));
    Settings::settings()->setBool(skSplitScollLock, lock);
}

bool SplitViewWidget::isScrollLocked()
{
    return mActSync->isChecked();
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

void SplitViewWidget::onSwitchOrientation()
{
    setOrientation(mSplitter->orientation() == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal);
}

void SplitViewWidget::onSyncScroll(bool checked)
{
    setScrollLocked(checked);
}

void SplitViewWidget::onClose()
{
    setVisible(false);
    emit hidden();
}

} // namespace split
} // namespace studio
} // namespace gams
