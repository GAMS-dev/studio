/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include <QWidget>
#include <QMultiMap>
#include <QUrl>
#include <QLabel>
#include <QWebEngineView>

#include "helpdata.h"


class QMenu;
class QToolButton;

namespace gams {
namespace studio {

namespace support {
class CheckForUpdate;
}

namespace help {

namespace Ui {
class HelpWidget;
}

class HelpToolBar;
class HelpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HelpWidget(QWidget *parent = nullptr);
    ~HelpWidget();

    QMap<QString, QString> getBookmarkMap() const;
    void setBookmarkMap(const QMap<QString, QString> &value);
    void clearStatusBar();

public slots:
    void on_urlOpened(const QUrl& location);
    void on_helpContentRequested(const gams::studio::help::DocumentType &type, const QString& keyword, const QString &submoduleName = "");
    void on_bookmarkNameUpdated(const QString& location, const QString& name);
    void on_bookmarkLocationUpdated(const QString& oldLocation, const QString& newLocation, const QString& name);
    void on_bookmarkRemoved(const QString& location, const QString& name);

    void on_loadFinished(bool ok);
    void linkHovered(const QString& url);

    void on_actionHome_triggered();
    void on_actionAddBookmark_triggered();
    void on_actionOrganizeBookmark_triggered();
    void on_bookmarkaction();

    void on_actionOnlineHelp_triggered(bool checked);
    void on_actionOpenInBrowser_triggered();
    void on_actionCopyPageURL_triggered();

    void addBookmarkAction(const QString& objectName, const QString& title);

    void on_searchHelp();
    void on_backButtonTriggered();
    void on_forwardButtonTriggered();
    void on_closeButtonTriggered();
    void on_caseSensitivityToggled(bool checked);
    void searchText(const QString& text);
    void copySelection();

    void zoomIn();
    void zoomOut();
    void resetZoom();

    void setZoomFactor(qreal factor);
    qreal getZoomFactor();

    QWebEngineView *createHelpView();
    void on_webActionTriggered(QWebEnginePage::WebAction webAction, bool checked);

signals:
    void webActionEnabledChanged(QWebEnginePage::WebAction webAction, bool enabled);

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent* event);

private:
    void setupToolbar(QToolButton* bookmarkToolButton, QToolButton* helpToolButton);

private:
    Ui::HelpWidget *ui;

    QMap<QString, QString> mBookmarkMap;
    QMenu* mBookmarkMenu;
    QLabel mStatusBarLabel;
    QUrl onlineStartPageUrl;

    QUrl getStartPageUrl();
    QUrl getOnlineStartPageUrl();
    bool isDocumentAvailable(const QString& path, const QString& chapter);
    QString getCurrentReleaseVersion();

    void getErrorHTMLText(QString& htmlText, const QUrl& url);
    enum SearchDirection {
        Forward = 0,
        Backward = 1
    };
    void findText(const QString &text, SearchDirection direction, bool caseSensitivity);
    void createWebActionTrigger(QWebEnginePage *page, QWebEnginePage::WebAction, const QIcon &icon);

    QScopedPointer<support::CheckForUpdate> mC4U;
};

} // namespace help
} // namespace studio
} // namespace gams

#endif // HELPWIDGET_H
