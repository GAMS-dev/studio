#ifndef HELPVIEW_H
#define HELPVIEW_H

#include <QtWidgets>
#include <QWebEngineView>

namespace gams {
namespace studio {

class HelpView : public QDockWidget
{
    Q_OBJECT
public:
    HelpView(QWidget *parent = nullptr);
    ~HelpView();

    void setupUi(QWidget* parent);
    void load(QUrl locaiton);

private slots:
    void on_loadFinished(bool ok);
    void on_actionHome_triggered();
    void on_actionBack_triggered();
    void on_actionNext_triggered();
    void on_actionOnlineHelp_triggered(bool checked);
    void on_actionOpenInBrowser_triggered();

private:
    QAction* actionHome;
    QAction* actionBack;
    QAction* actionNext;

    QAction* actionOnlineHelp;
    QAction* actionOpenInBrowser;

    QWebEngineView* helpView;
    QUrl helpLocation;
    QDir defaultLocalHelpDir;
    QString defaultOnlineHelpLocation;
};

} // namespace studio
} // namespace gams

#endif // HELPVIEW_H
