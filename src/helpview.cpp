/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include <QDir>
#include <QtWidgets>
#include "bookmarkdialog.h"
#include "exception.h"
#include "commonpaths.h"
#include "gclgms.h"
#include "helpview.h"
#include "checkforupdatewrapper.h"

namespace gams {
namespace studio {

const QString HelpView::START_CHAPTER = "docs/index.html";
const QString HelpView::DOLLARCONTROL_CHAPTER = "docs/UG_DollarControlOptions.html";
const QString HelpView::GAMSCALL_CHAPTER = "docs/UG_GamsCall.html";
const QString HelpView::OPTION_CHAPTER = "docs/UG_OptionStatement.html";
const QString HelpView::INDEX_CHAPTER = "docs/keyword.html";
const QString HelpView::LATEST_ONLINE_HELP_URL = "https://www.gams.com/latest";

HelpView::HelpView(QWidget *parent) :
    QDockWidget(parent)
{
    CheckForUpdateWrapper c4uWrapper;
    mThisRelease = c4uWrapper.currentDistribVersionShort();
    mLastRelease = c4uWrapper.lastDistribVersionShort();

    if (c4uWrapper.distribIsLatest())
        onlineStartPageUrl = QUrl(LATEST_ONLINE_HELP_URL);
    else
        onlineStartPageUrl = QUrl( QString("https://www.gams.com/%1").arg(mThisRelease) );

    QDir dir = QDir(CommonPaths::systemDir()).filePath(START_CHAPTER);
    baseLocation = QDir(CommonPaths::systemDir()).absolutePath();
    startPageUrl = QUrl::fromLocalFile(dir.absolutePath());
    mOfflineHelpAvailable = (!dir.canonicalPath().isEmpty() && QFileInfo::exists(dir.canonicalPath()));

    setupUi(parent);
}

HelpView::~HelpView()
{
}

void HelpView::setupUi(QWidget *parent)
{
    this->setObjectName(QStringLiteral("dockHelpView"));
    this->setEnabled(true);
    this->setWindowTitle("Help");
    this->resize(QSize(950, 600));
    this->move(QPoint(200, 200));

    QWidget* helpWidget = new QWidget(parent);
    QVBoxLayout* helpVLayout = new QVBoxLayout(helpWidget);
    helpVLayout->setObjectName(QStringLiteral("helpVLayout"));
    helpVLayout->setContentsMargins(0, 0, 0, 0);
    helpWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    mHelpView = new QWebEngineView(this);
    if (mOfflineHelpAvailable) {
        mHelpView->load(startPageUrl);
    } else {
        QString htmlText;
        getErrorHTMLText( htmlText, START_CHAPTER);
        mHelpView->setHtml( htmlText );
    }
    connect(mHelpView, &QWebEngineView::loadFinished, this, &HelpView::on_loadFinished);

    QToolBar* toolbar = new QToolBar(this);

    QAction* actionHome = new QAction(this);
    actionHome->setObjectName(QStringLiteral("actionHome"));
    actionHome->setToolTip("Start page ("+ QDir(CommonPaths::systemDir()).filePath(START_CHAPTER)+")");
    actionHome->setStatusTip("Start page ("+ QDir(CommonPaths::systemDir()).filePath(START_CHAPTER)+")");
    QIcon homeButtonIcon(":/img/home");
    actionHome->setIcon(homeButtonIcon);
    connect(actionHome, &QAction::triggered, this, &HelpView::on_actionHome_triggered);

    toolbar->addAction(actionHome);
    toolbar->addSeparator();
    toolbar->addAction(mHelpView->pageAction(QWebEnginePage::Back));
    toolbar->addAction(mHelpView->pageAction(QWebEnginePage::Forward));
    toolbar->addSeparator();
    toolbar->addAction(mHelpView->pageAction(QWebEnginePage::Reload));
    toolbar->addSeparator();
    toolbar->addAction(mHelpView->pageAction(QWebEnginePage::Stop));
    toolbar->addSeparator();

    actionAddBookmark = new QAction(tr("Bookmark This Page"), this);
    actionAddBookmark->setStatusTip(tr("Bookmark This Page"));
    connect(actionAddBookmark, &QAction::triggered, this, &HelpView::on_actionAddBookMark_triggered);

    actionOrganizeBookmark = new QAction(tr("Organize Bookmarks"), this);
    actionOrganizeBookmark->setStatusTip(tr("Organize Bookmarks"));
    connect(actionOrganizeBookmark, &QAction::triggered, this, &HelpView::on_actionOrganizeBookMark_triggered);

    mBookmarkMenu = new QMenu(this);
    mBookmarkMenu->addAction(actionAddBookmark);
    mBookmarkMenu->addSeparator();
    mBookmarkMenu->addAction(actionOrganizeBookmark);

    QToolButton* bookmarkToolButton = new QToolButton(this);
    QIcon bookmarkButtonIcon(":/img/bookmark");
    bookmarkToolButton->setToolTip("Bookmarks");
    bookmarkToolButton->setIcon(bookmarkButtonIcon);
    bookmarkToolButton->setIcon(bookmarkButtonIcon);
    bookmarkToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    bookmarkToolButton->setMenu(mBookmarkMenu);

    toolbar->addWidget(bookmarkToolButton);

    QWidget* spacerWidget = new QWidget();
    spacerWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    toolbar->addWidget(spacerWidget);

    QMenu* helpMenu = new QMenu;
    actionOnlineHelp = new QAction("View This Page from https://www.gams.com/"+mThisRelease+"/", this);
    actionOnlineHelp->setStatusTip("View This Page from https://www.gams.com/"+mThisRelease+"/");
    actionOnlineHelp->setCheckable(true);
    connect(actionOnlineHelp, &QAction::triggered, this, &HelpView::on_actionOnlineHelp_triggered);
    helpMenu->addAction(actionOnlineHelp);
    helpMenu->addSeparator();

    actionOpenInBrowser = new QAction(tr("Open in Default Web Browser"), this);
    actionOpenInBrowser->setStatusTip(tr("Open this page in Default Web Browser"));
    connect(actionOpenInBrowser, &QAction::triggered, this, &HelpView::on_actionOpenInBrowser_triggered);
    helpMenu->addAction(actionOpenInBrowser);
    helpMenu->addSeparator();

    actionCopyPageURL = helpMenu->addAction(tr("Copy Page URL to Clipboard"), this,  &HelpView::copyURLToClipboard);
    actionCopyPageURL->setStatusTip(tr("Copy URL of this page to Clipboard"));

    QToolButton* helpToolButton = new QToolButton(this);
    QIcon toolButtonIcon(":/img/config");
    helpToolButton->setToolTip("Help Option");
    helpToolButton->setIcon(toolButtonIcon);
    helpToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    helpToolButton->setMenu(helpMenu);
    toolbar->addWidget(helpToolButton);

    helpVLayout->addWidget( toolbar );
    helpVLayout->addWidget( mHelpView );

    createSearchBar();
    helpVLayout->addWidget(mSearchBar);

    this->setWidget( helpWidget );
}

void HelpView::createSearchBar()
{
    QWidget* searchWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout;
    searchWidget->setLayout(layout);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setVerticalStretch(0);
    searchWidget->setSizePolicy(sizePolicy);

    mSearchLineEdit = new QLineEdit(this);
    mSearchLineEdit->setPlaceholderText(tr("Find in page..."));
    mSearchLineEdit->setClearButtonEnabled(true);
    connect(mSearchLineEdit, &QLineEdit::textChanged, this, &HelpView::searchText);
    layout->addWidget(mSearchLineEdit);

    QSizePolicy buttonSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    buttonSizePolicy.setVerticalStretch(0);

    QPushButton* backButton = new QPushButton(this);
    QPushButton* forwardButton = new QPushButton(this);
    QIcon backButtonIcon(":/img/backward");
    QIcon forwardButtonIcon(":/img/forward");
    backButton->setIcon(backButtonIcon);
    backButton->setSizePolicy(buttonSizePolicy);
    backButton->setToolTip(tr("Find the previous occurrence"));
    forwardButton->setIcon(forwardButtonIcon);
    forwardButton->setSizePolicy(buttonSizePolicy);
    forwardButton->setToolTip(tr("Find the next occurrence"));
    connect(backButton, &QPushButton::clicked, this, &HelpView::on_backButtonTriggered);
    connect(forwardButton, &QPushButton::clicked, this, &HelpView::on_forwardButtonTriggered);

    layout->addWidget(backButton);
    layout->addWidget(forwardButton);

    QCheckBox* caseSensitivity = new QCheckBox(this);
    caseSensitivity->setText(tr("Case Sensitivity"));
    caseSensitivity->setSizePolicy(buttonSizePolicy);
    caseSensitivity->setToolTip(tr("Find with Case Sensitvity"));
    connect(caseSensitivity, &QCheckBox::clicked, this, &HelpView::on_caseSensitivityToggled);
    layout->addWidget(caseSensitivity);

    QWidget* statusWidget = new QWidget(this);
    QHBoxLayout* statusWidgetLayout = new QHBoxLayout;
    statusWidget->setSizePolicy(sizePolicy);
    statusWidget->setLayout(statusWidgetLayout);
    mStatusText = new QLabel("", this);
    mStatusText->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    statusWidgetLayout->addWidget(mStatusText);

    QWidget* closeWidget = new QWidget(this);
    QHBoxLayout* closeWidgetlayout = new QHBoxLayout;
    closeWidget->setLayout(closeWidgetlayout);
    closeWidget->setSizePolicy(sizePolicy);

    QPushButton* closeButton = new QPushButton(this);
    QIcon closeButtonIcon(":/img/delete");
    closeButton->setIcon(closeButtonIcon);

    closeButton->setSizePolicy(buttonSizePolicy);
    closeButton->setToolTip(QStringLiteral("Close Find Help"));
    connect(closeButton, &QPushButton::clicked, this, &HelpView::on_searchCloseButtonTriggered);

    closeWidgetlayout->addWidget(closeButton);

    mSearchBar = new QStatusBar(this);
    mSearchBar->addPermanentWidget(searchWidget, 2);
    mSearchBar->addPermanentWidget(statusWidget, 1);
    mSearchBar->addPermanentWidget(closeWidget, 0);

    clearSearchBar();
}

void HelpView::clearSearchBar()
{
    mSearchLineEdit->clear();
    findText("", Forward);
    mSearchBar->hide();
}

void HelpView::on_urlOpened(const QUrl& location)
{
    mHelpView->load(location);
}

void HelpView::on_commandLineHelpRequested()
{
    QDir dir = QDir(baseLocation).filePath(GAMSCALL_CHAPTER);
    if (!dir.canonicalPath().isEmpty() && QFileInfo::exists(dir.canonicalPath())) {
        mHelpView->load(QUrl::fromLocalFile(dir.canonicalPath()));
    } else { // show latest online doc
        QString htmlText;
        getErrorHTMLText( htmlText, GAMSCALL_CHAPTER);
        mHelpView->setHtml( htmlText );
    }
}

void HelpView::on_dollarControlHelpRequested(const QString &word)
{
    QString anchorStr;
    if (word.toLower().startsWith("off")) {
        anchorStr = "DOLLARon"+word.toLower();
    } else if (word.toLower().startsWith("on")) {
               anchorStr = "DOLLARonoff"+word.toLower().mid(2);
    } else {
        anchorStr = "DOLLAR"+word.toLower();
    }

    QDir dir = QDir(baseLocation).filePath(DOLLARCONTROL_CHAPTER);
    if (!dir.canonicalPath().isEmpty() && QFileInfo::exists(dir.canonicalPath())) {
       QUrl url = QUrl::fromLocalFile(dir.canonicalPath());
       url.setFragment(anchorStr);
       mHelpView->load(url);
    } else { // show latest online doc
        QString htmlText;
        getErrorHTMLText( htmlText, DOLLARCONTROL_CHAPTER);
        mHelpView->setHtml( htmlText );
    }
}

void HelpView::on_keywordHelpRequested(const QString &word)
{
    QDir dir = QDir(baseLocation).filePath(INDEX_CHAPTER);
    if (!dir.canonicalPath().isEmpty() && QFileInfo::exists(dir.canonicalPath())) {
        QUrl url = QUrl::fromLocalFile(dir.canonicalPath());
        url.setQuery("q="+word);
        mHelpView->load(url);
    } else { // show latest online doc
        QString htmlText;
        getErrorHTMLText( htmlText, INDEX_CHAPTER);
        mHelpView->setHtml( htmlText );
    }
}

void HelpView::on_bookmarkNameUpdated(const QString& location, const QString& name)
{
    if (mBookmarkMap.contains(location)) {
        foreach (QAction* action, mBookmarkMenu->actions()) {
            if (action->isSeparator())
                continue;
            if (QString::compare(action->objectName(), location, Qt::CaseInsensitive) == 0) {
                action->setText(name);
                mBookmarkMap.replace(location, name);
                break;
           }
        }
    }
}

void HelpView::on_bookmarkLocationUpdated(const QString& oldLocation, const QString& newLocation, const QString& name)
{
     if (mBookmarkMap.contains(oldLocation)) {
         mBookmarkMap.remove(oldLocation);
         foreach (QAction* action, mBookmarkMenu->actions()) {
             if (action->isSeparator())
                 continue;
             if (QString::compare(action->objectName(), oldLocation, Qt::CaseInsensitive) == 0) {
                 mBookmarkMenu->removeAction( action );
                 break;
            }
         }
     }

     bool found = false;
     foreach (QAction* action, mBookmarkMenu->actions()) {
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
         mBookmarkMap.insert(newLocation, name);
     }
}

void HelpView::on_bookmarkRemoved(const QString &location, const QString& name)
{
    foreach (QAction* action, mBookmarkMenu->actions()) {
        if (action->isSeparator())
            continue;
        if ((QString::compare(action->objectName(), location, Qt::CaseInsensitive) == 0) &&
            (QString::compare(action->text(), name, Qt::CaseInsensitive) == 0)
           ) {
              mBookmarkMap.remove(location, name);
              mBookmarkMenu->removeAction( action );
              break;
        }
    }
}

void HelpView::on_loadFinished(bool ok)
{
    actionOnlineHelp->setEnabled( true );
    actionOnlineHelp->setChecked( false );
    if (ok) {
       if ( mHelpView->url().host().compare("www.gams.com", Qt::CaseInsensitive) == 0 ) {
           if (mHelpView->url().path().contains(mThisRelease))
               actionOnlineHelp->setChecked( true );
           else if (mHelpView->url().path().contains("latest") && (mThisRelease == mLastRelease))
               actionOnlineHelp->setChecked( true );
           else
               actionOnlineHelp->setEnabled( false );

       } else {
           if (mHelpView->url().scheme().compare("file", Qt::CaseSensitive) !=0 )
               actionOnlineHelp->setEnabled( false );
       }
    }
}

void HelpView::on_actionHome_triggered()
{
    mHelpView->load(startPageUrl);
}

void HelpView::on_actionAddBookMark_triggered()
{
    if (mBookmarkMap.size() == 0)
        mBookmarkMenu->addSeparator();

    QString pageUrl = mHelpView->page()->url().toString();
    bool found = false;
    foreach (QAction* action, mBookmarkMenu->actions()) {
        if (action->isSeparator())
            continue;
        if ((QString::compare(action->objectName(), pageUrl, Qt::CaseInsensitive) == 0) &&
            (QString::compare(action->text(), mHelpView->page()->title(), Qt::CaseInsensitive) == 0)
           ) {
              found = true;
              break;
        }
    }
    if (!found) {
       mBookmarkMap.replace(pageUrl, mHelpView->page()->title());
       addBookmarkAction(pageUrl, mHelpView->page()->title());
    }
}

void HelpView::on_actionOrganizeBookMark_triggered()
{
    BookmarkDialog bookmarkDialog(mBookmarkMap, this);
    bookmarkDialog.exec();
}

void HelpView::on_actionBookMark_triggered()
{
    QAction* sAction = qobject_cast<QAction*>(sender());
    mHelpView->load( QUrl(sAction->toolTip(), QUrl::TolerantMode) );
}


void HelpView::on_actionOnlineHelp_triggered(bool checked)
{
    QUrl url = mHelpView->url();

    if (checked) {
        QString urlStr = url.toDisplayString();
        urlStr.replace( urlStr.indexOf("file://"), 7, "");
        urlStr.replace( urlStr.indexOf( baseLocation),
                        baseLocation.size(),
                        onlineStartPageUrl.toDisplayString() );
        url = QUrl(urlStr);
    } else {
        if (mOfflineHelpAvailable) {
            QString urlStr = url.toDisplayString();
            urlStr.replace( urlStr.indexOf( onlineStartPageUrl.toDisplayString() ),
                            onlineStartPageUrl.toDisplayString().size(),
                            baseLocation);
            url.setUrl(urlStr);
            url.setScheme("file");
        } else {
            QString htmlText;
            getErrorHTMLText( htmlText, "");
            mHelpView->setHtml( htmlText );
        }
    }
    actionOnlineHelp->setChecked( checked );
    mHelpView->load( url );
}

void HelpView::on_actionOpenInBrowser_triggered()
{
    QDesktopServices::openUrl( mHelpView->url() );
}

void HelpView::on_searchHelp()
{
    if (mSearchBar->isVisible()) {
        clearSearchBar();
        mHelpView->setFocus();
    } else {
        mSearchBar->show();
        mSearchLineEdit->setFocus();
    }
}

void HelpView::on_backButtonTriggered()
{
    findText(mSearchLineEdit->text(), Backward);
}

void HelpView::on_forwardButtonTriggered()
{
    findText(mSearchLineEdit->text(), Forward);
}

void HelpView::on_searchCloseButtonTriggered()
{
    on_searchHelp();
}

void HelpView::on_caseSensitivityToggled(bool checked)
{
    mSearchCaseSensitivity = checked;
    findText("", Forward);
    findText(mSearchLineEdit->text(), Forward);
}

void HelpView::searchText(const QString &text)
{
    findText(text, Forward);
}

void HelpView::findText(const QString &text, SearchDirection direction)
{
    QWebEnginePage::FindFlags flags = (mSearchCaseSensitivity ? QWebEnginePage::FindCaseSensitively : QWebEnginePage::FindFlags());
    if (direction == Backward)
        flags = flags | QWebEnginePage::FindBackward;
    mHelpView->page()->findText(text, flags, [this](bool found) {
        if (found)
            mStatusText->setText("");
        else
            mStatusText->setText("No occurrences found");
    });
    if (text.isEmpty())
        mStatusText->setText("");
}

void HelpView::copyURLToClipboard()
{
    QClipboard* clip = QApplication::clipboard();;
    clip->setText( mHelpView->page()->url().toString());
}

void HelpView::zoomIn()
{
    mHelpView->page()->setZoomFactor( mHelpView->page()->zoomFactor() + 0.1);
}

void HelpView::zoomOut()
{
    mHelpView->page()->setZoomFactor( mHelpView->page()->zoomFactor() - 0.1);
}

void HelpView::resetZoom()
{
    mHelpView->page()->setZoomFactor(1.0);
}

void HelpView::setZoomFactor(qreal factor)
{
    mHelpView->page()->setZoomFactor(factor);
}

qreal HelpView::getZoomFactor()
{
    return mHelpView->page()->zoomFactor();
}

void HelpView::addBookmarkAction(const QString &objectName, const QString &title)
{
    QAction* action = new QAction(this);
    action->setObjectName(objectName);
    action->setText(title);
    action->setToolTip(objectName);

    if (objectName.startsWith("file")) {
           QIcon linkButtonIcon(":/img/link");
           action->setIcon(linkButtonIcon);
    } else if (objectName.startsWith("http")) {
           QIcon linkButtonIcon(":/img/external-link");
           action->setIcon(linkButtonIcon);
    }
    connect(action, &QAction::triggered, this, &HelpView::on_actionBookMark_triggered);
    mBookmarkMenu->addAction(action);
}

void HelpView::closeEvent(QCloseEvent *event)
{
    clearSearchBar();
    QDockWidget::closeEvent(event);
}

void HelpView::keyPressEvent(QKeyEvent *event)
{
    if (mSearchBar->isVisible()) {
        if (event->key() == Qt::Key_Escape) {
           clearSearchBar();
           mHelpView->setFocus();
        } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
                  on_forwardButtonTriggered();
        }
    }
     QDockWidget::keyPressEvent(event);
}

void HelpView::getErrorHTMLText(QString &htmlText, const QString &chapterText)
{
    QString downloadPage = QString("https://www.gams.com/%1").arg(mThisRelease);

    htmlText = "<html><head><title>Error Loading Help</title></head><body>";
    htmlText += "<div id='message'>Help Document Not Found from expected GAMS Installation at ";
    htmlText += QDir(CommonPaths::systemDir()).filePath(chapterText);
    htmlText += "</div><br/> <div>Please check your GAMS installation and configuration. You can reinstall GAMS from <a href='";
    htmlText += downloadPage;
    htmlText += "'>";
    htmlText += downloadPage;
    htmlText += "</a> or from the latest download page <a href='https://www.gams.com/latest'>https://www.gams.com/latest</a>.</div> </body></html>";
}

QMultiMap<QString, QString> HelpView::getBookmarkMap() const
{
    return mBookmarkMap;
}

void HelpView::setBookmarkMap(const QMultiMap<QString, QString> &value)
{
    mBookmarkMap = value;

    if (mBookmarkMap.size() > 0)
        mBookmarkMenu->addSeparator();

    QMultiMap<QString, QString>::iterator i;
    for (i = mBookmarkMap.begin(); i != mBookmarkMap.end(); ++i) {
        addBookmarkAction(i.key(), i.value());
    }
}

} // namespace studio
} // namespace gams
