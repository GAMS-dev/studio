#ifndef GAMS_STUDIO_PINVIEWWIDGET_H
#define GAMS_STUDIO_PINVIEWWIDGET_H

#include <QWidget>
#include <QSplitter>

namespace gams {
namespace studio {
namespace pin {

namespace Ui {
class PinViewWidget;
}

class PinViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PinViewWidget(QWidget *parent = nullptr);
    ~PinViewWidget() override;
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
    Ui::PinViewWidget *ui;
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
#endif // GAMS_STUDIO_PINVIEWWIDGET_H
