#include <QWebEngineHistory>

#include "gamspaths.h"
#include "helpview.h"
#include "bookmarkdialog.h"

namespace gams {
namespace studio {

HelpView::HelpView(QWidget *parent) :
    QDockWidget(parent)
{
    defaultLocalHelpDir = QDir( QDir( GAMSPaths::systemDir() ).filePath("docs") );
    QDir dir = defaultLocalHelpDir.filePath("index.html");
    helpStartPage = QUrl::fromLocalFile(dir.canonicalPath());

    defaultOnlineHelpLocation = "www.gams.com/latest/docs";
    setupUi(parent);
}

HelpView::~HelpView()
{
}

void HelpView::setupUi(QWidget *parent)
{
    this->setObjectName(QStringLiteral("dockHelpView"));
    this->setEnabled(true);
    this->setFloating(true);
    this->setAllowedAreas(Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea);
    this->setWindowTitle("Help");
    this->resize(QSize(950, 600));
    this->move(QPoint(200, 200));

    QWidget* helpWidget = new QWidget(parent);
    QVBoxLayout* helpVLayout = new QVBoxLayout(helpWidget);
    helpVLayout->setObjectName(QStringLiteral("helpVLayout"));
    helpVLayout->setContentsMargins(0, 0, 0, 0);
    helpWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    helpView = new QWebEngineView(this);
    helpView->load(helpStartPage);
    connect(helpView, &QWebEngineView::loadFinished, this, &HelpView::on_loadFinished);

    QToolBar* toolbar = new QToolBar(this);

    QAction* actionHome = new QAction(this);
    actionHome->setObjectName(QStringLiteral("actionHome"));
    actionHome->setToolTip("Start page");
    actionHome->setStatusTip(tr("Start page"));
    QPixmap homePixmap(":/img/home");
    QIcon homeButtonIcon(homePixmap);
    actionHome->setIcon(homeButtonIcon);

    toolbar->addAction(actionHome);
    toolbar->addSeparator();
    toolbar->addAction(helpView->pageAction(QWebEnginePage::Back));
    toolbar->addAction(helpView->pageAction(QWebEnginePage::Forward));
    toolbar->addSeparator();
    toolbar->addAction(helpView->pageAction(QWebEnginePage::Reload));
    toolbar->addSeparator();
    toolbar->addAction(helpView->pageAction(QWebEnginePage::Stop));
    toolbar->addSeparator();

    actionAddBookmark = new QAction(this);
    actionAddBookmark->setObjectName(QStringLiteral("actionAddBookmark"));
    actionAddBookmark->setText(QLatin1String("Bookmark This Page"));
    actionAddBookmark->setToolTip(tr("Bookmark This Page"));
    actionAddBookmark->setStatusTip(tr("Bookmark This Page"));
    connect(actionAddBookmark, &QAction::triggered, this, &HelpView::on_actionAddBookMark_triggered);

    actionOrganizeBookmark = new QAction(this);
    actionOrganizeBookmark->setObjectName(QStringLiteral("actionOrganizeBookmark"));
    actionOrganizeBookmark->setText(QLatin1String("Organize Bookmarks"));
    actionOrganizeBookmark->setToolTip(tr("Organize Bookmarks"));
    actionOrganizeBookmark->setStatusTip(tr("Organize Bookmarks"));
    connect(actionOrganizeBookmark, &QAction::triggered, this, &HelpView::on_actionOrganizeBookMark_triggered);

    bookmarkMenu = new QMenu(this);
    bookmarkMenu->addAction(actionAddBookmark);
    bookmarkMenu->addSeparator();
    bookmarkMenu->addAction(actionOrganizeBookmark);

    QToolButton* bookmarkToolButton = new QToolButton(this);
    QPixmap bookmarkPixmap(":/img/bookmark");
    QIcon bookmarkButtonIcon(bookmarkPixmap);
    bookmarkToolButton->setToolTip("Bookmarks");
    bookmarkToolButton->setIcon(bookmarkButtonIcon);
    bookmarkToolButton->setIcon(bookmarkButtonIcon);
    bookmarkToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    bookmarkToolButton->setMenu(bookmarkMenu);

    toolbar->addWidget(bookmarkToolButton);

    QWidget* spacerWidget = new QWidget();
    spacerWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    toolbar->addWidget(spacerWidget);

    QMenu* helpMenu = new QMenu;
    actionOnlineHelp = new QAction(this);
    actionOnlineHelp->setObjectName(QStringLiteral("actionOnlineHelp"));
    actionOnlineHelp->setText("View lastest online version of this page");
    actionOnlineHelp->setToolTip("View lastest online version of this page");
    actionOnlineHelp->setStatusTip(tr("View lastest online version of this page"));
    actionOnlineHelp->setCheckable(true);
    helpMenu->addAction(actionOnlineHelp);
    helpMenu->addSeparator();

    actionOpenInBrowser = new QAction(this);
    actionOpenInBrowser->setObjectName(QStringLiteral("actionOpenInBrowser"));
    actionOpenInBrowser->setText("Open in Default Web Browser");
    actionOpenInBrowser->setToolTip("Open in Default Web Browser");
    actionOpenInBrowser->setStatusTip(tr("Open in Default Web Browser"));
    helpMenu->addAction(actionOpenInBrowser);

    QToolButton* helpToolButton = new QToolButton(this);
    QPixmap toolPixmap(":/img/config");
    QIcon toolButtonIcon(toolPixmap);
    helpToolButton->setToolTip("Help Option");
    helpToolButton->setIcon(toolButtonIcon);
    helpToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    helpToolButton->setMenu(helpMenu);
    toolbar->addWidget(helpToolButton);

    helpVLayout->addWidget( toolbar );
    helpVLayout->addWidget( helpView );

    this->setWidget( helpWidget );

    connect(actionHome, &QAction::triggered, this, &HelpView::on_actionHome_triggered);
    connect(actionOnlineHelp, &QAction::triggered, this, &HelpView::on_actionOnlineHelp_triggered);
    connect(actionOpenInBrowser, &QAction::triggered, this, &HelpView::on_actionOpenInBrowser_triggered);
}

void HelpView::on_urlOpened(const QUrl& location)
{
    helpView->load(location);
}

void HelpView::on_bookmarkNameUpdated(const QString& location, const QString& name)
{
    if (bookmarkMap.contains(location)) {
        foreach (QAction* action, bookmarkMenu->actions()) {
            if (action->isSeparator())
                continue;
            if (QString::compare(action->objectName(), location, Qt::CaseInsensitive) == 0) {
                action->setText(name);
                bookmarkMap.replace(location, name);
                break;
           }
        }
    }
}

void HelpView::on_bookmarkLocationUpdated(const QString& oldLocation, const QString& newLocation, const QString& name)
{
     if (bookmarkMap.contains(oldLocation)) {
         bookmarkMap.remove(oldLocation);
         foreach (QAction* action, bookmarkMenu->actions()) {
             if (action->isSeparator())
                 continue;
             if (QString::compare(action->objectName(), oldLocation, Qt::CaseInsensitive) == 0) {
                 bookmarkMenu->removeAction( action );
                 break;
            }
         }
     }

     bool found = false;
     foreach (QAction* action, bookmarkMenu->actions()) {
         if (action->isSeparator())
             continue;
         if ((QString::compare(action->objectName(), newLocation, Qt::CaseInsensitive) == 0) &&
             (QString::compare(action->text(), name, Qt::CaseInsensitive) == 0)
            ) {
               found = true;
               break;
         }
     }
     if (!found) {
         addBookmarkAction(newLocation, name);
         bookmarkMap.insert(newLocation, name);
     }
}

void HelpView::on_bookmarkRemoved(const QString &location, const QString& name)
{
    foreach (QAction* action, bookmarkMenu->actions()) {
        if (action->isSeparator())
            continue;
        if ((QString::compare(action->objectName(), location, Qt::CaseInsensitive) == 0) &&
            (QString::compare(action->text(), name, Qt::CaseInsensitive) == 0)
           ) {
              bookmarkMap.remove(location, name);
              bookmarkMenu->removeAction( action );
              break;
        }
    }
}

void HelpView::on_loadFinished(bool ok)
{
    if (ok) {
       if ( helpView->url().toString().startsWith("http") )
           actionOnlineHelp->setChecked( true );
       else
           actionOnlineHelp->setChecked( false );
    }
}

void HelpView::on_actionHome_triggered()
{
    helpView->load(helpStartPage);
}

void HelpView::on_actionAddBookMark_triggered()
{
    if (bookmarkMap.size() == 0)
        bookmarkMenu->addSeparator();

    QString pageUrl = helpView->page()->url().toString();
    bool found = false;
    foreach (QAction* action, bookmarkMenu->actions()) {
        if (action->isSeparator())
            continue;
        if ((QString::compare(action->objectName(), pageUrl, Qt::CaseInsensitive) == 0) &&
            (QString::compare(action->text(), helpView->page()->title(), Qt::CaseInsensitive) == 0)
           ) {
              found = true;
              break;
        }
    }
    if (!found) {
       bookmarkMap.replace(pageUrl, helpView->page()->title());
       addBookmarkAction(pageUrl, helpView->page()->title());
    }
}

void HelpView::on_actionOrganizeBookMark_triggered()
{
    BookmarkDialog bookmarkDialog(bookmarkMap, this);
    bookmarkDialog.exec();
}

void HelpView::on_actionBookMark_triggered()
{
    QAction* sAction = qobject_cast<QAction*>(sender());
    helpView->load( QUrl(sAction->toolTip(), QUrl::TolerantMode) );
}


void HelpView::on_actionOnlineHelp_triggered(bool checked)
{
    QString urlStr = helpView->url().toString();
    if (checked) {
        urlStr.replace( urlStr.indexOf("file"), 4, "https");
        urlStr.replace( urlStr.indexOf( defaultLocalHelpDir.canonicalPath()),
                        defaultLocalHelpDir.canonicalPath().size(),
                        defaultOnlineHelpLocation );
        actionOnlineHelp->setChecked( true );
    } else {
        urlStr.replace( urlStr.indexOf("https"), 5, "file");
        urlStr.replace( urlStr.indexOf( defaultOnlineHelpLocation ),
                        defaultOnlineHelpLocation.size(),
                        defaultLocalHelpDir.canonicalPath());
        actionOnlineHelp->setChecked( false );
    }
    helpView->load( QUrl(urlStr, QUrl::TolerantMode) );
}

void HelpView::on_actionOpenInBrowser_triggered()
{
    QDesktopServices::openUrl( helpView->url() );
}

void HelpView::addBookmarkAction(const QString &objectName, const QString &title)
{
    QAction* action = new QAction(this);
    action->setObjectName(objectName);
    action->setText(title);
    action->setToolTip(objectName);

    if (objectName.startsWith("file")) {
           QPixmap linkPixmap(":/img/link");
           QIcon linkButtonIcon(linkPixmap);
           action->setIcon(linkButtonIcon);
    } else if (objectName.startsWith("http")) {
           QPixmap linkPixmap(":/img/external-link");
           QIcon linkButtonIcon(linkPixmap);
           action->setIcon(linkButtonIcon);
    }
    connect(action, &QAction::triggered, this, &HelpView::on_actionBookMark_triggered);
    bookmarkMenu->addAction(action);
}

QMultiMap<QString, QString> HelpView::getBookmarkMap() const
{
    return bookmarkMap;
}

void HelpView::setBookmarkMap(const QMultiMap<QString, QString> &value)
{
    bookmarkMap = value;

    if (bookmarkMap.size() > 0)
        bookmarkMenu->addSeparator();

    QMultiMap<QString, QString>::iterator i;
    for (i = bookmarkMap.begin(); i != bookmarkMap.end(); ++i) {
        addBookmarkAction(i.key(), i.value());
    }
}

} // namespace studio
} // namespace gams
