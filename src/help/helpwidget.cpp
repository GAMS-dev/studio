/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include <QCheckBox>
#include <QClipboard>
#include <QDir>
#include <QDesktopServices>
#include <QKeyEvent>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>

#include "helptoolbar.h"
#include "helpview.h"
#include "helpwidget.h"
#include "ui_helpwidget.h"

#include "bookmarkdialog.h"
#include "support/checkforupdatewrapper.h"
#include "commonpaths.h"
#include "gclgms.h"
#include "helpdata.h"
#include "helppage.h"

namespace gams {
namespace studio {
namespace help {

HelpWidget::HelpWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HelpWidget)
{
    ui->setupUi(this);

    ui->webEngineView->showMaximized();
    ui->webEngineView->setPage( new HelpPage(ui->webEngineView) );

    QString startPageUrl = getStartPageUrl().toString();
    ui->actionHome->setToolTip("Start page ("+ startPageUrl +")");
    ui->actionHome->setStatusTip("Start page ("+ startPageUrl +")");

    mBookmarkMenu = new QMenu(this);
    mBookmarkMenu->addAction(ui->actionAddBookmark);
    mBookmarkMenu->addSeparator();
    mBookmarkMenu->addAction(ui->actionOrganizeBookmark);
    mStatusBarLabel.setWindowFlags(Qt::ToolTip);
    mStatusBarLabel.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QToolButton* bookmarkToolButton = new QToolButton(this);
    QIcon bookmarkButtonIcon(":/img/bookmark");
    bookmarkToolButton->setToolTip("Bookmarks");
    bookmarkToolButton->setIcon(bookmarkButtonIcon);
    bookmarkToolButton->setIcon(bookmarkButtonIcon);
    bookmarkToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    bookmarkToolButton->setMenu(mBookmarkMenu);

    QMenu* helpMenu = new QMenu;
    ui->actionOnlineHelp->setText("View This Page Online");
    ui->actionOnlineHelp->setStatusTip("View This Page Online");
    ui->actionOnlineHelp->setCheckable(true);
    helpMenu->addAction(ui->actionOnlineHelp);
    helpMenu->addSeparator();

    ui->actionOpenInBrowser->setStatusTip(tr("Open this page in Default Web Browser"));
    helpMenu->addAction(ui->actionOpenInBrowser);
    helpMenu->addSeparator();
    helpMenu->addAction(ui->actionCopyPageURL);

    ui->actionCopyPageURL->setStatusTip(tr("Copy URL of this page to Clipboard"));

    QToolButton* helpToolButton = new QToolButton(this);
    QIcon toolButtonIcon(":/img/config");
    helpToolButton->setToolTip("Help Option");
    helpToolButton->setIcon(toolButtonIcon);
    helpToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    helpToolButton->setMenu(helpMenu);

    createWebActionTrigger(ui->webEngineView->page(), QWebEnginePage::Back, QIcon(":/img/backward"));
    createWebActionTrigger(ui->webEngineView->page(), QWebEnginePage::Forward, QIcon(":/img/forward"));
    createWebActionTrigger(ui->webEngineView->page(), QWebEnginePage::Reload, QIcon(":/img/reload"));
    createWebActionTrigger(ui->webEngineView->page(), QWebEnginePage::Stop, QIcon(":/img/stop"));

    setupToolbar(bookmarkToolButton, helpToolButton);

    if (isDocumentAvailable(CommonPaths::systemDir(), HelpData::getChapterLocation(DocumentType::Main))) {
        ui->webEngineView->load( getStartPageUrl() );
    } else {
        QString htmlText;
        getErrorHTMLText( htmlText, getStartPageUrl());
        ui->webEngineView->setHtml( htmlText );
    }
    connect(ui->webEngineView->page(), &QWebEnginePage::linkHovered, this, &HelpWidget::linkHovered);
    connect(ui->webEngineView->page(), &QWebEnginePage::loadFinished, this, &HelpWidget::on_loadFinished);

    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &HelpWidget::searchText);
    connect(ui->backButton, &QPushButton::clicked, this, &HelpWidget::on_backButtonTriggered);
    connect(ui->forwardButton, &QPushButton::clicked, this, &HelpWidget::on_forwardButtonTriggered);
    connect(ui->caseSenstivity, &QCheckBox::clicked, this, &HelpWidget::on_caseSensitivityToggled);
    connect(ui->closeButton, &QPushButton::clicked, this, &HelpWidget::on_closeButtonTriggered);

