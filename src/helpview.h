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
    HelpView(QWidget* parent = nullptr);
    ~HelpView();

    void setupUi(QWidget *parent);

public slots:
    void on_urlOpened(const QUrl& location);
    void on_bookmarkRemoved(const QUrl& location);

    void on_loadFinished(bool ok);
    void on_actionHome_triggered();
    void on_actionAddBookMark_triggered();
    void on_actionOrganizeBookMark_triggered();
    void on_actionBookMark_triggered();

    void on_actionOnlineHelp_triggered(bool checked);
    void on_actionOpenInBrowser_triggered();

private:
    QMap<QString, QString> bookmarkMap;
    QMenu* bookmarkMenu;

    QAction* actionAddBookmark;
    QAction* actionOrganizeBookmark;
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
