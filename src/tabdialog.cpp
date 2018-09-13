#include "tabdialog.h"
#include "ui_tabdialog.h"
#include "logger.h"
#include <QTimer>
#include <QScrollBar>

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
    ui->listView->setModel(mFilterModel);
    ui->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->listView->setMinimumHeight(1);
    connect(ui->listView, &QListView::activated, this, &TabDialog::selectTab);
    connect(ui->listView, &QListView::clicked, this, &TabDialog::selectTab);
    mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &TabDialog::setFilter);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &TabDialog::selectFirst);

//    mFilterModel->setFilterFixedString("");
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

void TabDialog::setFilter(const QString &filter)
{
    mFilterModel->setFilterWildcard(filter);
    resizeToContent();
}

void TabDialog::selectFirst()
{
    QModelIndex mi = mFilterModel->index(0,0);
    mTabModel->tabs()->setCurrentIndex(mFilterModel->mapToSource(mi).row());
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
    Q_UNUSED(parent);
    return mTabs->count();
}

QVariant TabListModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
        return mTabs->tabBar()->tabText(index.row());
    if (role == Qt::SizeHintRole) {
        QFont font = mTabs->font();
        if (index.row() == mTabs->currentIndex()) font.setBold(true);
        QFontMetrics fm = QFontMetrics(font);
        return QSize(fm.width(mTabs->tabText(index.row())), fm.height()+4);
    }
    if (role == Qt::FontRole) {
        QFont font = mTabs->font();
        if (index.row() == mTabs->currentIndex())
            font.setBold(true);
        return font;
    }
    if (role == Qt::BackgroundColorRole) {
        return mTabs->palette().color(mTabs->backgroundRole());
    }
    return  QVariant();
}

} // namespace studio
} // namespace gams