    clearStatusBar();
    ui->searchbarWidget->hide();
}

void HelpWidget::setupToolbar(QToolButton* bookmarkToolButton, QToolButton* helpToolButton)
{
    ui->toolbarWidget->addAction(ui->actionHome);
    ui->toolbarWidget->addSeparator();
    ui->toolbarWidget->addActionBack(ui->webEngineView->pageAction(QWebEnginePage::Back));
    ui->toolbarWidget->addActionForward(ui->webEngineView->pageAction(QWebEnginePage::Forward));
    ui->toolbarWidget->addSeparator();
    ui->toolbarWidget->addActionReload(ui->webEngineView->pageAction(QWebEnginePage::Reload));
    ui->toolbarWidget->addSeparator();
    ui->toolbarWidget->addActionStop(ui->webEngineView->pageAction(QWebEnginePage::Stop));
    ui->toolbarWidget->addSeparator();
    ui->toolbarWidget->addWidget(bookmarkToolButton);
    ui->toolbarWidget->addSpacer();
    ui->toolbarWidget->addWidget(helpToolButton);

    connect(this, &HelpWidget::webActionEnabledChanged, ui->toolbarWidget, &HelpToolBar::on_webActionEnabledChanged);
    connect(ui->toolbarWidget, &HelpToolBar::webActionTriggered, this, &HelpWidget::on_webActionTriggered);
}

HelpWidget::~HelpWidget()
{
    delete mBookmarkMenu;
    delete ui;
}

QMultiMap<QString, QString> HelpWidget::getBookmarkMap() const
{
    return mBookmarkMap;
}

void HelpWidget::setBookmarkMap(const QMultiMap<QString, QString> &value)
{
    mBookmarkMap = value;

    if (mBookmarkMap.size() > 0)
        mBookmarkMenu->addSeparator();

    QMultiMap<QString, QString>::iterator i;
    for (i = mBookmarkMap.begin(); i != mBookmarkMap.end(); ++i) {
        addBookmarkAction(i.key(), i.value());
    }
}

void HelpWidget::clearStatusBar()
{
    mStatusBarLabel.clear();
    mStatusBarLabel.hide();
    ui->searchLineEdit->clear();
    findText("", Forward, ui->caseSenstivity->isChecked());
}

void HelpWidget::on_urlOpened(const QUrl &location)
{
    ui->webEngineView->load(location);
}

void HelpWidget::on_helpContentRequested(const DocumentType &type, const QString &keyword, const QString &submoduleName)
{
    QDir dir = QDir(CommonPaths::systemDir()).filePath( HelpData::getChapterLocation(type) );
    if (!submoduleName.isEmpty()) {
        if (type == DocumentType::Solvers) {
            dir.setPath( QDir(CommonPaths::systemDir()).filePath( HelpData::getSolverChapterLocation(submoduleName) ));
        }
    }
    if (dir.canonicalPath().isEmpty() || !QFileInfo::exists(dir.canonicalPath())) {
        QString htmlText;
        getErrorHTMLText( htmlText, QUrl::fromLocalFile(dir.absolutePath()) );
        ui->webEngineView->setHtml( htmlText );
        return;
    }

    QUrl url = QUrl::fromLocalFile(dir.canonicalPath());

    switch(type) {
    case DocumentType::Main :  {
        ui->webEngineView->load(QUrl::fromLocalFile(dir.canonicalPath()));
        break;
    }
    case DocumentType::DollarControl : {
        QString dollarControlStr = HelpData::getDollarControlOptionAnchor(keyword);
        if (!dollarControlStr.isEmpty())
            url.setFragment(dollarControlStr);
        ui->webEngineView->load(url);
        break;
    }
    case DocumentType::GamsCall :  {
        QString gamscallStr = HelpData::getGamsCallOptionAnchor(keyword);
        if (!gamscallStr.isEmpty())
            url.setFragment(gamscallStr);
        ui->webEngineView->load(url);
        break;
    }
    case DocumentType::Index :  {
        url.setQuery(HelpData::getKeywordIndexAnchor(keyword));
        ui->webEngineView->load(url);
        break;
    }
    case DocumentType::Solvers :  {
        QString indexStr;
        if (keyword.isEmpty())
            indexStr = HelpData::getStudioSectionAnchor(submoduleName);
        else
            indexStr = HelpData::getSolverOptionAnchor(submoduleName, keyword);
        if (!indexStr.isEmpty())
            url.setFragment(indexStr);
        ui->webEngineView->load(url);
        break;
    }
    case DocumentType::StudioMain :  {
        QString indexStr = HelpData::getStudioSectionAnchor(submoduleName);
        if (!indexStr.isEmpty())
            url.setFragment(indexStr);
        ui->webEngineView->load(url);
        break;
    }
    default: break;
    }
}

void HelpWidget::on_bookmarkNameUpdated(const QString &location, const QString &name)
{
    if (mBookmarkMap.contains(location)) {
        for (QAction* action: mBookmarkMenu->actions()) {
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

void HelpWidget::on_bookmarkLocationUpdated(const QString &oldLocation, const QString &newLocation, const QString &name)
{
    if (mBookmarkMap.contains(oldLocation)) {
        mBookmarkMap.remove(oldLocation);
        for (QAction* action: mBookmarkMenu->actions()) {
            if (action->isSeparator())
                continue;
            if (QString::compare(action->objectName(), oldLocation, Qt::CaseInsensitive) == 0) {
                mBookmarkMenu->removeAction( action );
                break;
           }
        }
    }

    bool found = false;
    for (QAction* action: mBookmarkMenu->actions()) {
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

void HelpWidget::on_bookmarkRemoved(const QString &location, const QString &name)
{
    for (QAction* action: mBookmarkMenu->actions()) {
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

void HelpWidget::on_actionHome_triggered()
{
    if (isDocumentAvailable(CommonPaths::systemDir(), HelpData::getChapterLocation(DocumentType::Main))) {
        ui->webEngineView->load( getStartPageUrl() );
    } else {
        QString htmlText;
        getErrorHTMLText( htmlText, getStartPageUrl() );
        ui->webEngineView->setHtml( htmlText );
    }
}

void HelpWidget::on_loadFinished(bool ok)
{
    ui->actionOnlineHelp->setEnabled( true );
    ui->actionOnlineHelp->setChecked( false );
    if (ok) {
       if (ui->webEngineView->page()->url().host().compare("www.gams.com", Qt::CaseInsensitive) == 0 ) {
           if (onlineStartPageUrl.isValid()) {
               if (ui->webEngineView->page()->url().path().contains( onlineStartPageUrl.path()))
                   ui->actionOnlineHelp->setChecked( true );
               else if (ui->webEngineView->page()->url().path().contains("latest"))
                   ui->actionOnlineHelp->setChecked( true );
               else
                   ui->actionOnlineHelp->setEnabled( false );
           } else {
               ui->actionOnlineHelp->setEnabled( false );
           }
       } else {
           if (ui->webEngineView->page()->url().scheme().compare("file", Qt::CaseSensitive) !=0 )
               ui->actionOnlineHelp->setEnabled( false );
       }
   } /*else {
        QString htmlText;
        getErrorHTMLText( htmlText, ui->webEngineView->page()->requestedUrl());
        ui->webEngineView->setHtml( htmlText );
    }*/
}

void HelpWidget::linkHovered(const QString &url)
{
    if (url.isEmpty()) {
        mStatusBarLabel.hide();
    } else {
        mStatusBarLabel.setText(url);
        QSize size = mStatusBarLabel.sizeHint();
        QPoint fixPoint = mapToGlobal(ui->webEngineView->geometry().bottomLeft());
        mStatusBarLabel.resize(qAbs(size.width()), qAbs(size.height()));
        mStatusBarLabel.move(fixPoint.x(), fixPoint.y()-size.height());
        mStatusBarLabel.show();
    }
    ui->webEngineView->setCurrentHoveredLink(url);
}

void HelpWidget::on_actionAddBookmark_triggered()
{
    if (mBookmarkMap.size() == 0)
        mBookmarkMenu->addSeparator();

    QString pageUrl = ui->webEngineView->page()->url().toString();
    bool found = false;
    for (QAction* action: mBookmarkMenu->actions()) {
        if (action->isSeparator())
            continue;
        if ((QString::compare(action->objectName(), pageUrl, Qt::CaseInsensitive) == 0) &&
            (QString::compare(action->text(),  ui->webEngineView->page()->title(), Qt::CaseInsensitive) == 0)
           ) {
              found = true;
              break;
        }
    }
    if (!found) {
       mBookmarkMap.replace(pageUrl, ui->webEngineView->page()->title());
       addBookmarkAction(pageUrl, ui->webEngineView->page()->title());
    }
}

void HelpWidget::on_actionOrganizeBookmark_triggered()
{
    BookmarkDialog bookmarkDialog(mBookmarkMap, this);
    bookmarkDialog.exec();
}

void HelpWidget::on_actionOnlineHelp_triggered(bool checked)
{
   QUrl url = ui->webEngineView->page()->url();
   QString baseLocation = QDir(CommonPaths::systemDir()).canonicalPath();
   onlineStartPageUrl = getOnlineStartPageUrl();
   if (checked) {
       QString urlLocalFile = url.toLocalFile();

       int newSize = urlLocalFile.size() - urlLocalFile.indexOf(baseLocation) - baseLocation.size();
       QString newPath = urlLocalFile.right(newSize);

       QString onlinepath = onlineStartPageUrl.path();
       QStringList pathList = onlinepath.split("/", QString::SkipEmptyParts);
       pathList << newPath.split("/", QString::SkipEmptyParts) ;

       QUrl onlineUrl;
       onlineUrl.setScheme(onlineStartPageUrl.scheme());
       onlineUrl.setHost(onlineStartPageUrl.host());
       onlineUrl.setPath("/" + pathList.join("/"));

       if (!url.fragment().isEmpty())
           onlineUrl.setFragment(url.fragment());
       if (url.isValid())
           ui->webEngineView->page()->setUrl( onlineUrl );
       else
           ui->webEngineView->page()->setUrl( onlineStartPageUrl );
    } else {
       if (url.host().compare("www.gams.com", Qt::CaseInsensitive) == 0 )  {
           if (isDocumentAvailable(CommonPaths::systemDir(), HelpData::getChapterLocation(DocumentType::Main))) {
               QString urlStr = url.toDisplayString();
               int docsidx = HelpData::getURLIndexFrom(urlStr);
               if (docsidx > -1) {
                   QStringList pathStrList = HelpData::getPathList();
                   QString pathStr = pathStrList.at(docsidx);
                   int pathIndex = url.path().indexOf( pathStr );
                   QString newPath = url.path().mid( pathIndex, url.path().size());
                   QStringList pathList;
                   pathList << baseLocation.split("/", QString::SkipEmptyParts) << newPath.split("/", QString::SkipEmptyParts);

                   QUrl localUrl = QUrl::fromLocalFile(QString());
                   localUrl.setScheme("file");
                   localUrl.setPath("/" + pathList.join("/"));
                   if (!url.fragment().isEmpty())
                       localUrl.setFragment(url.fragment());

                   if (localUrl.isValid())
                       ui->webEngineView->page()->setUrl( localUrl );
                   else
                       ui->webEngineView->page()->setUrl( getStartPageUrl() );
               }  else {
                   ui->webEngineView->page()->setUrl( getStartPageUrl() );
               }
           } else {
               QString htmlText;
               getErrorHTMLText( htmlText, getStartPageUrl() );
               ui->webEngineView->setHtml( htmlText );
           }

       }  else {
           ui->webEngineView->page()->setUrl( getStartPageUrl() );
       }
    }
    ui->webEngineView->setFocus();
}

void HelpWidget::on_actionOpenInBrowser_triggered()
{
    QDesktopServices::openUrl( ui->webEngineView->url() );
}

void HelpWidget::on_actionCopyPageURL_triggered()
{
    QClipboard* clip = QApplication::clipboard();
    clip->setText( ui->webEngineView->page()->url().toString());
}

void HelpWidget::on_bookmarkaction()
{
    QAction* sAction = qobject_cast<QAction*>(sender());
    if (isDocumentAvailable(CommonPaths::systemDir(), HelpData::getChapterLocation(DocumentType::Main))) {
        ui->webEngineView->load( QUrl(sAction->toolTip(), QUrl::TolerantMode) );
    } else {
        QString htmlText;
        getErrorHTMLText( htmlText, QUrl(sAction->toolTip(), QUrl::TolerantMode) );
        ui->webEngineView->setHtml( htmlText );
    }
}

void HelpWidget::addBookmarkAction(const QString &objectName, const QString &title)
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
    connect(action, &QAction::triggered, this, &HelpWidget::on_bookmarkaction);
    mBookmarkMenu->addAction(action);
}

void HelpWidget::on_searchHelp()
{
    if (ui->searchbarWidget->isVisible()) {
        ui->searchbarWidget->hide();
        ui->webEngineView->setFocus();
    } else {
        ui->searchbarWidget->show();
        ui->searchLineEdit->setFocus();
    }
    clearStatusBar();
}

void HelpWidget::on_backButtonTriggered()
{
    findText(ui->searchLineEdit->text(), Backward, ui->caseSenstivity->isChecked());
}

void HelpWidget::on_forwardButtonTriggered()
{
    findText(ui->searchLineEdit->text(), Forward, ui->caseSenstivity->isChecked());
}

void HelpWidget::on_closeButtonTriggered()
{
    on_searchHelp();
}

void HelpWidget::on_caseSensitivityToggled(bool checked)
{
    findText("", Forward, checked);
    findText(ui->searchLineEdit->text(), Forward, checked);
}

void HelpWidget::searchText(const QString &text)
{
    findText(text, Forward, ui->caseSenstivity->isChecked());
}

void HelpWidget::copySelection()
{
    if (!ui->webEngineView->selectedText().isEmpty()) {
        ui->webEngineView->pageAction(QWebEnginePage::Copy)->trigger();
    }
}

void HelpWidget::zoomIn()
{
    ui->webEngineView->page()->setZoomFactor( ui->webEngineView->page()->zoomFactor() + 0.1);
}

void HelpWidget::zoomOut()
{
    ui->webEngineView->page()->setZoomFactor( ui->webEngineView->page()->zoomFactor() - 0.1);
}

void HelpWidget::resetZoom()
{
    ui->webEngineView->page()->setZoomFactor(1.0);
}

void HelpWidget::setZoomFactor(qreal factor)
{
    ui->webEngineView->page()->setZoomFactor(factor);
}

qreal HelpWidget::getZoomFactor()
{
    return ui->webEngineView->page()->zoomFactor();
}

QWebEngineView *HelpWidget::createHelpView()
{
    HelpPage* page = new HelpPage(ui->webEngineView);
    createWebActionTrigger(page, QWebEnginePage::Back, QIcon(":/img/backward"));
    createWebActionTrigger(page, QWebEnginePage::Forward, QIcon(":/img/forward"));
    createWebActionTrigger(page, QWebEnginePage::Reload, QIcon(":/img/reload"));
    createWebActionTrigger(page, QWebEnginePage::Stop, QIcon(":/img/stop"));
    ui->webEngineView->setPage( page );
    connect(ui->webEngineView->page(), &QWebEnginePage::linkHovered, this, &HelpWidget::linkHovered);
    connect(ui->webEngineView->page(), &QWebEnginePage::loadFinished, this, &HelpWidget::on_loadFinished);
    return ui->webEngineView;
}

void HelpWidget::on_webActionTriggered(QWebEnginePage::WebAction webAction, bool checked)
{
    ui->webEngineView->page()->triggerAction(webAction, checked);
}

void HelpWidget::createWebActionTrigger(QWebEnginePage *page, QWebEnginePage::WebAction webAction, QIcon icon)
{
    QAction *action = page->action(webAction);
    action->setEnabled(false);
    action->setIcon(icon);
    connect(action, &QAction::changed, [this, action, webAction]{
        emit webActionEnabledChanged(webAction, action->isEnabled());
    });
}

void HelpWidget::wheelEvent(QWheelEvent* e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        e->accept();
        const int delta = e->delta();
        if (delta < 0) zoomOut();
        else if (delta > 0) zoomIn();
        return;
    }
    QWidget::wheelEvent(e);
}

void HelpWidget::closeEvent(QCloseEvent *event)
{
    clearStatusBar();
    QWidget::closeEvent(event);
}

void HelpWidget::keyPressEvent(QKeyEvent *event)
{
    if (ui->searchbarWidget->isVisible()) {
        if (event->key() == Qt::Key_Escape) {
            event->accept();
            clearStatusBar();
            ui->webEngineView->setFocus();
            return;
        } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
            event->accept();
            on_forwardButtonTriggered();
            return;
        }
    }

    QWidget::keyPressEvent(event);
}

QUrl HelpWidget::getStartPageUrl()
{
    QDir dir = QDir(CommonPaths::systemDir()).filePath( HelpData::getChapterLocation(DocumentType::Main) );
    return QUrl::fromLocalFile(dir.absolutePath());
}

QUrl HelpWidget::getOnlineStartPageUrl()
{
    support::CheckForUpdateWrapper c4uWrapper;
    if (!c4uWrapper.isValid())
        return HelpData::getLatestOnlineHelpUrl();

    if (c4uWrapper.distribIsLatest()) {
        return HelpData::getLatestOnlineHelpUrl();
    } else {
        int marjorversion = c4uWrapper.currentDistribVersion()/100;
        if (marjorversion>=26)
            return QUrl( QString("https://www.gams.com/%1/").arg( marjorversion ), QUrl::TolerantMode);
        else
          return QUrl( QString("https://www.gams.com/%1/").arg( c4uWrapper.currentDistribVersionShort() ), QUrl::TolerantMode );
    }
}

bool HelpWidget::isDocumentAvailable(const QString &path, const QString &chapter)
{
    QDir dir = QDir(path).filePath(chapter);
    return (!dir.canonicalPath().isEmpty() && QFileInfo::exists(dir.canonicalPath()));
}

bool HelpWidget::isCurrentReleaseTheLatestVersion()
{
    support::CheckForUpdateWrapper c4uWrapper;
    if (c4uWrapper.isValid())
       return (c4uWrapper.currentDistribVersion() == c4uWrapper.lastDistribVersion());
    else
        return true;
}

QString HelpWidget::getCurrentReleaseVersion()
{
    support::CheckForUpdateWrapper c4uWrapper;
    if (c4uWrapper.isValid())
       return c4uWrapper.currentDistribVersionShort();
    else
        return "latest";
}

void HelpWidget::getErrorHTMLText(QString &htmlText, const QUrl &url)
{
    QString downloadPage = QString("https://www.gams.com/%1").arg( getCurrentReleaseVersion() );

    htmlText = "<html><head><title>Error Loading Help</title></head><body>";
    htmlText += "<div id='message'>The requested Help Document (";
    htmlText += url.toString();
    htmlText += ") has not been found from expected GAMS Installation at ";
    htmlText += QDir(CommonPaths::systemDir()).absolutePath();  //.filePath(chapterText);
    htmlText += ".</div><br/> <div>Please check your GAMS installation and configuration. You can reinstall GAMS from <a href='";
    htmlText += downloadPage;
    htmlText += "'>";
    htmlText += downloadPage;
    htmlText += "</a> or from the latest download page <a href='https://www.gams.com/latest'>https://www.gams.com/latest</a>.</div> </body></html>";
}

void HelpWidget::findText(const QString &text, HelpWidget::SearchDirection direction, bool caseSensitivity)
{
    QWebEnginePage::FindFlags flags = (caseSensitivity ? QWebEnginePage::FindCaseSensitively : QWebEnginePage::FindFlags());
    if (direction == Backward)
        flags = flags | QWebEnginePage::FindBackward;
    ui->webEngineView->page()->findText(text, flags, [this](bool found) {
        if (found)
            ui->statusText->clear();
        else
           ui->statusText->setText("No occurrences found");
    });
    if (text.isEmpty())
        ui->statusText->clear();
}

} // namespace help
} // namespace studio
} // namespace gams
