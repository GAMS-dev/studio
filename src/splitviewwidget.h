#ifndef GAMS_STUDIO_SPLITVIEWWIDGET_H
#define GAMS_STUDIO_SPLITVIEWWIDGET_H

#include <QWidget>
#include <QSplitter>

namespace gams {
namespace studio {
namespace split {

namespace Ui {
class SplitViewWidget;
}

class SplitViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SplitViewWidget(QWidget *parent = nullptr);
    ~SplitViewWidget() override;
    void setOrientation(Qt::Orientation orientation);
    bool setWidget(QWidget *widget);
    void removeWidget();
    QWidget *widget();
    void setFileName(const QString &fileName, const QString &filePath);
    void setScrollLocked(bool lock);
    bool isScrollLocked();
    QSize preferredSize();
    void showAndAdjust(Qt::Orientation orientation);
    QList<int> sizes();

signals:
    void hidden();
    void scrollLocked(bool lock);

private slots:
    void splitterMoved(int pos, int index);

    void on_bSwitchOrientation_clicked();
    void on_bClose_clicked();
    void on_bSyncScroll_toggled(bool checked);

private:
    Ui::SplitViewWidget *ui;
    QSplitter *mSplitter = nullptr;
    QWidget *mWidget = nullptr;
    QSize mPrefSize;
};


} // namespace split
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_SPLITVIEWWIDGET_H
