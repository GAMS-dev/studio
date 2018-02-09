#include <QWebEngineHistory>

#include "gamspaths.h"
#include "helpview.h"

namespace gams {
namespace studio {

HelpView::HelpView(QWidget *parent) :
    QDockWidget(parent)
{
    defaultLocalHelpDir = QDir( QDir( GAMSPaths::systemDir() ).filePath("docs") );
    QDir dir = defaultLocalHelpDir.filePath("index.html");
    helpLocation = QUrl::fromLocalFile(dir.canonicalPath());

    defaultOnlineHelpLocation = "www.gams.com/latest/docs";
    setupUi();
}

HelpView::~HelpView()
{
}

void HelpView::setupUi()
{
    this->setObjectName(QStringLiteral("dockHelpView"));
    this->setEnabled(true);
    this->setFloating(true);
    this->setAllowedAreas(Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea);
    this->setWindowTitle("Help");

    QWidget* helpWidget = new QWidget(this);
    QVBoxLayout* helpVLayout = new QVBoxLayout(helpWidget);
    helpVLayout->setObjectName(QStringLiteral("helpVLayout"));
    helpVLayout->setContentsMargins(0, 0, 0, 0);

    helpView = new QWebEngineView(this);
    helpView->load(helpLocation);
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

    bookmarkMenu = new QMenu(this);
    bookmarkMenu->addAction(actionAddBookmark);

//    QMap<QString, QString>::iterator i;
//    for (i = bookmarkMap.begin(); i != bookmarkMap.end(); ++i) {
//        qDebug() << i.key() << ": " << i.value();
//        QAction* action = new QAction(this);
//        action->setText(i.value());
//        action->setToolTip(i.key());
//        bookmarkMenu->addAction(action);
//    }

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
    actionOnlineHelp->setText("View this Help Online");
    actionOnlineHelp->setToolTip("View this help page Online");
    actionOnlineHelp->setStatusTip(tr("View this help page Online"));
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
    helpToolButton->setToolTip("Option");
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

void HelpView::openUrl(const QUrl& location)
{
    helpView->load(location);
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
    helpView->load(helpLocation);
}

void HelpView::on_actionAddBookMark_triggered()
{
    qDebug() << "on_actionBookMark_triggered()";

    if (bookmarkMap.size() == 0)
        bookmarkMenu->addSeparator();

    QString pageUrl = helpView->page()->url().toString();
    if (!bookmarkMap.contains(pageUrl)) {
        bookmarkMap.insert(pageUrl, helpView->page()->title());

        QAction* action = new QAction(this);
        action->setObjectName(pageUrl);
        action->setText(helpView->page()->title());
        action->setToolTip(pageUrl);

        if (pageUrl.startsWith("file")) {
            QPixmap linkPixmap(":/img/link");
            QIcon linkButtonIcon(linkPixmap);
            action->setIcon(linkButtonIcon);
        } else if (pageUrl.startsWith("http")) {
            QPixmap linkPixmap(":/img/external-link");
            QIcon linkButtonIcon(linkPixmap);
            action->setIcon(linkButtonIcon);
        }
        connect(action, &QAction::triggered, this, &HelpView::on_actionBookMark_triggered);
        bookmarkMenu->addAction(action);
    } /*else {
        actionAddBookmark->setEnabled(false);
    }*/
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

} // namespace studio
} // namespace gams
