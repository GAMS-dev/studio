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

    QMultiMap<QString, QString> getBookmarkMap() const;
    void setBookmarkMap(const QMultiMap<QString, QString> &value);

public slots:
    void on_urlOpened(const QUrl& location);
    void on_bookmarkNameUpdated(const QString& location, const QString& name);
    void on_bookmarkLocationUpdated(const QString& oldLocation, const QString& newLocation, const QString& name);
    void on_bookmarkRemoved(const QString& location, const QString& name);

    void on_loadFinished(bool ok);
    void on_actionHome_triggered();
    void on_actionAddBookMark_triggered();
    void on_actionOrganizeBookMark_triggered();
    void on_actionBookMark_triggered();

    void on_actionOnlineHelp_triggered(bool checked);
    void on_actionOpenInBrowser_triggered();

    void addBookmarkAction(const QString& objectName, const QString& title);

private:
    QMultiMap<QString, QString> bookmarkMap;
    QMenu* bookmarkMenu;

    QAction* actionAddBookmark;
    QAction* actionOrganizeBookmark;
    QAction* actionOnlineHelp;
    QAction* actionOpenInBrowser;

    QWebEngineView* helpView;

    QUrl helpStartPage;
    QDir defaultLocalHelpDir;
    QString defaultOnlineHelpLocation;
};

} // namespace studio
} // namespace gams

#endif // HELPVIEW_H
