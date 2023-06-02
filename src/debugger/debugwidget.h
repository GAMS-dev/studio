#ifndef GAMS_STUDIO_DEBUGGER_DEBUGWIDGET_H
#define GAMS_STUDIO_DEBUGGER_DEBUGWIDGET_H

#include <QWidget>

namespace gams {
namespace studio {
namespace debugger {

namespace Ui {
class DebugWidget;
}

class Server;

class DebugWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DebugWidget(QWidget *parent = nullptr);
    ~DebugWidget();

    void setText(const QString &text);
    void setDebugServer(Server *server);

signals:
    void sendRun();
    void sendStepLine();
    void sendInterrupt();

private slots:
    void on_tbRun_clicked();
    void on_tbStep_clicked();
    void on_tbStop_clicked();

private:
    Ui::DebugWidget *ui;
    Server *mServer = nullptr;
};


} // namespace debugger
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_DEBUGGER_DEBUGWIDGET_H
