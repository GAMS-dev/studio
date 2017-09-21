#include "tabwidget.h"
#include "codeeditor.h"
#include "exception.h"

namespace gams {
namespace ide {

TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent)
{
}

TabWidget::~TabWidget()
{}

int TabWidget::addTab(QWidget* page, const QString& label, int fileId)
{
    if (mFileId2TabIndex.contains(fileId))
        throw FATAL() << "error: TabWidget::addTab already contains fileId";
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

} // namespace ide
} // namespace gams
