#include <QWebEngineHistory>

#include "helpview.h"
#include "gamspaths.h"

namespace gams {
namespace studio {

HelpView::HelpView(QWidget *parent) :
    QDockWidget(parent)
{
    defaultLocalHelpDir = QDir( QDir( GAMSPaths::systemDir() ).filePath("docs") );
    QDir dir = defaultLocalHelpDir.filePath("index.html");
    helpLocation = QUrl::fromLocalFile(dir.canonicalPath());

    defaultOnlineHelpLocation = "www.gams.com/latest/docs";
    setupUi(parent);
}

HelpView::~HelpView()
{
}

void HelpView::setupUi(QWidget* parent)
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

    QToolBar* toolbar = new QToolBar(this);

    actionHome = new QAction(this);
    actionHome->setObjectName(QStringLiteral("actionHome"));
    actionHome->setText("Home");
    actionHome->setToolTip("Main document page");
    actionHome->setStatusTip(tr("Main help page"));

    actionBack = new QAction(this);
    actionBack->setObjectName(QStringLiteral("actionBack"));
    actionBack->setText("<");
    actionBack->setToolTip("Go back one page");
    actionBack->setShortcutVisibleInContextMenu(true);

    actionNext = new QAction(this);
    actionNext->setObjectName(QStringLiteral("actionNext"));
    actionNext->setText(">");
    actionNext->setToolTip("Go forward one page");
    actionNext->setShortcutVisibleInContextMenu(true);

    toolbar->addAction(actionHome);
    toolbar->addSeparator();
    toolbar->addAction(actionBack);
    toolbar->addAction(actionNext);
    toolbar->addSeparator();
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
        QPixmap pixmap(":/img/gams");
        QIcon ButtonIcon(pixmap);
        helpToolButton->setIcon(ButtonIcon);
    helpToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    helpToolButton->setMenu(helpMenu);
    toolbar->addWidget(helpToolButton);

    helpVLayout->addWidget( toolbar );

    helpView = new QWebEngineView(this);
    helpView->load(helpLocation);
    helpView->show();

    helpVLayout->addWidget( helpView );

    this->setWidget( helpWidget );

    connect(helpView, &QWebEngineView::loadFinished, this, &HelpView::on_loadFinished);
    connect(actionHome, &QAction::triggered, this, &HelpView::on_actionHome_triggered);
    connect(actionBack, &QAction::triggered, this, &HelpView::on_actionBack_triggered);
    connect(actionNext, &QAction::triggered, this, &HelpView::on_actionNext_triggered);
    connect(actionNext, &QAction::triggered, this, &HelpView::on_actionNext_triggered);
    connect(actionOnlineHelp, &QAction::triggered, this, &HelpView::on_actionOnlineHelp_triggered);
    connect(actionOpenInBrowser, &QAction::triggered, this, &HelpView::on_actionOpenInBrowser_triggered);
}

void HelpView::load(QUrl location)
{
    helpView->load(location);
}

void HelpView::on_loadFinished(bool ok)
{
    if (ok) {
       actionBack->setEnabled( helpView->history()->canGoBack() );
       actionNext->setEnabled( helpView->history()->canGoForward() );
    }
    if ( helpView->url().toString().startsWith("http") ) {
        actionOnlineHelp->setChecked( true );
    } else {
        actionOnlineHelp->setChecked( false );
    }
}

void HelpView::on_actionHome_triggered()
{
    helpView->load(helpLocation);
}

void HelpView::on_actionBack_triggered()
{
    helpView->back();
}

void HelpView::on_actionNext_triggered()
{
    helpView->forward();
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
