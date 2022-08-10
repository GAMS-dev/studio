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
    Ui::Navigator* ui;
};

}
}
