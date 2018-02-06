#ifndef HELPVIEW_H
#define HELPVIEW_H

#include <QtWidgets>

namespace gams {
namespace studio {

class HelpView : public QDockWidget
{
    Q_OBJECT
public:
    HelpView(QWidget *parent = nullptr);
    ~HelpView();

    void setupUi(QWidget* parent);

private slots:
    void on_actionHome_triggered();
    void on_actionBack_triggered();
    void on_actionNext_triggered();

public:
    QAction* actionHome;
    QAction* actionBack;
    QAction* actionNext;
};

} // namespace studio
} // namespace gams

#endif // HELPVIEW_H
