/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "tabdialog.h"
#include "ui_tabdialog.h"
#include "viewhelper.h"

#include <QTimer>
#include <QScrollBar>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QTabWidget>

namespace gams {
namespace studio {

TabDialog::TabDialog(QTabWidget *tabs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TabDialog),
    mTabModel(new TabListModel(tabs)),
    mFilterModel(new QSortFilterProxyModel(this))
{
    this->setWindowFlags(Qt::Popup);
    ui->setupUi(this);
    if (tabs->tabPosition() == QTabWidget::South) {
        layout()->removeWidget(ui->lineEdit);
        layout()->addWidget(ui->lineEdit);
    }
    mFilterModel->setSourceModel(mTabModel);
    mFilterModel->sort(0);
    mFilterModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui->listView->setModel(mFilterModel);
    ui->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->listView->setMinimumHeight(1);
    connect(ui->listView, &QListView::activated, this, &TabDialog::selectTab);
    connect(ui->listView, &QListView::clicked, this, &TabDialog::selectTab);
    mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &TabDialog::setFilter);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &TabDialog::returnPressed);
    ui->lineEdit->installEventFilter(this);
}

TabDialog::~TabDialog()
{
    mTabModel->deleteLater();
    delete ui;
}

void TabDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    resizeToContent();
    ui->listView->setCurrentIndex(mFilterModel->index(0, 0));
    ui->lineEdit->setFocus();
}

void TabDialog::resizeToContent()
{
    QTabWidget *tabs = mTabModel->tabs();
    int wid = 0;
    int hei = height() - ui->listView->contentsRect().height();
    for (int i = 0; i < mTabModel->rowCount(); ++i) {
        QModelIndex mi = mTabModel->index(i,0);
        QSize size = mTabModel->data(mi, Qt::SizeHintRole).toSize();
        if (size.isValid() && size.width() > wid) wid = size.width();
        if (mFilterModel->mapFromSource(mi).isValid() && mTabModel->tabs()->currentWidget()
                && hei+size.height() <= mTabModel->tabs()->currentWidget()->height())
            hei += size.height();
    }
    if (wid < 20) wid = 20;
    if (wid > 400) wid = 400;
    wid += width() - ui->listView->contentsRect().width() + ui->listView->verticalScrollBar()->width()+12;
    QPoint pos;
    if (tabs->tabPosition() == QTabWidget::North) {
        pos = tabs->mapToGlobal(tabs->rect().topRight());
        pos.ry() += tabs->tabBar()->height();
    } else {
        pos = tabs->mapToGlobal(tabs->rect().bottomRight());
        pos.ry() -= tabs->tabBar()->height();
    }
    int posY = (mTabModel->tabs()->tabPosition() == QTabWidget::North) ? pos.y() : pos.y()-hei;
    setGeometry(pos.x()-wid, posY, wid, hei);
    QModelIndex idx = mFilterModel->mapFromSource(mTabModel->index(mTabModel->tabs()->currentIndex()));
    if (idx.isValid())
        ui->listView->scrollTo(idx);
}

void TabDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Down) {
        int pos = ui->listView->currentIndex().row()+1;
        if (pos >= ui->listView->model()->rowCount()) pos = 0;
        ui->listView->setCurrentIndex(mFilterModel->index(pos, 0));
    } else if (e->key() == Qt::Key_Up) {
        int pos = ui->listView->currentIndex().row()-1;
        if (pos < 0) pos = ui->listView->model()->rowCount()-1;
        ui->listView->setCurrentIndex(mFilterModel->index(pos, 0));
    } else {
        QDialog::keyPressEvent(e);
    }
}

bool TabDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != ui->lineEdit) return false;
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
            keyPressEvent(keyEvent);
            return true;
        }
    }
    return false;
}

void TabDialog::setFilter(const QString &filter)
{
    mFilterModel->setFilterWildcard(filter);
    resizeToContent();
}

void TabDialog::returnPressed()
{
    mTabModel->tabs()->setCurrentIndex(mFilterModel->mapToSource(ui->listView->currentIndex()).row());
    close();
}

void TabDialog::selectTab(const QModelIndex &index)
{
    mTabModel->tabs()->setCurrentIndex(mFilterModel->mapToSource(index).row());
    close();
}

TabListModel::TabListModel(QTabWidget *tabs) : mTabs(tabs)
{
    if (mTabs->cornerWidget()->isEnabled())
        mTabs->cornerWidget()->setEnabled(false);
}

TabListModel::~TabListModel()
{
    if (!mTabs->cornerWidget()->isEnabled())
        mTabs->cornerWidget()->setEnabled(true);
}

int TabListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mTabs->count();
}

QVariant TabListModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        QString location = nameAppendix(index);
        return modName(index) + (!location.isEmpty() ? " [" + location + "]" : "");
    }
    if (role == Qt::SizeHintRole) {
        QFont font = mTabs->font();
        font.setBold(true); // cut-off-text workaround: always bold for calculating width
        QString location = nameAppendix(index);
        QFontMetrics fm = QFontMetrics(font);
        return QSize(fm.horizontalAdvance(modName(index) + " [" + location + "]"), fm.height()+4);
    }
    if (role == Qt::FontRole) {
        QFont font = mTabs->font();
        if (index.row() == mTabs->currentIndex())
            font.setBold(true);
        return font;
    }
    if (role == Qt::BackgroundRole) {
        return mTabs->palette().color(mTabs->backgroundRole());
    }
    return  QVariant();
}

QString TabListModel::modName(const QModelIndex &index) const
{
    QString mod = ViewHelper::modified(mTabs->widget(index.row())) ? "*" : " ";
    return mod + mTabs->tabBar()->tabText(index.row());
}

QString TabListModel::nameAppendix(const QModelIndex &index) const
{
    QString appendix = ViewHelper::location(mTabs->widget(index.row()));
    QFileInfo fi(appendix);
    appendix = fi.absoluteDir().dirName();
    if (appendix == ".") appendix = "";
    return appendix;
}

} // namespace studio
} // namespace gams
