#ifndef HELPVIEW_H
#define HELPVIEW_H

#include <QWidget>

namespace Ui {
class HelpView;
}

namespace gams {
namespace studio {

class HelpView : public QWidget
{
    Q_OBJECT

public:
    explicit HelpView(QWidget *parent = 0);
    ~HelpView();

private:
    Ui::HelpView *ui;
};

} // namespace studio
} // namespace gams

#endif // HELPVIEW_H
