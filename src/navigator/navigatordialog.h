#include "ui_navigator.h"
#include <QDialog>

namespace gams {
namespace studio {

class NavigatorDialog : public QDialog {
    Q_OBJECT

public:
    NavigatorDialog(QWidget* parent = nullptr);
    ~NavigatorDialog();

private:
    void keyPressEvent(QKeyEvent *e) override;

private:
    Ui::Navigator* ui;
};

}
}
