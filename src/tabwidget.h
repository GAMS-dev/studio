#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QtWidgets>

namespace gams {
namespace ide {

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = nullptr);
    ~TabWidget();

    int addTab(QWidget *page, const QString &label, int fileId = -1);
    int addTab(QWidget *page, const QIcon &icon, const QString &label, int fileId = -1);

signals:

public slots:
    void tabNameChanged(int fileId, QString newName);

private:
    QHash<int, int> mFileId2TabIndex;
};

} // namespace ide
} // namespace gams

#endif // TABWIDGET_H
