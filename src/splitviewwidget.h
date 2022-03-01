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

private slots:
    void splitterMoved(int pos, int index);

    void onSwitchOrientation();
    void onSyncScroll(bool checked);
    void onClose();

private:
    Ui::SplitViewWidget *ui;
    QAction *mActOrient;
    QAction *mActSync;
    QAction *mActClose;
    QSplitter *mSplitter = nullptr;
    QWidget *mWidget = nullptr;
    QSize mPrefSize;
};


} // namespace split
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_SPLITVIEWWIDGET_H
