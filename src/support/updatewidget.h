#ifndef UPDATEWIDGET_H
#define UPDATEWIDGET_H

#include <QWidget>

namespace gams {
namespace studio {
namespace support {

namespace Ui {
class UpdateWidget;
}

class UpdateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UpdateWidget(QWidget *parent = nullptr);
    ~UpdateWidget();

    void setText(const QString &text);

signals:
    void openSettings();

private slots:
    void remindMeLater();

private:
    Ui::UpdateWidget *ui;
};

}
}
}

#endif // UPDATEWIDGET_H
