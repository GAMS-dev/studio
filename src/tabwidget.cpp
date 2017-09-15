#include "tabwidget.h"


namespace gams {
namespace ide {

TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent)
{
    connect(this, &TabWidget::currentChanged, this, &TabWidget::onTabChanged);
}

TabWidget::~TabWidget()
{}

int TabWidget::addTab(QWidget* page, const QString& label, int fileId)
{
    if (mFileId2TabIndex.contains(fileId))
        throw std::runtime_error("error: TabWidget::addTab already contains fileId");
    int index = QTabWidget::addTab(page, label);
    if (fileId >= 0)
        mFileId2TabIndex.insert(fileId, index);
    return index;
}

int TabWidget::addTab(QWidget* page, const QIcon& icon, const QString& label, int fileId)
{
    if (mFileId2TabIndex.contains(fileId))
        throw std::runtime_error("error: TabWidget::addTab already contains fileId");
    int index = QTabWidget::addTab(page, icon, label);
    if (fileId >= 0)
        mFileId2TabIndex.insert(fileId, index);
    return index;
}

void TabWidget::tabNameChanged(int fileId, QString newName)
{
    int index = mFileId2TabIndex.value(fileId, -1);
    if (index < 0)
        throw std::runtime_error("error: TabWidget::tabNameChanged fileId not registered");
    setTabText(index, newName);
}

void TabWidget::onTabChanged(int index)
{
    for (int fileId: mFileId2TabIndex.keys()) {
        if (mFileId2TabIndex.value(fileId) == index) {
            emit fileActivated(fileId);
            break;
        }
    }
}

} // namespace ide
} // namespace gams
