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
#include "pinviewwidget.h"
#include "ui_pinviewwidget.h"
#include "theme.h"
#include "exception.h"
#include "logger.h"
#include "settings.h"

#include <QTimer>
#include <QMouseEvent>

namespace gams {
namespace studio {
namespace pin {

PinViewWidget::PinViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PinViewWidget)
{
    ui->setupUi(this);
    setVisible(false);
    mSplitter = qobject_cast<QSplitter*>(parent);
    if (!mSplitter)
        FATAL() << "PinViewWidget needs to be child of a QSplitter";
    connect(mSplitter, &QSplitter::splitterMoved, this, &PinViewWidget::splitterMoved);
    mPrefSize = Settings::settings()->toSize(skPinViewSize);
    if (mPrefSize == QSize(10,10))
        mPrefSize = QSize(mSplitter->width() / 2, mSplitter->height() / 2);
    ui->toolBar->setIconSize(QSize(16,16));

    mActOrient = new QAction(Theme::icon(":/%1/split-h"), "Pin below", this);
    connect(mActOrient, &QAction::triggered, this, &PinViewWidget::onSwitchOrientation);
    ui->toolBar->addAction(mActOrient);

    bool locked = Settings::settings()->toBool(skPinScollLock);
    mActSync = new QAction(Theme::icon(":/%1/lock-open"), "Synchronize scrolling", this);
    mActSync->setCheckable(true);
    connect(mActSync, &QAction::triggered, this, &PinViewWidget::onSyncScroll);
    ui->toolBar->addAction(mActSync);
    mActSync->setChecked(locked);
    setScrollLocked(locked);

    mActClose = new QAction(Theme::icon(":/%1/remove"), "Close split view", this);
    connect(mActClose, &QAction::triggered, this, &PinViewWidget::onClose);
    ui->toolBar->addAction(mActClose);
    ui->laFile->installEventFilter(this);
}

PinViewWidget::~PinViewWidget()
{
    delete ui;
}

void PinViewWidget::setOrientation(Qt::Orientation orientation)
{
    bool visible = isVisible();
    if (!visible && widget()) setVisible(true);
    if (mSplitter->orientation() == orientation && visible) return;
    mSplitter->setOrientation(orientation);
    mActOrient->setIcon(Theme::icon(mSplitter->orientation() == Qt::Horizontal ? ":/%1/split-h" : ":/%1/split-v"));
    mActOrient->setToolTip(mSplitter->orientation() == Qt::Horizontal ? "Pin below" : "Pin right");
    Settings::settings()->setInt(skPinOrientation, orientation);
    QTimer::singleShot(0, this, [this, orientation](){
        int splitSize = qMax(50, orientation == Qt::Horizontal ? mPrefSize.width() : mPrefSize.height());
        int all = (orientation == Qt::Horizontal ? mSplitter->width() : mSplitter->height());
        mSplitter->setSizes({ all - mSplitter->handleWidth() - splitSize, splitSize });
    });
}

Qt::Orientation PinViewWidget::orientation()
{
    return mSplitter->orientation();
}

bool PinViewWidget::setWidget(QWidget *widget)
{
    if (mWidget) {
        layout()->removeWidget(mWidget);
        mWidget->setParent(nullptr);
        mWidget = nullptr;
    }
    if (!widget) {
        return true;
    }
    widget->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::MinimumExpanding);
    layout()->addWidget(widget);
    mWidget = widget;
    return true;
}

QWidget *PinViewWidget::widget()
{
    return mWidget;
}

void PinViewWidget::setTabName(const QString &tabName)
{
    ui->laFile->setText(tabName);
}

void PinViewWidget::setFileName(const QString &tabName, const QString &filePath)
{
    ui->laFile->setText(tabName);
    ui->laFile->setToolTip(filePath);
}

void PinViewWidget::setFontGroup(FontGroup fontGroup)
{
    mActSync->setEnabled(fontGroup == FontGroup::fgText);
}

void PinViewWidget::setScrollLocked(bool lock)
{
    if (!mActSync->isEnabled()) return;
    if (mActSync->isChecked() != lock) mActSync->setChecked(lock);
    mActSync->setIcon(Theme::icon(lock ? ":/%1/lock" : ":/%1/lock-open"));
    Settings::settings()->setBool(skPinScollLock, lock);
}

bool PinViewWidget::isScrollLocked()
{
    return mActSync->isEnabled() && mActSync->isChecked();
}

QSize PinViewWidget::preferredSize()
{
    return mPrefSize;
}

void PinViewWidget::showAndAdjust(Qt::Orientation orientation)
{
    setOrientation(orientation);
}

QList<int> PinViewWidget::sizes()
{
    int splitSize = (mSplitter->orientation() == Qt::Horizontal) ? width() : height();
    return { mSplitter->width() - mSplitter->handleWidth() - splitSize, splitSize };
}

bool PinViewWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->laFile && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::MiddleButton) {
            onClose();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void PinViewWidget::splitterMoved(int pos, int index)
{
    Q_UNUSED(pos)
    if (!isVisible()) return;
    if (mSplitter->widget(index) != this) return;
    if (mSplitter->orientation() == Qt::Horizontal)
        mPrefSize.setWidth(mSplitter->sizes().at(index));
    else
        mPrefSize.setHeight(mSplitter->sizes().at(index));
    Settings::settings()->setSize(skPinViewSize, preferredSize());
}

void PinViewWidget::onSwitchOrientation()
{
    setOrientation(mSplitter->orientation() == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal);
}

void PinViewWidget::onSyncScroll(bool checked)
{
    setScrollLocked(checked);
}

void PinViewWidget::onClose()
{
    setVisible(false);
    emit hidden();
}

} // namespace split
} // namespace studio
} // namespace gams
