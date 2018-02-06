#include <QWebEngineView>

#include "helpview.h"
#include "gamspaths.h"

namespace gams {
namespace studio {

HelpView::HelpView(QWidget *parent) :
    QDockWidget(parent)
{
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
    this->setAllowedAreas(Qt::TopDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea);
    this->setWindowTitle("Help");

    QWidget* helpWidget = new QWidget(this);
    QVBoxLayout* helpVLayout = new QVBoxLayout(helpWidget);
    helpVLayout->setObjectName(QStringLiteral("helpVLayout"));
    helpVLayout->setContentsMargins(0, 0, 0, 0);

    QToolBar* toolbar = new QToolBar(this);

    actionHome = new QAction(this);
    actionHome->setObjectName(QStringLiteral("homeAction"));
    actionHome->setText("?");
    actionHome->setToolTip("Main document page");
    actionHome->setShortcutVisibleInContextMenu(true);
    actionHome->setStatusTip(tr("Main help page"));

    actionBack = new QAction(this);
    actionBack->setObjectName(QStringLiteral("backAction"));
    actionBack->setText("<");
    actionBack->setToolTip("Go back one page");
    actionBack->setShortcutVisibleInContextMenu(true);

    actionNext = new QAction(this);
    actionNext->setObjectName(QStringLiteral("nextAction"));
    actionNext->setText(">");
    actionNext->setToolTip("Go forward one page");
    actionNext->setShortcutVisibleInContextMenu(true);

    toolbar->addAction(actionHome);
    toolbar->addAction(actionBack);
    toolbar->addAction(actionNext);

    helpVLayout->addWidget( toolbar );

    QWebEngineView* view = new QWebEngineView(this);
//    view->load(QUrl("https://www.gams.com/latest/docs"));
    QDir dir = QDir( QDir( GAMSPaths::systemDir() ).filePath("docs") ).filePath("index.html") ;
    view->load(QUrl::fromLocalFile(dir.canonicalPath()));
    view->show();

    helpVLayout->addWidget( view );

    this->setWidget( helpWidget );

    connect(actionHome, &QAction::triggered, this, &HelpView::on_actionHome_triggered);
    connect(actionBack, &QAction::triggered, this, &HelpView::on_actionBack_triggered);
    connect(actionNext, &QAction::triggered, this, &HelpView::on_actionNext_triggered);
}

void HelpView::on_actionHome_triggered()
{
    qDebug() << "on_actionHome_triggered()";
}

void HelpView::on_actionBack_triggered()
{
    qDebug() << "on_actionBack_triggered()";
}

void HelpView::on_actionNext_triggered()
{
    qDebug() << "on_actionNext_triggered()";
}

} // namespace studio
} // namespace gams
